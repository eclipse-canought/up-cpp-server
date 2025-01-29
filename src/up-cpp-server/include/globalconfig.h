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
#define USE_TRANSLATOR 0

#include <vector>
#include <string>

#include "mqtt/client.h"
#include "mqtt/async_client.h"
#include "apiconstants.h"
#include "imqtt-wrapper.h"
#include <fmt/format.h>

/** @brief Subscription topics
 * Sample topics added as example. Modify as per neeed.
 */
namespace application
{
	const std::array<std::string, 1> Subscriptiontopics = { global::subscribe::OPEN_COMM_CHANNEL
														  };
	
	// Populate the below list with your application specific topic attribute														
	const std::vector<std::string> list = {
		"diagstack"
	};

}