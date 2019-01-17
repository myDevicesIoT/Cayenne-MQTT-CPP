# Cayenne MQTT C++ Library
The Cayenne MQTT C++ Library provides functions to easily connect to the Cayenne IoT project builder.

This library bundles the [Eclipse Paho MQTT C/C++ client](https://github.com/eclipse/paho.mqtt.embedded-c).

## Repository Structure
- **src** - The library source code.
  - **CayenneMQTTClient** - Platform independent Cayenne C++ library using the Paho MQTT C++ library. To create platform specific versions of this library networking and timer code for the platform are required.
  - **CayenneUtils** - Common code for creating and parsing Cayenne topics and payloads. This code can be used with any MQTT client.
  - **MQTTCommon** - Common Paho MQTT C code.
  - **Platform** - Platform specific networking and timer code, as well as test and example applications.
    - **Linux** - Linux C++ networking and timer code, as well as test and example applications. Test and example files can be built using the makefile.
    - **Windows** - Windows C++ networking and timer code, as well as test and example applications. Test and example files can be built using Visual Studio 2017.

## Building Examples
To build on Linux switch to the root folder and run: `make`

To build on Windows load the Visual Studio 2017 solution file at `src\Platform\Windows\Cayenne.sln` and build the project.

## Adding Additional Platforms
The Cayenne MQTT client code is platform independent but it requires platform specific code to create timers and to read and write data over the network. To add support for additional platforms you will need to create platform specific timer and networking code.
  - **Timer** - This class is used to create countdown timers. It is described in the [TimerInterface.h](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP/blob/master/src/CayenneMQTTClient/TimerInterface.h) file. An example implementation for Linux is in the [Linux/MQTTTimer.h](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP/blob/master/src/Platform/Linux/MQTTTimer.h) file.
  - **Network** - This class is used to read and write data over the network. It is described in the [NetworkInterface.h](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP/blob/master/src/CayenneMQTTClient/NetworkInterface.h) file. An example implementation for Linux is in the [Linux/MQTTNetwork.h](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP/blob/master/src/Platform/Linux/MQTTNetwork.h) file.
  
After creating new platform specific timer and networking classes you can use them by passing them as template parameters to the [MQTTClient](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP/blob/master/src/CayenneMQTTClient/CayenneMQTTClient.h) class.

## Cayenne MQTT Libraries
- **[Cayenne-MQTT-C](https://github.com/myDevicesIoT/Cayenne-MQTT-C)** - C version of the Cayenne MQTT Library.
- **[Cayenne-MQTT-CPP](https://github.com/myDevicesIoT/Cayenne-MQTT-CPP)** - C++ version of the Cayenne MQTT Library.
- **[Cayenne-MQTT-Arduino](https://github.com/myDevicesIoT/Cayenne-MQTT-Arduino)** - Arduino version of the Cayenne MQTT Library.
- **[Cayenne-MQTT-mbed](https://github.com/myDevicesIoT/Cayenne-MQTT-mbed)** - mbed version of the Cayenne MQTT Library. This is also available on the mbed developer site [here](https://developer.mbed.org/teams/myDevicesIoT/code/Cayenne-MQTT-mbed/).
