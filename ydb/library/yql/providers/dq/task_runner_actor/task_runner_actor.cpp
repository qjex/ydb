#include "task_runner_actor.h"

#include <ydb/library/yql/dq/actors/dq.h>
#include <ydb/library/yql/providers/dq/actors/actor_helpers.h>
#include "ydb/library/yql/providers/dq/actors/events.h"
#include <ydb/library/yql/providers/dq/common/yql_dq_settings.h>
#include <ydb/library/yql/providers/dq/counters/counters.h>
#include <ydb/library/yql/providers/dq/task_runner/tasks_runner_proxy.h>

#include <ydb/library/yql/minikql/mkql_string_util.h>

#include <library/cpp/actors/core/hfunc.h>

using namespace NYql::NDqs;
using namespace NActors;

namespace NYql::NDq {

namespace NTaskRunnerActor {

namespace {
template<typename T>
TTaskRunnerActorSensors GetSensors(const T& t) {
    TTaskRunnerActorSensors result;
    for (const auto& m : t.GetMetric()) {
        result.push_back(
            {
                m.GetName(), m.GetSum(), m.GetMax(), m.GetMin(), m.GetAvg(), m.GetCount()
            });
    }
    return result;
}

template<typename T>
TTaskRunnerActorRusage GetRusage(const T& t) {
    TTaskRunnerActorRusage rusage = {
        t.GetRusage().GetUtime(),
        t.GetRusage().GetStime(),
        t.GetRusage().GetMajorPageFaults()
    };
    return rusage;
}

} // namespace

class TTaskRunnerActor
    : public TActor<TTaskRunnerActor>
    , public ITaskRunnerActor
{
public:
    static constexpr char ActorName[] = "YQL_DQ_TASK_RUNNER";

    TTaskRunnerActor(
        ITaskRunnerActor::ICallbacks* parent,
        const NTaskRunnerProxy::IProxyFactory::TPtr& factory,
        const ITaskRunnerInvoker::TPtr& invoker,
        const TString& traceId)
        : TActor<TTaskRunnerActor>(&TTaskRunnerActor::Handler)
        , Parent(parent)
        , TraceId(traceId)
        , Factory(factory)
        , Invoker(invoker)
        , Local(Invoker->IsLocal())
        , Settings(MakeIntrusive<TDqConfiguration>())
        , StageId(0) {
    }

    ~TTaskRunnerActor() { }

    STRICT_STFUNC(Handler, {
        cFunc(NActors::TEvents::TEvPoison::EventType, TTaskRunnerActor::PassAway);
        HFunc(TEvTaskRunnerCreate, OnDqTask);
        HFunc(TEvContinueRun, OnContinueRun);
        HFunc(TEvPop, OnChannelPop);
        HFunc(TEvPush, OnChannelPush);
        HFunc(TEvSinkPop, OnSinkPop);
        HFunc(TEvSinkPopFinished, OnSinkPopFinished);
    })

private:
    struct TRunResult {
        bool Retriable;
        bool Fallback;
        TString FilteredStderr;
        NDqProto::TDqFailure MetricContainer;

        TRunResult() : Retriable(false), Fallback(false), FilteredStderr(), MetricContainer() {
        }
    };

    static TRunResult ParseStderr(const TString& input, TIntrusivePtr<TDqConfiguration> settings) {
        THashSet<TString> fallbackOn;
        if (settings->_FallbackOnRuntimeErrors.Get()) {
            TString parts = settings->_FallbackOnRuntimeErrors.Get().GetOrElse("");
            for (const auto& it : StringSplitter(parts).Split(',')) {
                fallbackOn.insert(TString(it.Token()));
            }
        }

        TRunResult result;
        NYql::TCounters stat;
        for (TStringBuf line: StringSplitter(input).SplitByString("\n").SkipEmpty()) {
            if (line.StartsWith("Counter1:")) {
                TVector<TString> parts;
                Split(TString(line), " ", parts);
                if (parts.size() >= 3) {
                    auto name = parts[1];
                    i64 value;
                    if (TryFromString<i64>(parts[2], value)) {
                        stat.AddCounter(name, TDuration::MilliSeconds(value));
                    }
                }
            } else if (line.StartsWith("Counter:")) {
                TVector<TString> parts;
                Split(TString(line), " ", parts);
                // name sum min max avg count
                if (parts.size() >= 7) {
                    auto name = parts[1];
                    TCounters::TEntry entry;
                    if (
                        TryFromString<i64>(parts[2], entry.Sum) &&
                        TryFromString<i64>(parts[3], entry.Min) &&
                        TryFromString<i64>(parts[4], entry.Max) &&
                        TryFromString<i64>(parts[5], entry.Avg) &&
                        TryFromString<i64>(parts[6], entry.Count))
                    {
                        stat.AddCounter(name, entry);
                    }
                }
            } else if (line.Contains("mlockall failed")) {
                // skip
            } else {
                if (!result.Fallback) {
                    if (line.Contains("FindColumnInfo(): requirement memberType->GetKind() == TType::EKind::Data")) {
                    // temporary workaround for part6/produce-reduce_lambda_list_table-default.txt
                        result.Fallback = true;
                    } else if (line.Contains("Unsupported builtin function:")) {
                        // temporary workaround for YQL-11791
                        result.Fallback = true;
                    } else if (line.Contains("embedded:Len")) {
                        result.Fallback = true;
                    } else if (line.Contains("Container killed by OOM")) {
                        // temporary workaround for YQL-12066
                        result.Fallback = true;
                    } else if (line.Contains("Expected data or optional of data, actual:")) {
                        // temporary workaround for YQL-12835
                        result.Fallback = true;
                    } else if (line.Contains("Cannot create Skiff writer for ")) {
                        // temporary workaround for YQL-12986
                        result.Fallback = true;
                    } else if (line.Contains("Skiff format expected")) {
                        // temporary workaround for YQL-12986
                        result.Fallback = true;
                    } else if (line.Contains("Pattern nodes can not get computation node by index:")) {
                        // temporary workaround for YQL-12987
                        result.Fallback = true;
                    } else if (line.Contains("contrib/libs/protobuf/src/google/protobuf/messagext.cc") && line.Contains("Message size") && line.Contains("exceeds")) {
                        // temporary workaround for YQL-12988
                        result.Fallback = true;
                    } else if (line.Contains("Cannot start container")) {
                        // temporary workaround for YQL-14221
                        result.Retriable = true;
                        result.Fallback = true;
                    } else if (line.Contains("Cannot execl")) {
                        // YQL-14099
                        result.Retriable = true;
                        result.Fallback = true;
                    } else {
                        for (const auto& part : fallbackOn) {
                            if (line.Contains(part)) {
                                result.Fallback = true;
                            }
                        }
                    }
                }

                result.FilteredStderr += line;
                result.FilteredStderr += "\n";
            }
        }
        stat.FlushCounters(result.MetricContainer);
        return result;
    }
    
    static THolder<IEventBase> StatusToError(
        const TEvError::TStatus& status, 
        TIntrusivePtr<TDqConfiguration> settings,
        ui64 stageId,
        TString message = CurrentExceptionMessage()) {
        // stderr always affects retriable/fallback flags
        auto runResult = ParseStderr(status.Stderr, settings);
        auto stderrStr = TStringBuilder{}
            << "ExitCode: " << status.ExitCode << "\n"
            << "StageId: " << stageId << "\n"
            << runResult.FilteredStderr;
        auto issueCode = runResult.Fallback
            ? TIssuesIds::DQ_GATEWAY_NEED_FALLBACK_ERROR
            : TIssuesIds::DQ_GATEWAY_ERROR;
        TIssue issue;
        if (status.ExitCode == 0) {
            // if exit code is 0, then problem lies in our code => passing message produced by it
            issue = TIssue(message).SetCode(issueCode, TSeverityIds::S_ERROR);
        } else {
            // if exit code is not 0, then problem is in their code => passing stderr
            issue = TIssue(stderrStr).SetCode(issueCode, TSeverityIds::S_ERROR);
            TStringBuf terminationMessage = stderrStr;
            auto parsedPos = TryParseTerminationMessage(terminationMessage);
            if (terminationMessage.size() < stderrStr.size()) {
                issue.AddSubIssue(MakeIntrusive<TIssue>(YqlIssue(parsedPos.GetOrElse(TPosition()), TIssuesIds::DQ_GATEWAY_ERROR, TString{terminationMessage})));
            }
        }

        if (settings->EnableComputeActor.Get().GetOrElse(false)) {
            return MakeHolder<NDq::TEvDq::TEvAbortExecution>(runResult.Retriable ? Ydb::StatusIds::UNAVAILABLE : Ydb::StatusIds::BAD_REQUEST, TVector<TIssue>{issue});
        }
        auto dqFailure = MakeHolder<TEvDqFailure>(issue, runResult.Retriable, runResult.Fallback);
        dqFailure->Record.MutableMetric()->Swap(runResult.MetricContainer.MutableMetric());
        return dqFailure;
    }

    void PassAway() override {
        if (TaskRunner) {
            Invoker->Invoke([taskRunner=std::move(TaskRunner)] () {
                taskRunner->Kill();
            });
            TaskRunner.Reset();
        }
        TActor<TTaskRunnerActor>::PassAway();
    }

    void OnContinueRun(TEvContinueRun::TPtr& ev, const TActorContext& ctx) {
        Run(ev, ctx);
    }

    void OnChannelPush(TEvPush::TPtr& ev, const NActors::TActorContext& ctx) {
        auto* actorSystem = ctx.ExecutorThread.ActorSystem;
        auto replyTo = ev->Sender;
        auto selfId = SelfId();
        auto hasData = ev->Get()->HasData;
        auto finish = ev->Get()->Finish;
        auto askFreeSpace = ev->Get()->AskFreeSpace;
        auto channelId = ev->Get()->ChannelId;
        auto cookie = ev->Cookie;
        auto data = ev->Get()->Data;
        Invoker->Invoke([hasData, selfId, cookie, askFreeSpace, finish, channelId, taskRunner=TaskRunner, data, actorSystem, replyTo, settings=Settings, stageId=StageId] () mutable {
            try {
                ui64 freeSpace = 0;
                if (hasData) {
                    // auto guard = taskRunner->BindAllocator(); // only for local mode
                    taskRunner->GetInputChannel(channelId)->Push(std::move(data));
                    if (askFreeSpace) {
                        freeSpace = taskRunner->GetInputChannel(channelId)->GetFreeSpace();
                    }
                }
                if (finish) {
                    taskRunner->GetInputChannel(channelId)->Finish();
                }

                // run
                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        new TEvContinueRun(channelId, freeSpace),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(),
                        /*flags=*/0,
                        cookie));
            }
        });
    }

