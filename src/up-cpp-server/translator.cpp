/*********************************************************************
* Copyright (c) 9-7-2024 Cummins Inc
*
* This program and the accompanying materials are made
* available under the terms of the Eclipse Public License 2.0
* which is available at https://www.eclipse.org/legal/epl-2.0/
*
* SPDX-License-Identifier: EPL-2.0
**********************************************************************/
#include "translator.h"

google::protobuf::Struct UDS::OpenCommunicationChannel(const std::string &message)
{   
        translator::UDSOpenCommChannelRequest opencommrequest;
        std::string sequenceno;
        if(opencommrequest.ParseFromString(message))
        {
            
            sequenceno = opencommrequest.sequenceno();
            spdlog::info("Sequence number is {}",sequenceno);
        }
   	    google::protobuf::Struct responsepayload;
		auto& fields = *responsepayload.mutable_fields();
		fields["appid"].set_string_value("up_cpp_client");
		fields["connectionid"].set_string_value("0x12345678");
		fields["sequenceno"].set_string_value(sequenceno);
		fields["responsecode"].set_string_value("0");
		return responsepayload;
}