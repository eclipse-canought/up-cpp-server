/********************************************************************************
* Copyright (c) 2025 Contributors to the Eclipse Foundation
*
* See the NOTICE file(s) distributed with this work for additional
* information regarding copyright ownership.
*
* This program and the accompanying materials are made available under the
* terms of the Apache License Version 2.0 which is available at
* https://www.apache.org/licenses/LICENSE-2.0
*
* SPDX-License-Identifier: Apache-2.0
********************************************************************************/
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <up-cpp/communication/RpcClient.h>

#include <chrono>
#include <csignal>
#include <iostream>

#include <filesystem>

#include "ZenohUTransport.h"
#include "common.h"

#include "can_message.pb.h"
using namespace proto_test::example;

#include "translator-protobuf.pb.h"
using namespace translator;

using namespace uprotocol::v1;
using namespace uprotocol::communication;
using namespace uprotocol::datamodel::builder;
using namespace uprotocol::transport;


std::promise<UMessage> promise;


void OnReceive(RpcClient::MessageOrStatus expected) {
    bool bRet = true;

    if (!expected.has_value()) {
        UStatus status = expected.error();
        spdlog::error("Expected value not found. -- Status: {}",
                    status.DebugString());
        bRet = false;
    }

    UMessage message = std::move(expected).value();

    if (message.attributes().payload_format() !=
        UPayloadFormat::UPAYLOAD_FORMAT_PROTOBUF) {
        spdlog::error("Received message has unexpected payload format (no protobuf):\n{}",
                    message.DebugString());
        bRet = false;
    }

    if  (bRet)
    {
        spdlog::debug("(Client) Received message: {}", message.DebugString());
    }

    promise.set_value(message);
}

// return types for rpc message calls
struct GetClaimedAddressResponseT {
    std::string appID;
    std::string sequenceNo;
    std::string claimedAddress;
    std::string responseCode;
};

struct UDSCANFormatT {
    std::string canPhysReqFormat;
    std::string canRespUSDTFormat;
};

struct UDSOpenCommChannelRequestT {
    std::string appID;
    std::string sequenceNo;
    std::string toolAddress;
    std::string ecuAddress;
    std::vector<UDSCANFormatT> canFormat; //repeated UDSCANFormat
    std::string resourceName;
};

struct UDSOpenCommChannelResponseT {
    std::string appID;
    std::string connectionID;
    std::string sequenceNo;
    std::string responseCode;
};

class ExampleClient {
public:
    ExampleClient(std::shared_ptr<UTransport> transport)
    {
        UUri method = getRpcUUri(12);
        // set authority for server connection
        std::string server_authority = "test_rpc_server.app";
        method.set_authority_name(server_authority);
        client_ = std::make_unique<RpcClient>(transport, std::move(method),
                                              UPriority::UPRIORITY_CS4,
                                              std::chrono::milliseconds(500));
    }

    template <typename ProtobufT>
    CanMessage createCanMsg(const ProtobufT& message, const CanMessage::CanMessageType canMsgType) {
        CanMessage can_message;
        std::string serialized_can_message;
        message.SerializeToString(&serialized_can_message);

        can_message.set_type(canMsgType);
        spdlog::debug("(Client) use CAN message of type {}",
                      CanMessage::CanMessageType_Name(can_message.type()));
        can_message.set_message_payload(serialized_can_message);

        return can_message;
    }

    CanMessage rpcCall(CanMessage msg) {
        CanMessage can_message_ret;

        promise = std::promise<UMessage>();
        std::future<UMessage> future = promise.get_future();

        spdlog::info("Sending payload:  {}", msg.DebugString());
        spdlog::info("<<== (Client) Sending message of type:  {}", CanMessage::CanMessageType_Name(msg.type()));
        uprotocol::datamodel::builder::Payload client_request_payload(msg);
        handle = client_->invokeMethod(std::move(client_request_payload), OnReceive);

        // wait for RPC response message arrived
        UMessage result_message = future.get();

        can_message_ret.ParseFromString(result_message.payload());      // UMessage Payload to Protobuf CanMessage
        spdlog::debug("==>> (Client) Received CAN message response of type {}", CanMessage::CanMessageType_Name(can_message_ret.type()));

        return can_message_ret;
    }

