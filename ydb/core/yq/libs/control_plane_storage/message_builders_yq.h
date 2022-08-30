// TODO: remove YQ-1055

#pragma once

#include <util/datetime/base.h>

#include <ydb/library/yql/dq/actors/protos/dq_status_codes.pb.h>
#include <ydb/library/yql/public/issue/yql_issue_message.h>
#include <ydb/public/api/protos/yq.pb.h>

#include <ydb/core/yq/libs/control_plane_storage/events/events.h>

#include <library/cpp/protobuf/interop/cast.h>

namespace NYq {

// Queries

class TCreateQueryBuilder {
    YandexQuery::CreateQueryRequest Request;

public:
    TCreateQueryBuilder()
    {
        SetMode(YandexQuery::RUN);
        SetType(YandexQuery::QueryContent::ANALYTICS);
        SetName("test_query_name_1");
        SetVisibility(YandexQuery::Acl::SCOPE);
        SetText("SELECT 1;");
    }

    TCreateQueryBuilder& SetMode(YandexQuery::ExecuteMode mode)
    {
        Request.set_execute_mode(mode);
        return *this;
    }

    TCreateQueryBuilder& SetType(YandexQuery::QueryContent::QueryType type)
    {
        Request.mutable_content()->set_type(type);
        return *this;
    }

    TCreateQueryBuilder& SetAutomatic(bool automatic)
    {
        Request.mutable_content()->set_automatic(automatic);
        return *this;
    }

    TCreateQueryBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TCreateQueryBuilder& SetText(const TString& content)
    {
        Request.mutable_content()->set_text(content);
        return *this;
    }

    TCreateQueryBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TCreateQueryBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TCreateQueryBuilder& SetDisposition(const YandexQuery::StreamingDisposition& disposition)
    {
        *Request.mutable_disposition() = disposition;
        return *this;
    }

    TCreateQueryBuilder& ClearAcl()
    {
        Request.mutable_content()->clear_acl();
        return *this;
    }

    const YandexQuery::CreateQueryRequest& Build()
    {
        return Request;
    }
};

class TListQueriesBuilder {
    YandexQuery::ListQueriesRequest Request;

public:
    TListQueriesBuilder()
    {
        SetLimit(10);
    }

    TListQueriesBuilder& SetPageToken(const TString& pageToken)
    {
        Request.set_page_token(pageToken);
        return *this;
    }

    TListQueriesBuilder& SetLimit(int64_t limit)
    {
        Request.set_limit(limit);
        return *this;
    }

    const YandexQuery::ListQueriesRequest& Build()
    {
        return Request;
    }
};

class TDescribeQueryBuilder {
    YandexQuery::DescribeQueryRequest Request;

public:
    TDescribeQueryBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    const YandexQuery::DescribeQueryRequest& Build()
    {
        return Request;
    }
};

class TGetQueryStatusBuilder {
    YandexQuery::GetQueryStatusRequest Request;

public:
    TGetQueryStatusBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    const YandexQuery::GetQueryStatusRequest& Build()
    {
        return Request;
    }
};

class TDeleteQueryBuilder {
    YandexQuery::DeleteQueryRequest Request;

public:
    TDeleteQueryBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    TDeleteQueryBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TDeleteQueryBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::DeleteQueryRequest& Build()
    {
        return Request;
    }
};

class TModifyQueryBuilder {
    YandexQuery::ModifyQueryRequest Request;

public:
    TModifyQueryBuilder()
    {
        SetName("test_query_name_2");
        SetMode(YandexQuery::RUN);
        SetType(YandexQuery::QueryContent::ANALYTICS);
        SetVisibility(YandexQuery::Acl::SCOPE);
        SetText("SELECT 1;");
    }

    TModifyQueryBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    TModifyQueryBuilder& SetType(YandexQuery::QueryContent::QueryType type)
    {
        Request.mutable_content()->set_type(type);
        return *this;
    }

    TModifyQueryBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TModifyQueryBuilder& SetMode(YandexQuery::ExecuteMode mode)
    {
        Request.set_execute_mode(mode);
        return *this;
    }

