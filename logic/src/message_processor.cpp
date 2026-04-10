#include "message_processor.h"
#include "logger/logger.h"
#include "utils/time_utils.h"
#include "utils/retry.h"

#if __has_include("message.grpc.pb.h")
#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
using chat::pb::MessageService;
using chat::pb::SaveMsgReq;
using chat::pb::SaveMsgResp;
#endif

namespace chat::logic {

MessageProcessor& MessageProcessor::GetInstance() {
    static MessageProcessor instance;
    return instance;
}

bool MessageProcessor::ProcessIncomingMessage(LogicChatMsg& msg, std::string& out_seq_id) {
    LOG_INFO("Processing incoming message: " + msg.msg_id);

    auto handler = GetHandler(msg.session_type);
    if (!handler) {
        LOG_ERROR("Unknown session type: " + std::to_string(msg.session_type));
        return false;
    }

    if (!handler->Validate(msg)) {
        LOG_WARN("Message validation failed: " + msg.msg_id);
        return false;
    }

    if (!handler->ProcessSend(msg)) {
        LOG_WARN("Message processing failed: " + msg.msg_id);
        return false;
    }

    if (!CallMessageServiceToSave(msg, out_seq_id)) {
        LOG_ERROR("Failed to save message to Message Service: " + msg.msg_id);
        return false;
    }

    if (!DispatchToPushService(msg)) {
        LOG_WARN("Failed to dispatch message to Push Service: " + msg.msg_id);
    }

    LOG_INFO("Successfully processed message: " + msg.msg_id + ", seq_id: " + out_seq_id);
    return true;
}

std::unique_ptr<BaseChatHandler> MessageProcessor::GetHandler(int session_type) {
    if (session_type == 1) { // SESSION_SINGLE
        return std::make_unique<SingleChatHandler>();
    } else if (session_type == 2) { // SESSION_GROUP
        return std::make_unique<GroupChatHandler>();
    }
    return nullptr;
}

bool MessageProcessor::CallMessageServiceToSave(LogicChatMsg& msg, std::string& out_seq_id) {
#if __has_include("message.grpc.pb.h")
    try {
        out_seq_id = chat::common::utils::ExecuteWithRetry<std::string>(3, 500, "gRPC SaveMsg", [&]() {
            auto channel = grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials());
            auto stub = MessageService::NewStub(channel);

            SaveMsgReq req;
            req.set_msg_id(msg.msg_id);
            req.set_sender_id(msg.sender_id);
            req.set_receiver_id(msg.receiver_id);
            req.set_session_type(msg.session_type);
            req.set_msg_type(msg.msg_type);
            req.set_content(msg.content);

            SaveMsgResp resp;
            grpc::ClientContext context;
            context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

            grpc::Status status = stub->SaveMsg(&context, req, &resp);
            if (!status.ok()) {
                throw std::runtime_error("gRPC Error: " + status.error_message());
            }
            if (resp.code() != 0) {
                throw std::runtime_error("Business Error: " + resp.msg());
            }
            
            return resp.seq_id();
        });
        
        msg.seq_id = out_seq_id;
        return true;
    } catch (...) {
        return false;
    }
#else
    LOG_DEBUG("Mock Calling MessageService.SaveMsg RPC...");
    int64_t timestamp = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
    out_seq_id = "SEQ_" + std::to_string(timestamp) + "_10001";
    msg.seq_id = out_seq_id;
    return true;
#endif
}

bool MessageProcessor::DispatchToPushService(const LogicChatMsg& msg) {
    LOG_DEBUG("Publishing to MQ for PushService: " + msg.msg_id);
    return true;
}

} // namespace chat::logic
