/*********************************************************************
* Copyright (c) 9-7-2024 Cummins Inc
*
* This program and the accompanying materials are made
* available under the terms of the Eclipse Public License 2.0
* which is available at https://www.eclipse.org/legal/epl-2.0/
*
* SPDX-License-Identifier: EPL-2.0
**********************************************************************/

#include "include/Application.h"

using namespace std;

void Application::Init()
{
     //Modify function/class as per need. 
     payloadResponseMap={
        {global::subscribe::OPEN_COMM_CHANNEL,[this](const std::string& msg){return uds->OpenCommunicationChannel(msg);}}
    };
}

void Application::ProcessMessage(const std::string &topic,
                                 const std::string &payload)
{
    if (payloadResponseMap.find(topic) != payloadResponseMap.end())
    {
        google::protobuf::Struct response = payloadResponseMap[topic](payload);
        std::string finalResponse;
        response.SerializeToString(&finalResponse);
        SendMessageonInput(global::publish::OPEN_COMM_CHANNEL, finalResponse);
        
    }
    else
    {
        //spdlog::debug("Request topic not found.");
    }
}

std::string Application::OnIncomingMessage(const std::string &topic,
                                           const std::string &payload)
{
    ProcessMessage(topic, payload);
    return "";
}