    TModifyQueryBuilder& SetAutomatic(bool automatic)
    {
        Request.mutable_content()->set_automatic(automatic);
        return *this;
    }

    TModifyQueryBuilder& SetText(const TString& content)
    {
        Request.mutable_content()->set_text(content);
        return *this;
    }

    TModifyQueryBuilder& SetDisposition(const YandexQuery::StreamingDisposition& disposition)
    {
        *Request.mutable_disposition() = disposition;
        return *this;
    }

    TModifyQueryBuilder& SetState(const YandexQuery::StateLoadMode& state)
    {
        Request.set_state_load_mode(state);
        return *this;
    }

    TModifyQueryBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TModifyQueryBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TModifyQueryBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    TModifyQueryBuilder& SetDescription(const TString& description)
    {
        Request.mutable_content()->set_description(description);
        return *this;
    }

    const YandexQuery::ModifyQueryRequest& Build()
    {
        return Request;
    }
};

class TControlQueryBuilder {
    YandexQuery::ControlQueryRequest Request;

public:
    TControlQueryBuilder()
    {
        SetAction(YandexQuery::ABORT);
    }

    TControlQueryBuilder& SetAction(const YandexQuery::QueryAction& action)
    {
        Request.set_action(action);
        return *this;
    }

    TControlQueryBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    TControlQueryBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TControlQueryBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::ControlQueryRequest& Build()
    {
        return Request;
    }
};

// Results

class TGetResultDataBuilder {
    YandexQuery::GetResultDataRequest Request;

public:
    TGetResultDataBuilder()
    {
        SetLimit(10);
    }

    TGetResultDataBuilder& SetQueryId(const TString& queryId)
    {
        Request.set_query_id(queryId);
        return *this;
    }

    TGetResultDataBuilder& SetResultSetIndex(int64_t resultSetIndex)
    {
        Request.set_result_set_index(resultSetIndex);
        return *this;
    }

    TGetResultDataBuilder& SetOffset(int64_t offset)
    {
        Request.set_offset(offset);
        return *this;
    }

    TGetResultDataBuilder& SetLimit(int64_t limit)
    {
        Request.set_limit(limit);
        return *this;
    }

    const YandexQuery::GetResultDataRequest& Build()
    {
        return Request;
    }
};

// Jobs

class TListJobsBuilder {
    YandexQuery::ListJobsRequest Request;

public:
    TListJobsBuilder()
    {
        SetLimit(10);
    }

    TListJobsBuilder& SetQueryId(const TString& queryId)
    {
        Request.mutable_filter()->set_query_id(queryId);
        return *this;
    }

    TListJobsBuilder& SetPageToken(const TString& pageToken)
    {
        Request.set_page_token(pageToken);
        return *this;
    }

    TListJobsBuilder& SetLimit(int64_t limit)
    {
        Request.set_limit(limit);
        return *this;
    }

    const YandexQuery::ListJobsRequest& Build()
    {
        return Request;
    }
};

class TDescribeJobBuilder {
    YandexQuery::DescribeJobRequest Request;

public:
    TDescribeJobBuilder& SetJobId(const TString& jobId)
    {
        Request.set_job_id(jobId);
        return *this;
    }

    const YandexQuery::DescribeJobRequest& Build()
    {
        return Request;
    }
};

// Connections

class TCreateConnectionBuilder {
    YandexQuery::CreateConnectionRequest Request;

public:
    TCreateConnectionBuilder()
    {
        SetName("test_connection_name_1");
        SetVisibility(YandexQuery::Acl::SCOPE);
        CreateDataStreams("my_database_id", "");
    }

    TCreateConnectionBuilder& CreateYdb(const TString& database, const TString& endpoint, const TString& serviceAccount)
    {
        auto& ydb = *Request.mutable_content()->mutable_setting()->mutable_ydb_database();
        if (serviceAccount) {
            ydb.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            ydb.mutable_auth()->mutable_current_iam();
        }

        ydb.set_database(database);
        ydb.set_endpoint(endpoint);
        return *this;
    }

