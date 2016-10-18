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
