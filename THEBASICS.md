# Understanding the gateway
This will be boring so just imagine having to write it. However, I think this could help you understand everything else.

# The whole uCentral system
So uCentral is a protocol, a schema, and a whole system. No wonder you are confused. Let's split the system in its components:

## The controller
The controller responds to API calls. The APi calls may be used to reboot a device, force a firmware upgrade, set the configuration
for a device, look at the logs for a device. The controller receives connection requests from the devices.

## The devices
The devices are Access Points or Switches (and other forms in the future), that support the uCentral configuration schema. Devices contact
a controller using the uCentral protocol.

## The communication
The communication between the controller and the devices use the uCentral protocol. This protocol is defined in this [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/PROTOCOL.md).

## Device configuration
A device is configured by ingesting a uCentral configuration. That configuration will be provided by the controller as a result of a command through the API
of because the device's configuration is too old for what is known in the controller. The uCentral is a JSON document containing parameters to 
set on a particular device.

## Talking to the controller
In order to speak to the controller, you must implement a client that uses the OpenAPI definition for the gateway. You can find its 
[definition here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/openapi/ucentral/ucentral.yaml). You cannot talk to a device directly. 

## API Basics

### The `serialNumber`
Throughout the API, the `serialNumber` of the device is used as the key. The `serialNumber` is actual the MAC address of the device, without its `:`. The `serialNumber` is
guaranteed to be unique worldwide. The device uses its serial number to identify itself to the controller.

### The configuration
The configuration can be supplied when the device is created. After the device is created, the only way to modify the configuration is by using the `/device/{serialNumber}/configure`
endpoint. The controller maintains the versioning of the configuration through the use of a `uuid`. The controller maintains that number and will ignore anything your supply. The 
controller also does minimum validation on the configuration: it must be a valid JSON document and must have a `uuid` field which will be ignored.

### Device capabilities
Device capabilities are uploaded to the controller when the device does its initial connection. Capabilities tell the controller what the device is able to support. The controller
can use this information in order to provide a better configuration for the device. 

### The command queue
The controller will send commands to the devices. These commands are kept in a table and are sent at the appropriate time or immediately when the connects. For example, you could ask a device to 
change its configuration, however it is unreachable. So the next time the device connects, this configure command will be sent at that time. You can get the list of commands using
the `/commands` endpoint. 

### The commands
Several commands maybe sent to a device: reboot, configure, factory reset, firmware upgrade, LEDs, trace, message request, etc. The APi endpoint `/device/{serialNumber}/{command}` details all the available commands.

### Device specific collections
For each device, a number of collections are collected and kept in the database. Here's a brief list:
- `logs`: device specific logs are kept. A device amy also send something it wants added into its own logs. `crashlogs` are a special type of logs created after a device has had a 
    hard crash.
- `statistics`: statistics about the device. This is current la JSON document and will be documented at a later date.
- `healthchecks`: periodically, a device will run a self-test and report its results. These includes anything that maybe going wrong with the current device configuration. A `sanity` level is associated to the degree of health of the device. 100 meaning a properly operating device.
- `status`: tells you where the device is and how much data is used for protocol communication.

## The API is for an operator
This API is really meant for an operator who would have to help a subscriber in configuring her devices, reboot, manage the firmware, etc. The API should be the lowest level 
anyone should go in managing a device. Someone could be an entire `firmware` management system with the existing API. 