    TCreateConnectionBuilder& CreateYdb(const TString& databaseId, const TString& serviceAccount)
    {
        auto& ydb = *Request.mutable_content()->mutable_setting()->mutable_ydb_database();
        if (serviceAccount) {
            ydb.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            ydb.mutable_auth()->mutable_current_iam();
        }

        ydb.set_database_id(databaseId);
        return *this;
    }

    TCreateConnectionBuilder& CreateDataStreams(const TString& databaseId, const TString& serviceAccount)
    {
        auto& yds = *Request.mutable_content()->mutable_setting()->mutable_data_streams();
        if (serviceAccount) {
            yds.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            yds.mutable_auth()->mutable_current_iam();
        }

        yds.set_database_id(databaseId);
        return *this;
    }

    TCreateConnectionBuilder& CreateClickHouse(const TString& databaseId, const TString& login, const TString& password, const TString& serviceAccount)
    {
        auto& ch = *Request.mutable_content()->mutable_setting()->mutable_clickhouse_cluster();
        if (serviceAccount) {
            ch.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            ch.mutable_auth()->mutable_current_iam();
        }

        ch.set_database_id(databaseId);
        ch.set_login(login);
        ch.set_password(password);
        return *this;
    }

    TCreateConnectionBuilder& CreateObjectStorage(const TString& bucket, const TString& serviceAccount)
    {
        auto& os = *Request.mutable_content()->mutable_setting()->mutable_object_storage();
        if (serviceAccount) {
            os.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            os.mutable_auth()->mutable_current_iam();
        }

        os.set_bucket(bucket);
        return *this;
    }

    TCreateConnectionBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TCreateConnectionBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TCreateConnectionBuilder& SetDescription(const TString& description)
    {
        Request.mutable_content()->set_name(description);
        return *this;
    }

    TCreateConnectionBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    const YandexQuery::CreateConnectionRequest& Build()
    {
        return Request;
    }
};

class TListConnectionsBuilder {
    YandexQuery::ListConnectionsRequest Request;

public:
    TListConnectionsBuilder()
    {
        SetLimit(10);
    }

    TListConnectionsBuilder& SetPageToken(const TString& pageToken)
    {
        Request.set_page_token(pageToken);
        return *this;
    }

    TListConnectionsBuilder& SetLimit(int64_t limit)
    {
        Request.set_limit(limit);
        return *this;
    }

    const YandexQuery::ListConnectionsRequest& Build()
    {
        return Request;
    }
};

class TDescribeConnectionBuilder {
    YandexQuery::DescribeConnectionRequest Request;

public:
    TDescribeConnectionBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.set_connection_id(connectionId);
        return *this;
    }

    const YandexQuery::DescribeConnectionRequest& Build()
    {
        return Request;
    }
};

class TModifyConnectionBuilder {
    YandexQuery::ModifyConnectionRequest Request;

public:
    TModifyConnectionBuilder()
    {
        SetName("test_connection_name_2");
        SetVisibility(YandexQuery::Acl::SCOPE);
        CreateDataStreams("my_database_id", "");
    }

    TModifyConnectionBuilder& CreateYdb(const TString& databaseId, const TString& serviceAccount)
    {
        auto& ydb = *Request.mutable_content()->mutable_setting()->mutable_ydb_database();
        if (serviceAccount) {
            ydb.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            ydb.mutable_auth()->mutable_current_iam();
        }

        ydb.set_database_id(databaseId);
        return *this;
    }

    TModifyConnectionBuilder& CreateDataStreams(const TString& databaseId, const TString& serviceAccount)
    {
        auto& yds = *Request.mutable_content()->mutable_setting()->mutable_data_streams();
        if (serviceAccount) {
            yds.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            yds.mutable_auth()->mutable_current_iam();
        }

        yds.set_database_id(databaseId);
        return *this;
    }