    void SourcePush(
        ui64 cookie,
        ui64 index,
        NKikimr::NMiniKQL::TUnboxedValueVector&& batch,
        i64 space,
        bool finish) override
    {
        auto* actorSystem = NActors::TlsActivationContext->ExecutorThread.ActorSystem;
        auto selfId = SelfId();

        TVector<TString> strings;
        for (auto& row : batch) {
            strings.emplace_back(row.AsStringRef());
        }

        Invoker->Invoke([strings=std::move(strings),taskRunner=TaskRunner, actorSystem, selfId, cookie, parentId=ParentId, space, finish, index, settings=Settings, stageId=StageId]() mutable {
            try {
                // auto guard = taskRunner->BindAllocator(); // only for local mode
                auto source = taskRunner->GetSource(index);
                (static_cast<NTaskRunnerProxy::IStringSource*>(source.Get()))->PushString(std::move(strings), space);
                if (finish) {
                    source->Finish();
                }
                actorSystem->Send(
                    new IEventHandle(
                        parentId,
                        selfId,
                        new TEvSourcePushFinished(index),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(
                        parentId,
                        selfId,
                        StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(),
                        /*flags=*/0,
                        cookie));
            }
        });
    }

    void OnChannelPop(TEvPop::TPtr& ev, const NActors::TActorContext& ctx) {
        auto* actorSystem = ctx.ExecutorThread.ActorSystem;
        auto replyTo = ev->Sender;
        auto selfId = SelfId();
        auto cookie = ev->Cookie;
        auto wasFinished = ev->Get()->WasFinished;
        auto toPop = ev->Get()->Size;
        Invoker->Invoke([cookie,selfId,channelId=ev->Get()->ChannelId, actorSystem, replyTo, wasFinished, toPop, taskRunner=TaskRunner, settings=Settings, stageId=StageId]() {
            try {
                // auto guard = taskRunner->BindAllocator(); // only for local mode
                auto channel = taskRunner->GetOutputChannel(channelId);
                int maxChunks = std::numeric_limits<int>::max();
                bool changed = false;
                bool isFinished = false;
                i64 remain = toPop;
                ui32 dataSize = 0;
                bool hasData = true;

                if (remain == 0) {
                    // special case to WorkerActor
                    remain = 5<<20;
                    maxChunks = 1;
                }

                TVector<NDqProto::TData> chunks;
                NDqProto::TPopResponse lastPop;
                NDqProto::TPopResponse response;
                for (;maxChunks && remain > 0 && !isFinished && hasData; maxChunks--, remain -= dataSize) {
                    NDqProto::TData data;
                    lastPop = std::move(channel->Pop(data, remain));

                    for (auto& metric : lastPop.GetMetric()) {
                        *response.AddMetric() = metric;
                    }

                    hasData = lastPop.GetResult();
                    dataSize = data.GetRaw().size();
                    isFinished = !hasData && channel->IsFinished();
                    response.SetResult(response.GetResult() || hasData);
                    changed = changed || hasData || (isFinished != wasFinished);

                    if (hasData) {
                        chunks.emplace_back(std::move(data));
                    }
                }

                TDqTaskRunnerStatsInplace stats;
                NDqProto::TGetStatsResponse pbStats;
                lastPop.GetStats().UnpackTo(&pbStats);
                stats.FromProto(pbStats.GetStats());

                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        new TEvChannelPopFinished(
                            channelId,
                            std::move(chunks),
                            isFinished,
                            changed,
                            GetSensors(response),
                            TDqTaskRunnerStatsView(std::move(stats))),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(),
                        /*flags=*/0,
                        cookie));
            }
        });
    }

    void OnSinkPopFinished(TEvSinkPopFinished::TPtr& ev, const NActors::TActorContext& ctx) {
        Y_UNUSED(ctx);
        auto guard = TaskRunner->BindAllocator();
        NKikimr::NMiniKQL::TUnboxedValueVector batch;
        for (auto& row: ev->Get()->Strings) {
            batch.emplace_back(NKikimr::NMiniKQL::MakeString(row));
        }
        Parent->SinkSend(
            ev->Get()->Index,
            std::move(batch),
            std::move(ev->Get()->Checkpoint),
            ev->Get()->CheckpointSize,
            ev->Get()->Size,
            ev->Get()->Finished,
            ev->Get()->Changed);
    }

    void OnSinkPop(TEvSinkPop::TPtr& ev, const NActors::TActorContext& ctx) {
        auto selfId = SelfId();
        auto* actorSystem = ctx.ExecutorThread.ActorSystem;

        Invoker->Invoke([taskRunner=TaskRunner, selfId, actorSystem, ev=std::move(ev), settings=Settings, stageId=StageId] {
            auto cookie = ev->Cookie;
            auto replyTo = ev->Sender;

            try {
                // auto guard = taskRunner->BindAllocator(); // only for local mode
                auto sink = taskRunner->GetSink(ev->Get()->Index);
                TVector<TString> batch;
                NDqProto::TCheckpoint checkpoint;
                TMaybe<NDqProto::TCheckpoint> maybeCheckpoint;
                i64 size = 0;
                i64 checkpointSize = 0;
                if (ev->Get()->Size > 0) {
                    size = (static_cast<NTaskRunnerProxy::IStringSink*>(sink.Get()))->PopString(batch, ev->Get()->Size);
                }
                bool hasCheckpoint = sink->Pop(checkpoint);
                if (hasCheckpoint) {
                    checkpointSize = checkpoint.ByteSize();
                    maybeCheckpoint.ConstructInPlace(std::move(checkpoint));
                }
                auto finished = sink->IsFinished();
                bool changed = finished || ev->Get()->Size > 0 || hasCheckpoint;
                auto event = MakeHolder<TEvSinkPopFinished>(
                    ev->Get()->Index,
                    std::move(maybeCheckpoint), size, checkpointSize, finished, changed);
                event->Strings = std::move(batch);
                // repack data and forward
                actorSystem->Send(
                    new IEventHandle(
                        selfId,
                        replyTo,
                        event.Release(),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(),
                        /*flags=*/0,
                        cookie));
            }
        });
    }

    void OnDqTask(TEvTaskRunnerCreate::TPtr& ev, const NActors::TActorContext& ctx) {
        auto replyTo = ev->Sender;
        auto selfId = SelfId();
        auto cookie = ev->Cookie;
        auto taskId = ev->Get()->Task.GetId();
        auto& inputs = ev->Get()->Task.GetInputs();
        for (auto inputId = 0; inputId < inputs.size(); inputId++) {
            auto& input = inputs[inputId];
            if (input.HasSource()) {
                Sources.emplace(inputId);
            } else {
                for (auto& channel : input.GetChannels()) {
                    Inputs.emplace(channel.GetId());
                }
            }
        }
        ParentId = ev->Sender;

        try {
            TaskRunner = Factory->GetOld(ev->Get()->Task, TraceId);
        } catch (...) {
            TString message = "Could not create TaskRunner for " + ToString(taskId) + " on node " + ToString(replyTo.NodeId()) + ", error: " + CurrentExceptionMessage();
            Send(replyTo, MakeHolder<TEvDqFailure>(message, /*retriable = */ true, /*fallback=*/ true), 0, cookie);
            return;
        }

        auto* actorSystem = ctx.ExecutorThread.ActorSystem;
        {
            Yql::DqsProto::TTaskMeta taskMeta;
            ev->Get()->Task.GetMeta().UnpackTo(&taskMeta);
            Settings->Dispatch(taskMeta.GetSettings());
            Settings->FreezeDefaults();
            StageId = taskMeta.GetStageId();
        }
        Invoker->Invoke([taskRunner=TaskRunner, replyTo, selfId, cookie, actorSystem, settings=Settings, stageId=StageId](){
            try {
                //auto guard = taskRunner->BindAllocator(); // only for local mode
                auto result = taskRunner->Prepare();

                auto event = MakeHolder<TEvTaskRunnerCreateFinished>(
                    taskRunner->GetSecureParams(),
                    taskRunner->GetTaskParams(),
                    taskRunner->GetTypeEnv(),
                    taskRunner->GetHolderFactory(),
                    GetSensors(result));

                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        event.Release(),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(replyTo, selfId, StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(), 0, cookie));
            }
        });
    }

    void Run(TEvContinueRun::TPtr& ev, const TActorContext& ctx) {
        auto* actorSystem = ctx.ExecutorThread.ActorSystem;
        auto replyTo = ev->Sender;
        auto selfId = SelfId();
        auto cookie = ev->Cookie;
        auto inputMap = ev->Get()->AskFreeSpace
            ? Inputs
            : ev->Get()->InputChannels;

        auto sourcesMap = Sources;

        Invoker->Invoke([selfId, cookie, actorSystem, replyTo, taskRunner=TaskRunner, inputMap, sourcesMap, memLimit=ev->Get()->MemLimit, settings=Settings, stageId=StageId]() mutable {
            try {
                // auto guard = taskRunner->BindAllocator(); // only for local mode
                // guard.GetMutex()->SetLimit(memLimit);
                auto response = taskRunner->Run();
                auto res = static_cast<NDq::ERunStatus>(response.GetResult());

                THashMap<ui32, ui64> inputChannelFreeSpace;
                THashMap<ui32, ui64> sourcesFreeSpace;
                if (res == ERunStatus::PendingInput) {
                    for (auto& channelId : inputMap) {
                        inputChannelFreeSpace[channelId] = taskRunner->GetInputChannel(channelId)->GetFreeSpace();
                    }

                    for (auto& index : sourcesMap) {
                        sourcesFreeSpace[index] = taskRunner->GetSource(index)->GetFreeSpace();
                    }
                }

                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        new TEvTaskRunFinished(
                            res,
                            std::move(inputChannelFreeSpace),
                            std::move(sourcesFreeSpace),
                            GetSensors(response),
                            GetRusage(response)),
                        /*flags=*/0,
                        cookie));
            } catch (...) {
                auto status = taskRunner->GetStatus();
                actorSystem->Send(
                    new IEventHandle(
                        replyTo,
                        selfId,
                        StatusToError({status.ExitCode, status.Stderr}, settings, stageId).Release(),
                        /*flags=*/0,
                        cookie));
            }
        });
    }

    NActors::TActorId ParentId;
    ITaskRunnerActor::ICallbacks* Parent;
    TString TraceId;
    NTaskRunnerProxy::IProxyFactory::TPtr Factory;
    NTaskRunnerProxy::ITaskRunner::TPtr TaskRunner;
    ITaskRunnerInvoker::TPtr Invoker;
    bool Local;
    THashSet<ui32> Inputs;
    THashSet<ui32> Sources;
    TIntrusivePtr<TDqConfiguration> Settings;
    ui64 StageId;
};

