/*********************************************************************
* Copyright (c) 9-7-2024 Cummins Inc
*
* This program and the accompanying materials are made
* available under the terms of the Eclipse Public License 2.0
* which is available at https://www.eclipse.org/legal/epl-2.0/
*
* SPDX-License-Identifier: EPL-2.0
**********************************************************************/
#pragma once
#include <functional>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/json_util.h>
#include "apiconstants.h"
#include "iintercontainer-messenger.h"
#include "translator-protobuf.pb.h"

class UDS
{
    std::string connectionId;
public:
    // UDS constructor
    explicit UDS() {

    };

    /** @brief - OpenCommunicationChannel method opens a connection with requested ecu
     * @param None - No input parameters
     * @return jsonType - returns payload with connection details.
     */
    google::protobuf::Struct OpenCommunicationChannel(const std::string &message);
};
class J1939
{
    std::string connectionId;
public:
    explicit J1939()
    {
    }
};