    TModifyConnectionBuilder& CreateClickHouse(const TString& databaseId, const TString& login, const TString& password, const TString& serviceAccount)
    {
        auto& ch = *Request.mutable_content()->mutable_setting()->mutable_clickhouse_cluster();
        if (serviceAccount) {
            ch.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            ch.mutable_auth()->mutable_current_iam();
        }

        ch.set_database_id(databaseId);
        ch.set_login(login);
        ch.set_password(password);
        return *this;
    }

    TModifyConnectionBuilder& CreateObjectStorage(const TString& bucket, const TString& serviceAccount)
    {
        auto& os = *Request.mutable_content()->mutable_setting()->mutable_object_storage();
        if (serviceAccount) {
            os.mutable_auth()->mutable_service_account()->set_id(serviceAccount);
        } else {
            os.mutable_auth()->mutable_current_iam();
        }

        os.set_bucket(bucket);
        return *this;
    }

    TModifyConnectionBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TModifyConnectionBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TModifyConnectionBuilder& SetDescription(const TString& description)
    {
        Request.mutable_content()->set_description(description);
        return *this;
    }

    TModifyConnectionBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TModifyConnectionBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.set_connection_id(connectionId);
        return *this;
    }

    TModifyConnectionBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::ModifyConnectionRequest& Build()
    {
        return Request;
    }
};

class TDeleteConnectionBuilder {
    YandexQuery::DeleteConnectionRequest Request;

public:
    TDeleteConnectionBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.set_connection_id(connectionId);
        return *this;
    }

    TDeleteConnectionBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TDeleteConnectionBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::DeleteConnectionRequest& Build()
    {
        return Request;
    }
};

// Bindings

class TCreateBindingBuilder {
    YandexQuery::CreateBindingRequest Request;

public:
    TCreateBindingBuilder()
    {
        SetName("test_binding_name_1");
        SetVisibility(YandexQuery::Acl::SCOPE);
        YandexQuery::DataStreamsBinding binding;
        binding.set_stream_name("my_stream");
        binding.set_format("json");
        binding.set_compression("zip");
        auto* column = binding.mutable_schema()->add_column();
        column->set_name("sample_column_name");
        column->mutable_type()->set_type_id(Ydb::Type::UINT64);
        CreateDataStreams(binding);
    }

    TCreateBindingBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.mutable_content()->set_connection_id(connectionId);
        return *this;
    }

    TCreateBindingBuilder& CreateDataStreams(const YandexQuery::DataStreamsBinding& binding)
    {
        *Request.mutable_content()->mutable_setting()->mutable_data_streams() = binding;
        return *this;
    }

    TCreateBindingBuilder& CreateObjectStorage(const YandexQuery::ObjectStorageBinding& binding)
    {
        *Request.mutable_content()->mutable_setting()->mutable_object_storage() = binding;
        return *this;
    }

    TCreateBindingBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TCreateBindingBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TCreateBindingBuilder& SetDescription(const TString& description)
    {
        Request.mutable_content()->set_name(description);
        return *this;
    }

    TCreateBindingBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    const YandexQuery::CreateBindingRequest& Build()
    {
        return Request;
    }
};

class TListBindingsBuilder {
    YandexQuery::ListBindingsRequest Request;

public:
    TListBindingsBuilder()
    {
        SetLimit(10);
    }

    TListBindingsBuilder& SetPageToken(const TString& pageToken)
    {
        Request.set_page_token(pageToken);
        return *this;
    }

    TListBindingsBuilder& SetLimit(int64_t limit)
    {
        Request.set_limit(limit);
        return *this;
    }

    TListBindingsBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.mutable_filter()->set_connection_id(connectionId);
        return *this;
    }

    const YandexQuery::ListBindingsRequest& Build()
    {
        return Request;
    }
};

class TDescribeBindingBuilder {
    YandexQuery::DescribeBindingRequest Request;

public:
    TDescribeBindingBuilder& SetBindingId(const TString& bindingId)
    {
        Request.set_binding_id(bindingId);
        return *this;
    }

    const YandexQuery::DescribeBindingRequest& Build()
    {
        return Request;
    }
};