class TTaskRunnerActorFactory: public ITaskRunnerActorFactory {
public:
    TTaskRunnerActorFactory(
        const NTaskRunnerProxy::IProxyFactory::TPtr& proxyFactory,
        const NDqs::ITaskRunnerInvokerFactory::TPtr& invokerFactory)
        : ProxyFactory(proxyFactory)
        , InvokerFactory(invokerFactory)
    { }

    std::tuple<ITaskRunnerActor*, NActors::IActor*> Create(
        ITaskRunnerActor::ICallbacks* parent,
        const TString& traceId) override
    {
        auto* actor = new TTaskRunnerActor(parent, ProxyFactory, InvokerFactory->Create(), traceId);
        return std::make_tuple(
            static_cast<ITaskRunnerActor*>(actor),
            static_cast<NActors::IActor*>(actor)
            );
    }

private:
    NTaskRunnerProxy::IProxyFactory::TPtr ProxyFactory;
    NDqs::ITaskRunnerInvokerFactory::TPtr InvokerFactory;
};

ITaskRunnerActorFactory::TPtr CreateTaskRunnerActorFactory(
    const NTaskRunnerProxy::IProxyFactory::TPtr& proxyFactory,
    const NDqs::ITaskRunnerInvokerFactory::TPtr& invokerFactory)
{
    return ITaskRunnerActorFactory::TPtr(new TTaskRunnerActorFactory(proxyFactory, invokerFactory));
}

} // namespace NTaskRunnerActor

} // namespace NYql::NDq
