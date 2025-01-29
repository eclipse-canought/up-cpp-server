# up-cpp-server
uProtocol C++ server for CAN Translator
uProtocol C++ server is designed to work synchronously with uProtocol C++ client. 
The server is supposed to respond to the client protobuf message request based on the sequence and the service needed by the client.
This server simulator container Application contains dev container dockerfile that can be utilized for debugging and testing using VS code on the local machine.

Getting Started

## Building docker images

To generate the first base images use docker-compose to generate dev and prod images
docker-compose -f opensource/docker-compose.yaml build

## Test and Deploy

To execute unit tests please run the shell script runUT.sh after generating the base image
./runUT.sh

## If using VSCode
For debugging you can execute cleanup task followed by build task which will deploy container services mentioned under docker-compose.dev.yml. Modify Launch and Task.json as per need for debugging application and test suite.
Tip : Use docker exec command to run application inside the created container
     set flags 
    -DCMAKE_BUILD_TYPE -DENABLE_TESTING -DAPP_NAME:STRING as needed.
change type from CRLF to LF during script execution.
## Mosquitto conf changes required to run both client and server 
When running client and server together the mosquitto.conf file located at mosquiito/config needs to be modified to connect to the mosquitto_simulator bridge and the changes need to be committed to the mosquiito container referred in the client compose.Modify the mosquiito/config/mosquitto.conf file on client end to accomodate below changes.
1. Under "Extra listner" section add below command for listener port-number [ip address/host name]
	listener 1883 [ip address of mosquitto_simulator x.x.x.x. Run docker ps command to get the ip]
2. Under "Bridges" section add the following commands
	connection [bridge name]
	address mosquitto_simulator:1883
	topic # both 0
3. Commit the changes to the client side mosquitto container image using "docker commit [containerid] [imagename]"
   command and launch the container with new image. 