class TModifyBindingBuilder {
    YandexQuery::ModifyBindingRequest Request;

public:
    TModifyBindingBuilder()
    {
        SetName("test_binding_name_2");
        SetVisibility(YandexQuery::Acl::SCOPE);
        YandexQuery::DataStreamsBinding binding;
        binding.set_stream_name("my_stream");
        binding.set_format("json");
        binding.set_compression("zip");
        auto* column = binding.mutable_schema()->add_column();
        column->set_name("sample_column_name");
        column->mutable_type()->set_type_id(Ydb::Type::UINT64);
        CreateDataStreams(binding);
    }

    TModifyBindingBuilder& SetConnectionId(const TString& connectionId)
    {
        Request.mutable_content()->set_connection_id(connectionId);
        return *this;
    }

    TModifyBindingBuilder& CreateDataStreams(const YandexQuery::DataStreamsBinding& binding)
    {
        *Request.mutable_content()->mutable_setting()->mutable_data_streams() = binding;
        return *this;
    }

    TModifyBindingBuilder& CreateObjectStorage(const YandexQuery::ObjectStorageBinding& binding)
    {
        *Request.mutable_content()->mutable_setting()->mutable_object_storage() = binding;
        return *this;
    }

    TModifyBindingBuilder& SetVisibility(YandexQuery::Acl::Visibility visibility)
    {
        Request.mutable_content()->mutable_acl()->set_visibility(visibility);
        return *this;
    }

    TModifyBindingBuilder& SetName(const TString& name)
    {
        Request.mutable_content()->set_name(name);
        return *this;
    }

    TModifyBindingBuilder& SetDescription(const TString& description)
    {
        Request.mutable_content()->set_name(description);
        return *this;
    }

    TModifyBindingBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TModifyBindingBuilder& SetBindingId(const TString& bindingId)
    {
        Request.set_binding_id(bindingId);
        return *this;
    }

    TModifyBindingBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::ModifyBindingRequest& Build()
    {
        return Request;
    }
};

class TDeleteBindingBuilder {
    YandexQuery::DeleteBindingRequest Request;

public:
    TDeleteBindingBuilder& SetBindingId(const TString& bindingId)
    {
        Request.set_binding_id(bindingId);
        return *this;
    }

    TDeleteBindingBuilder& SetIdempotencyKey(const TString& idempotencyKey)
    {
        Request.set_idempotency_key(idempotencyKey);
        return *this;
    }

    TDeleteBindingBuilder& SetPreviousRevision(const int64_t periousRevision)
    {
        Request.set_previous_revision(periousRevision);
        return *this;
    }

    const YandexQuery::DeleteBindingRequest& Build()
    {
        return Request;
    }
};

// internal

class TWriteResultDataBuilder {
    TString ResultId;
    int32_t ResultSetId = 0;
    int64_t StartRowId = 0;
    TInstant Deadline;
    Ydb::ResultSet ResultSet;

public:
    TWriteResultDataBuilder()
    {
        SetDeadline(TInstant::Now() + TDuration::Minutes(5));
        Ydb::ResultSet resultSet;
        auto& value = *resultSet.add_rows();
        value.set_int64_value(1);
        SetResultSet(resultSet);
    }

    TWriteResultDataBuilder& SetResultId(const TString& resultId)
    {
        ResultId = resultId;
        return *this;
    }

    TWriteResultDataBuilder& SetResultSetIndex(int32_t resultSetId)
    {
        ResultSetId = resultSetId;
        return *this;
    }

    TWriteResultDataBuilder& SetStartRowId(int64_t startRowId)
    {
        StartRowId = startRowId;
        return *this;
    }

    TWriteResultDataBuilder& SetDeadline(const TInstant& deadline)
    {
        Deadline = deadline;
        return *this;
    }

    TWriteResultDataBuilder& SetResultSet(const Ydb::ResultSet& resultSet)
    {
        ResultSet = resultSet;
        return *this;
    }

