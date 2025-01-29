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

#include <map>
#include <queue>
#include <thread>
#include "mqtt/async_client.h"
#include "mqtt/client.h"
#include "imqtt-wrapper.h"
#include "apiconstants.h"
#include "globalconfig.h"
#include "observer.h"
#include "translator.h"


class Application : public Observer
{
public:

    //Application constructor
    explicit Application(std::shared_ptr<InterContainerMessenger> Client) : Observer(Client){
        uds = std::make_unique<UDS>();
        j1939 = std::make_unique<J1939>();
    };

    //default application destructor
    ~Application()  = default;

    /** @brief Init method to execute all initialization operations.
     * @return void
     */

    void Init() override;

    /** @brief Read incoming messages from client and process it
     * @param  topic - server subscribed topic in string format
     * @param  payload - client request payload in string format
     * @return bool - Returns true
     */

    std::string OnIncomingMessage(const std::string &topic,const std::string &payload);
    void ProcessMessage(const std::string &topic,
                                 const std::string &payload);
private:
    std::unique_ptr<UDS> uds;
    std::unique_ptr<J1939> j1939;
    std::unordered_map<std::string,std::function<google::protobuf::Struct(const std::string&)>>payloadResponseMap;
};