    // RPC Client Stub methods
    GetClaimedAddressResponseT GetClaimedAddress(std::string appID, std::string sequenceNo) {
        GetClaimedAddressRequest request;
        GetClaimedAddressResponse response;
        GetClaimedAddressResponseT response_value;
        CanMessage::CanMessageType msgTypeReq, msgTypeResp;
        
        std::cout << "<==== Client calls GetClaimedAddress ====>" << std::endl;

        msgTypeReq = CanMessage::GETCLAIMEDADDRESSREQUEST;
        msgTypeResp = CanMessage::GETCLAIMEDADDRESSRESPONSE;
        request.set_appid(appID);
        request.set_sequenceno(sequenceNo);
        std::cout << request.DebugString() << std::endl;

        // fill msg into CanMessage with type info
        CanMessage can_message;
        can_message = createCanMsg(request, msgTypeReq);

        // send RPC message
        CanMessage can_message_ret;
        can_message_ret = rpcCall(can_message);

        if (can_message_ret.type() == msgTypeResp)
        {
            // parse Response as CanMessage
            response.ParseFromString(can_message_ret.message_payload());
            response_value.appID = response.appid();
            response_value.sequenceNo = response.sequenceno();
            response_value.claimedAddress = response.claimedaddress();
            response_value.responseCode = response.responsecode();
        }
        else
        {
            spdlog::error("received wrong message type as response");
        }

        return response_value;
    }

    UDSOpenCommChannelResponseT UDSOpenCommChannel(UDSOpenCommChannelRequestT req) {
        UDSOpenCommChannelRequest request;
        UDSOpenCommChannelResponse response;
        UDSOpenCommChannelResponseT response_value;
        CanMessage::CanMessageType msgTypeReq, msgTypeResp;
        
        std::cout << "<==== Client calls UDSOpenCommChannel ====>" << std::endl;

        msgTypeReq = CanMessage::UDSOPENCOMMCHANNELREQUEST;
        msgTypeResp = CanMessage::UDSOPENCOMMCHANNELRESPONSE;
        request.set_appid(req.appID);
        request.set_sequenceno(req.sequenceNo);
        request.set_tooladdress(req.toolAddress);
        request.set_ecuaddress(req.ecuAddress);
        for (const auto& format : req.canFormat) {
            UDSCANFormat* udsformat = request.add_canformat();
            udsformat->set_canphysreqformat(format.canPhysReqFormat);
            udsformat->set_canrespusdtformat(format.canRespUSDTFormat);
        }
        request.set_resourcename(req.resourceName);
        std::cout << request.DebugString() << std::endl;

        // fill msg into CanMessage with type info
        CanMessage can_message;
        can_message = createCanMsg(request, msgTypeReq);

        // send RPC message
        CanMessage can_message_ret;
        can_message_ret = rpcCall(can_message);

        if (can_message_ret.type() == msgTypeResp)
        {
            // parse Response as CanMessage
            response.ParseFromString(can_message_ret.message_payload());
            response_value.appID = response.appid();
            response_value.connectionID = response.connectionid();
            response_value.sequenceNo = response.sequenceno();
            response_value.responseCode = response.responsecode();
        }
        else
        {
            spdlog::error("received wrong message type as response");
        }

        return response_value;
    }

 private:
    std::unique_ptr<RpcClient> client_;
    RpcClient::InvokeHandle handle;
};



bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true;
    }
}



/* The sample RPC client application demonstrates how to send RPC requests and
 * wait for the response
 * The communication is using ZenohUTransport and message catalog is provided via protobuf definitions */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    signal(SIGINT, signalHandler);

    UUri source = getRpcUUri(0);
    source.set_authority_name("test_rpc_client.app");
    auto transport = std::make_shared<ZenohUTransport>(source, "./zenoh_config_transl_test.json5");

    ExampleClient client(transport);

    GetClaimedAddressResponseT resp1 = client.GetClaimedAddress("appid_0011", "seq01");
    std::cout << "Client received address response: "
        << "id " << resp1.appID << ", "
        << "seq# " << resp1.sequenceNo << ", "
        << "claimedAddress " << resp1.claimedAddress << ", "
        << "code " << resp1.responseCode << ", "
        << std::endl;

    UDSOpenCommChannelRequestT req2;
    req2.appID = "appid_0012";
    req2.sequenceNo = "seq012";
    req2.toolAddress = "tool01";
    req2.ecuAddress = "ecu02";
    std::vector<UDSCANFormatT> canFormat;
    UDSCANFormatT el1, el2;
    el1.canPhysReqFormat = "canphys 1";
    el1.canRespUSDTFormat = "canrespusdt 1";
    canFormat.push_back(el1);
    el2.canPhysReqFormat = "canphys 2";
    el2.canRespUSDTFormat = "canrespusdt 2";
    canFormat.push_back(el2);
    req2.resourceName = "res09";
    UDSOpenCommChannelResponseT resp2 = client.UDSOpenCommChannel(req2);
    std::cout << "Client received UDSOpenCommChannel response: "
        << "id " << resp2.appID << ", "
        << "seq# " << resp2.sequenceNo << ", "
        << "claimedAddress " << resp2.connectionID << ", "
        << "code " << resp2.responseCode << ", "
        << std::endl;

    while (!gTerminate) {
        // wait some minimum time to receive answer
        sleep(3);
    }

    return 0;
}