    std::unique_ptr<TEvControlPlaneStorage::TEvWriteResultDataRequest> Build()
    {
        auto request = std::make_unique<TEvControlPlaneStorage::TEvWriteResultDataRequest>();
        request->Request.mutable_result_id()->set_value(ResultId);
        *request->Request.mutable_result_set() = ResultSet;
        request->Request.set_result_set_id(ResultSetId);
        request->Request.set_offset(StartRowId);
        *request->Request.mutable_deadline() = NProtoInterop::CastToProto(Deadline);
        return request;
    }
};

class TGetTaskBuilder {
    TString Owner;
    TString HostName;
    TString TenantName;

public:
    TGetTaskBuilder()
    {
        SetOwner(DefaultOwner());
        SetHostName("localhost");
        SetTenantName("/root/tenant");
    }

    static TString DefaultOwner() {
        return "owner";
    }

    TGetTaskBuilder& SetOwner(const TString& owner)
    {
        Owner = owner;
        return *this;
    }

    TGetTaskBuilder& SetHostName(const TString& hostName)
    {
        HostName = hostName;
        return *this;
    }

    TGetTaskBuilder& SetTenantName(const TString& tenantName)
    {
        TenantName = tenantName;
        return *this;
    }

    std::unique_ptr<TEvControlPlaneStorage::TEvGetTaskRequest> Build()
    {
        auto request = std::make_unique<TEvControlPlaneStorage::TEvGetTaskRequest>();
        request->Request.set_tenant(TenantName);
        request->Request.set_owner_id(Owner);
        request->Request.set_host(HostName);
        return request;
    }
};

class TPingTaskBuilder {
    TString TenantName;
    TString CloudId;
    TString Scope;
    TString QueryId;
    TString ResultId;
    TString Owner;
    TInstant Deadline;
    TMaybe<YandexQuery::QueryMeta::ComputeStatus> Status;
    TMaybe<NYql::TIssues> Issues;
    TMaybe<NYql::TIssues> TransientIssues;
    TMaybe<TString> Statistics;
    TMaybe<TVector<YandexQuery::ResultSetMeta>> ResultSetMetas;
    TMaybe<TString> Ast;
    TMaybe<TString> Plan;
    TMaybe<TInstant> StartedAt;
    TMaybe<TInstant> FinishedAt;
    bool ResignQuery = false;
    NYql::NDqProto::StatusIds::StatusCode StatusCode = NYql::NDqProto::StatusIds::UNSPECIFIED;
    TVector<NYq::TEvControlPlaneStorage::TTopicConsumer> CreatedTopicConsumers;
    TVector<TString> DqGraphs;
    i32 DqGraphIndex = 0;

public:
    TPingTaskBuilder()
    {
        SetDeadline(TInstant::Now() + TDuration::Minutes(5));
        SetTenantName("/root/tenant");
    }

    TPingTaskBuilder& SetTenantName(const TString& tenantName)
    {
        TenantName = tenantName;
        return *this;
    }

    TPingTaskBuilder& SetCloudId(const TString& cloudId)
    {
        CloudId = cloudId;
        return *this;
    }

    TPingTaskBuilder& SetScope(const TString& scope)
    {
        Scope = scope;
        return *this;
    }

    TPingTaskBuilder& SetQueryId(const TString& queryId)
    {
        QueryId = queryId;
        return *this;
    }

    TPingTaskBuilder& SetResultId(const TString& resultId)
    {
        ResultId = resultId;
        return *this;
    }

    TPingTaskBuilder& SetOwner(const TString& owner)
    {
        Owner = owner;
        return *this;
    }

    TPingTaskBuilder& SetDeadline(const TInstant& deadline)
    {
        Deadline = deadline;
        return *this;
    }

    TPingTaskBuilder& SetStatus(const YandexQuery::QueryMeta::ComputeStatus& status)
    {
        Status = status;
        return *this;
    }

    TPingTaskBuilder& SetIssues(const NYql::TIssues& issues)
    {
        Issues = issues;
        return *this;
    }

    TPingTaskBuilder& SetTransientIssues(const NYql::TIssues& issues)
    {
        TransientIssues = issues;
        return *this;
    }

