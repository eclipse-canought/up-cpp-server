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
#include <up-cpp/communication/RpcServer.h>

#include <chrono>
#include <csignal>
#include <iostream>

#include "ZenohUTransport.h"
#include "common.h"

using namespace uprotocol::v1;
using namespace uprotocol::communication;
using namespace uprotocol::datamodel::builder;
using namespace uprotocol::transport;

#include "can_message.pb.h"
using namespace proto_test::example;

#include "translator-protobuf.pb.h"
using namespace translator;


bool gTerminate = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Ctrl+C received. Exiting..." << std::endl;
        gTerminate = true;
    }
}

template <typename T>
bool SerializeToCanMessage(const T& message,
                                CanMessage::CanMessageType type,
                                CanMessage* can_message) {
    std::string serialized_data;
    if (!message.SerializeToString(&serialized_data)) {
        std::cerr << "Error for serializing of message."
                    << std::endl;
        return false;
    }
    can_message->set_type(type);
    can_message->set_message_payload(serialized_data);
    return true;
}

// RPC Server Stub methods
CanMessage handleMessageGetClaimedAddressRequest(const CanMessage& msg) {
    CanMessage can_message_ret;

    spdlog::debug("(Server) Received GetClaimedAddressRequest");
    GetClaimedAddressRequest request;
    request.ParseFromString(msg.message_payload());
    std::string appID = request.appid();
    std::string sequenceNo = request.sequenceno();

    // action ...
    spdlog::debug("(Server) Received GetClaimedAddressRequest with appID = {} and sequenceNo {}", appID, sequenceNo);

    // answer to client
    GetClaimedAddressResponse response;
    response.set_appid(appID);
    response.set_sequenceno(sequenceNo);
    response.set_claimedaddress("adress1");
    response.set_responsecode("0");

    SerializeToCanMessage(response, CanMessage::GETCLAIMEDADDRESSRESPONSE, &can_message_ret);

    return can_message_ret;
}

CanMessage handleMessageUDSOpenCommChannelRequest(const CanMessage& msg) {
    CanMessage can_message_ret;

    spdlog::debug("(Server) Received UDSOpenCommChannelRequest");
    UDSOpenCommChannelRequest request;
    request.ParseFromString(msg.message_payload());
    std::string appID = request.appid();
    std::string sequenceNo = request.sequenceno();
    int vec_size = request.canformat_size();

    // action ...
    spdlog::debug("(Server) Received UDSOpenCommChannelRequest with appID = {} and sequenceNo {}, vector size {}",
       appID, sequenceNo, vec_size);

    for (int elem = 0; elem < request.canformat_size(); ++elem) {
        const UDSCANFormat& format = request.canformat(elem);
        spdlog::debug("UDSCANFormat {}: canphysreqformat = {} canrespusdtformat = {}",
            elem, format.canphysreqformat(), format.canrespusdtformat());
    }

    // answer to client
    UDSOpenCommChannelResponse response;
    response.set_appid(appID);
    response.set_sequenceno(sequenceNo);
    response.set_connectionid("conn1");
    response.set_responsecode("001");

    SerializeToCanMessage(response, CanMessage::UDSOPENCOMMCHANNELRESPONSE, &can_message_ret);

    return can_message_ret;
}

std::optional<uprotocol::datamodel::builder::Payload> OnReceive(
    const UMessage& message) {
    // Validate message is an RPC request
    if (message.attributes().type() != UMessageType::UMESSAGE_TYPE_REQUEST) {
        spdlog::error("Received message is not a request\n{}",
                      message.DebugString());
        return {};
    }

    // Received request, generate response by calling appropriate handle method
    spdlog::debug("(Server) Received request now:\n{}", message.DebugString());

    CanMessage can_message, can_message_ret;
    can_message.ParseFromString(message.payload());
    spdlog::debug("==>> (Server) Received CAN message type {}", CanMessage::CanMessageType_Name(can_message.type()));

    // dispatch to server stub methods for CANTranslator access
    switch (can_message.type()) {
        case CanMessage::GETCLAIMEDADDRESSREQUEST:
            can_message_ret = handleMessageGetClaimedAddressRequest(can_message);
            break;
        case CanMessage::UDSOPENCOMMCHANNELREQUEST:
            can_message_ret = handleMessageUDSOpenCommChannelRequest(can_message);
            break;
        default:
            can_message_ret.set_type(CanMessage::UNKNOWN);
    }
    Payload payload(can_message_ret);

    // payload contains answer from CANTranslator access
    spdlog::info("(Server) Sending payload:  {}", can_message_ret.DebugString());
    spdlog::info("<<== (Server) Sending message of type:  {}", CanMessage::CanMessageType_Name(can_message_ret.type()));

    // hand over payload for sending back answer to RPC Client
    return payload;
}


/* The sample RPC server application demonstrates how to receive RPC requests
 * and send a response back to the client.
 * The communication is using ZenohUTransport and message catalog is provided via protobuf definitions */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    signal(SIGINT, signalHandler);

    std::string my_authority = "test_rpc_server.app";
    UUri source = getRpcUUri(0);
    source.set_authority_name(my_authority);
    UUri method = getRpcUUri(12);
    method.set_authority_name(my_authority);

    auto transport = std::make_shared<ZenohUTransport>(source, "zenoh_config_transl_test.json5");
    auto server = RpcServer::create(transport, method, OnReceive);

    if (!server.has_value()) {
        spdlog::error("Failed to create RPC server: {}",
                      server.error().DebugString());
        return 1;
    }

    while (!gTerminate) {
        // spdlog::info("wait loop ...");
        sleep(1);
    }

    return 0;
}