    TPingTaskBuilder& SetStatistics(const TString& statistics)
    {
        Statistics = statistics;
        return *this;
    }

    TPingTaskBuilder& SetResultSetMetas(const TVector<YandexQuery::ResultSetMeta>& resultSetMetas)
    {
        ResultSetMetas = resultSetMetas;
        return *this;
    }

    TPingTaskBuilder& SetAst(const TString& ast)
    {
        Ast = ast;
        return *this;
    }

    TPingTaskBuilder& SetPlan(const TString& plan)
    {
        Plan = plan;
        return *this;
    }

    TPingTaskBuilder& SetStatedAt(const TInstant& started)
    {
        StartedAt = started;
        return *this;
    }

    TPingTaskBuilder& SetFinishedAt(const TInstant& finished)
    {
        FinishedAt = finished;
        return *this;
    }

    TPingTaskBuilder& SetResignQuery(bool resignQuery = true)
    {
        ResignQuery = resignQuery;
        return *this;
    }

    TPingTaskBuilder& SetStatusCode(NYql::NDqProto::StatusIds::StatusCode statusCode = NYql::NDqProto::StatusIds::UNSPECIFIED)
    {
        StatusCode = statusCode;
        return *this;
    }

    TPingTaskBuilder& AddCreatedConsumer(const TString& databaseId, const TString& database, const TString& topicPath, const TString& consumerName, const TString& clusterEndpoint, bool useSsl)
    {
        CreatedTopicConsumers.emplace_back(NYq::TEvControlPlaneStorage::TTopicConsumer{databaseId, database, topicPath, consumerName, clusterEndpoint, useSsl, "", false});
        return *this;
    }

    TPingTaskBuilder& AddDqGraph(const TString& dqGraph)
    {
        DqGraphs.push_back(dqGraph);
        return *this;
    }

    TPingTaskBuilder& SetDqGraphIndex(i32 dqGraphIndex)
    {
        DqGraphIndex = dqGraphIndex;
        return *this;
    }

    std::unique_ptr<TEvControlPlaneStorage::TEvPingTaskRequest> Build()
    {
        Fq::Private::PingTaskRequest request;
        request.set_owner_id(Owner);
        request.mutable_query_id()->set_value(QueryId);
        request.mutable_result_id()->set_value(ResultId);
        if (Status) {
            request.set_status(*Status);
        }
        request.set_status_code(StatusCode);
        if (Issues) {
            NYql::IssuesToMessage(*Issues, request.mutable_issues());
        }
        if (TransientIssues) {
            NYql::IssuesToMessage(*TransientIssues, request.mutable_transient_issues());
        }
        if (Statistics) {
            request.set_statistics(*Statistics);
        }
        if (ResultSetMetas) {
            for (const auto& meta : *ResultSetMetas) {
                *request.add_result_set_meta() = meta;
            }
        }
        for (const auto& dqGraph : DqGraphs) {
            request.add_dq_graph(dqGraph);
        }
        request.set_dq_graph_index(DqGraphIndex);
        if (Ast) {
            request.set_ast(*Ast);
        }
        if (Plan) {
            request.set_plan(*Plan);
        }
        request.set_resign_query(ResignQuery);
        for (const auto& consumer : CreatedTopicConsumers) {
            auto& cons = *request.add_created_topic_consumers();
            cons.set_database_id(consumer.DatabaseId);
            cons.set_database(consumer.Database);
            cons.set_topic_path(consumer.TopicPath);
            cons.set_consumer_name(consumer.ConsumerName);
            cons.set_cluster_endpoint(consumer.ClusterEndpoint);
            cons.set_use_ssl(consumer.UseSsl);
            cons.set_token_name(consumer.TokenName);
            cons.set_add_bearer_to_token(consumer.AddBearerToToken);
        }
        request.set_tenant(TenantName);
        request.set_scope(Scope);
        *request.mutable_deadline() = NProtoInterop::CastToProto(Deadline);
        if (StartedAt) {
            *request.mutable_started_at() = NProtoInterop::CastToProto(*StartedAt);
        }
        if (FinishedAt) {
            *request.mutable_finished_at() = NProtoInterop::CastToProto(*FinishedAt);
        }

        return std::make_unique<TEvControlPlaneStorage::TEvPingTaskRequest>(std::move(request));
    }
};

class TNodesHealthCheckBuilder {
    TString TenantName;
    ui32 NodeId = 0;
    TString HostName;
    TString InstanceId;
    ui64 ActiveWorkers = 0;
    ui64 MemoryLimit = 0;
    ui64 MemoryAllocated = 0;

public:
    TNodesHealthCheckBuilder()
    {}

    TNodesHealthCheckBuilder& SetTenantName(const TString& tenantName)
    {
        TenantName = tenantName;
        return *this;
    }

    TNodesHealthCheckBuilder& SetNodeId(const ui32& nodeId)
    {
        NodeId = nodeId;
        return *this;
    }

    TNodesHealthCheckBuilder& SetHostName(const TString& hostName)
    {
        HostName = hostName;
        return *this;
    }

    TNodesHealthCheckBuilder& SetInstanceId(const TString& instanceId)
    {
        InstanceId = instanceId;
        return *this;
    }

    TNodesHealthCheckBuilder& SetActiveWorkers(const ui64& activeWorkers)
    {
        ActiveWorkers = activeWorkers;
        return *this;
    }

    TNodesHealthCheckBuilder& SetMemoryLimit(const ui64& memoryLimit)
    {
        MemoryLimit = memoryLimit;
        return *this;
    }

    TNodesHealthCheckBuilder& SetMemoryAllocated(const ui64& memoryAllocated)
    {
        MemoryAllocated = memoryAllocated;
        return *this;
    }

    std::unique_ptr<TEvControlPlaneStorage::TEvNodesHealthCheckRequest> Build()
    {
        Fq::Private::NodesHealthCheckRequest request;
        request.set_tenant(TenantName);
        auto& node = *request.mutable_node();
        node.set_node_id(NodeId);
        node.set_instance_id(InstanceId);
        node.set_hostname(HostName);
        node.set_active_workers(ActiveWorkers);
        node.set_memory_limit(MemoryLimit);
        node.set_memory_allocated(MemoryAllocated);
        return std::make_unique<TEvControlPlaneStorage::TEvNodesHealthCheckRequest>(std::move(request));
    }
};

template <class TEvent, class TMessage>
class TRateLimiterResourceBuilderImpl {
    TString Owner;
    TString QueryId;
    TString Scope;
    TString Tenant;

public:
    TRateLimiterResourceBuilderImpl()
    {
        SetOwner(DefaultOwner());
    }

    static TString DefaultOwner()
    {
        return "owner";
    }

    TRateLimiterResourceBuilderImpl& SetOwner(const TString& owner)
    {
        Owner = owner;
        return *this;
    }

    TRateLimiterResourceBuilderImpl& SetQueryId(const TString& queryId)
    {
        QueryId = queryId;
        return *this;
    }

    TRateLimiterResourceBuilderImpl& SetScope(const TString& scope)
    {
        Scope = scope;
        return *this;
    }

    TRateLimiterResourceBuilderImpl& SetTenant(const TString& tenant)
    {
        Tenant = tenant;
        return *this;
    }

    std::unique_ptr<TEvent> Build()
    {
        TMessage req;
        req.set_owner_id(Owner);
        req.mutable_query_id()->set_value(QueryId);
        req.set_scope(Scope);
        req.set_tenant(Tenant);
        return std::make_unique<TEvent>(std::move(req));
    }
};

using TCreateRateLimiterResourceBuilder = TRateLimiterResourceBuilderImpl<TEvControlPlaneStorage::TEvCreateRateLimiterResourceRequest, Fq::Private::CreateRateLimiterResourceRequest>;
using TDeleteRateLimiterResourceBuilder = TRateLimiterResourceBuilderImpl<TEvControlPlaneStorage::TEvDeleteRateLimiterResourceRequest, Fq::Private::DeleteRateLimiterResourceRequest>;

}
