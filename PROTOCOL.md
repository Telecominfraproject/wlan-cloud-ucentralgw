# The uCentral communication protocol
In order to operate in a uCentral system, devices and services must operate using a specific protocol. This protocol describes 
the interaction between the managed devices and the controller that manages the devices. This communictaion protocol
uses the [WebSocket protocol](https://tools.ietf.org/html/rfc6455) on port 15002.

## The controller
The controller is an application waiting for connection from devices on a given port. The controller waits for connections
over TCP. The controller can operate over IPv4 in this initial version. An IPv6 version will exist later. The controller
is able to support thousands of devices.

## The devices
A device will be given a way to find the IP address of its controller as well s the port to communicate over. Devices always initiate 
the connection to the controller.

## The connection
The connection uses the WebSocket protocol to maintain and frame messages. The communication protocol used is modeled after JSON-RPC for sending JSON structured messages.

## The messages
The controller can send commands to the devices. These commands must be acknowledged by the devices. Any errors should be reported byu the devices. The devices may send unsolicited 
messages to the devices. These messages are not acknowledged by the controller and should be expected to be received and processed. The next section explains the different messages.  

### Event Messages
In this RPC, here are some common interpretations:
- `when` : In a command, this is a suggestion as to when to perform something. 0 means right now, or otherwise the UTC time in seconds.
- `serial` : This is the text representation of the serial number the device is using. Usually will be the MAC address of the device without an separator.
- `uuid` : Is an int64 representing the current configuration ID.
- `JSON documents` : when a field requires an JSON document, this is free form JSON and the controller and the AP agree on its content.
-
#### Connection event
Device Sends connection notification to the controller after establishing a connection. The controller
my decide to send the AP a newer configuration. The controller will record the device capabilities provided.
```
{     "jsonrpc" : "2.0" , 
      "method" : "connect" , 
      "params" : {
        "serial" : <serial number> ,
        "uuid" : <current active configuration uuid>,
        "firmware" : <Current firmware version string>,
        "capabilities" : <JSON Document: current device capabilities.>
    }
}
```

#### State event
The device sends device state information periodically. If the controller detects that it has a newer configuration, it
may decide to send this new configuration to the AP.
```
{   "jsonrpc" : "2.0" , 
    "method" : "state" , 
    "params" : {
        "serial" : <serial number> ,
        "uuid" : <current active configuration uuid>,
	"request_uuid" : <optional string identifying the request that triggered this state event. The absence(or empty) means that this is a normal scheduled state>
        "state" : <JSON Document: current device state.>
      }
}
```

#### Healthcheck event
Device sends a `healthcheck` periodically. This message contains information about how vital subsystems are operating and 
if they need attention.
```
{   "jsonrpc" : "2.0" , 
    "method" : "healthcheck" , 
    "params" : {
        "serial" : <serial number> ,
        "uuid" : <current active configuration uuid>,
	"request_uuid" : <optional string identifying the request that triggered this healthcheck. The absence(or empty) means that this is a normal scheduled healthcheck>
        "sanity: <integer representing a percentage level of operation. 0 - device is dead 100 - all perfect.>
        "data" : <Optiona/may be empty: JSON Document about current device healthcheck.>
      }
}
```

#### Log event
Device sends a log message whenever necessary. The controller will log this message to the log system for the device.
```
{   "jsonrpc" : "2.0" , 
    "method" : "log" , 
    "params" : {
        "serial" : <serial number> ,
        "log" : <text to appear in the logs>
        "severity" : <as stated below>,
        "data" : <Optiona/may be empty: JSON Document providing additional information related to this log message>
    }
}
```

##### `severity`
The `severity` matches the `syslog` levels. Here are the details:
- 0 : LOG_EMERG       0       /* system is unusable */
- 1 : LOG_ALERT       1       /* action must be taken immediately */
- 2 : LOG_CRIT        2       /* critical conditions */
- 3 : LOG_ERR         3       /* error conditions */
- 4 : LOG_WARNING     4       /* warning conditions */
- 5 : LOG_NOTICE      5       /* normal but significant condition */
- 6 : LOG_INFO        6       /* informational */
- 7 : LOG_DEBUG       7       /* debug-level messages */

#### Crash Log event
Device may send a crash log event after rebooting after a crash. The event cannot be sent until a connection event has been sent.
```
{   "jsonrpc" : "2.0" , 
    "method" : "crashlog" , 
    "params" : {
        "serial" : <serial number> ,
        "uuid"   : <the UUID of the configuration that generated the crash log>,
        "loglines" : [ an array of strings representing the logs from the log file ]
      }
}
```

#### Config change pending event
Device sends this message to tell the controller that the device 
has received a configuration but is still running an older configuration. The controller will not
reply to this message.
```
{   "jsonrpc" : "2.0" , 
    "method" : "cfgpending" , 
    "params" : {
        "serial" : <serial number> ,
        "active" : <UUID current active configuration uuid>,
        "uuid" : <UUID waiting to apply this configuration>
    }
}
```

#### Send a keepalive to the controller event
Device sends a keepalive whenever necessary. The device will send this message to tell the controller
which version it is running. The Controller may decide to send the device a newer configuration.
```
{   "jsonrpc" : "2.0" , 
    "method" : "ping" , 
    "params" : {
        "serial" : <serial number> ,
        "uuid" : <current running config>
    }
}
```

### Controller commands
Most controller commands include a `when` member. This is a UTC clock time asking the AP
to perform the command at that time. This is a suggestion only. The AP may ignore this
parameter. If a 0 (zero) is given, the command should be performed immediately. `when` is
always a numeric parameter.

#### Controller wants the device to apply a given configuration
Controller sends this command when it believes the device should load a new configuration. The device
should send message with `pending change` events until this version has been applied and running.
```
{   "jsonrpc" : "2.0" , 
    "method" : "configure" , 
    "params" : {
        "serial" : <serial number> ,
        "uuid" : <waiting to apply this configuration>,
        "when" : Optional - <UTC time when to apply this config, 0 mean immediate, this is a suggestion>
        "config" : <JSON Document: New configuration”
     },
     "id" : <some number>
}
```

The device should answer:
```
{    "jsonrpc" : "2.0" , 
     "result" : {
         "serial" : <serial number> ,
         "uuid" : <waiting to apply this configuration>,
         "status" : {
             "error" : 0 or an error number,
             "text" : <description of the error or success>
             "when" : <indication as to when this will be performed>,
             "rejected" : [
                            {   "parameter" : <JSON Document: text that caused the rejection> ,
                                "reason" : <why it was rejected>,
                                "substitution" : <JSON Document: replaced by this JSON. Optional> 
                            }
                        ]
             }
         },
     "id" : <same number>
}

```
##### The Answer
The device can answer and tell the controller it has rejected certain parts of the config and potentially replaced them with
appropriate values. This could be used to allow a device to replace frequencies for the regions it is located in. The device 
can provide an array of these rejections. The substitution JSON is optional.

###### Error codes
- 0 : configuration was applied as-is.
- 1 : configuration was applied with the included substitutions in the `rejected` section. The device is operating with the new modified config.
- 2 : configuration was rejected and will not be applied at all. The `rejected` section can be used to tell the controller why.

###### The `rejected` section
The rejected section is an array containing the following:
- `parameter` : the JSON code in the config that is causing this rejection
- `reason` : anything to explain the rejection.
- `substution` : the JSON code that `parameter` was replaced with. This could be absent meaning that the `parameter` code was simply removed from the configuration.

#### Controller wants the device to reboot
Controller sends this command when it believes the device should reboot.
```
{    "jsonrpc" : "2.0" , 
     "method" : "reboot" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : Optional - <UTC time when to reboot, 0 mean immediately, this is a suggestion>
     },
     "id" : <some number>
}
```

The device should answer:
```
{     "jsonrpc" : "2.0" , 
      "result" : {
      "serial" : <serial number> ,
      "status" : {
	    "error" : 0 or an error number,
	    "text" : <description of the error or success>,
	    "when" : <time when this will be performed as UTC seconds>,
  	},
  "id" : <same number>
}
```
###### Error codes
- 0 : is rebooting at `when` seconds.
- 1 : the device is busy but will reboot soon. `text` may indicate why.
- 2 : the device will not reboot. `text` contains information as to why.

#### Controller wants the device to upgrade its firmware
Controller sends this command when it believes the device should upgrade its firmware.
```
{    "jsonrpc" : "2.0" , 
     "method" : "upgrade" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when"  : Optional - <UTC time when to upgrade the firmware, 0 mean immediate, this is a suggestion>,
		    "uri"   : <URI to download the firmware>
     },
     "id" : <some number>
}
```

The device should answer:
```
{     "jsonrpc" : "2.0" , 
      "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>,
          }
  	  },
      "id" : <same number>
}
```

#### Controller wants the device to perform a factory reset
Controller sends this command when it believes the device should upgrade its firmware.
```
{    "jsonrpc" : "2.0" , 
     "method" : "factory" , 
     "params" : {
        "serial" : <serial number> ,
        "when" : Optional - <UTC time when to upgrade the firmware, 0 mean immediate, this is a suggestion>,
        "keep_redirector" : <0 or 1>
     },
     "id" : <some number>
}
```

The device should answer:
```
{     "jsonrpc" : "2.0" , 
      "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>
          }
      },
  "id" : <same number>
}
```

#### Controller wants the device to flash its LEDs
Controller sends this command when it wants the device to flash its LEDs.
```
{    "jsonrpc" : "2.0" , 
     "method" : "leds" , 
     "params" : {
        "serial" : <serial number> ,
        "when" : Optional - <UTC time when to upgrade the firmware, 0 mean immediate, this is a suggestion>,
        "duration" : number in milliseconds (only applies to the "blink" pattern)
        "pattern" : One of "on", "off", or "blink"
     },
     "id" : <some number>
}
```

The device should answer:
```
{     "jsonrpc" : "2.0" , 
      "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>,
          }
  	},
  "id" : <same number>
}
```

###### Error codes
- 0 : device will perform blink at `when` seconds.
- 1 : device cannot flash LEDs because it does not have any.
- 2 : device rejects the request. `text` should include information as to why.

#### Controller sends a device specific command
Controller sends this command specific to this device. The command is proprietary and must be agreed upon by the device 
and the controller.
```
{     "jsonrpc" : "2.0" , 
      "method" : "perform" , 
      "params" : {
          "serial" : <serial number> ,
          "when" : Optional - <UTC time when to perform this command, 0 mean immediate, this is a suggestion>,
          "command" : <this is device specific and is TEXT only>,
          "payload" : <JSON Document: containing additional information about the command>
      },
      "id" : <some number>
}
```

The device should answer:
```
{     "jsonrpc" : "2.0" , 
      "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <in UTC time in seconds>,
            "resultCode" : <0 or an appropriate error code>,
            "resultText" : <any text resulting from the command. This is propietary to each command>
          }
      },
      "id" : <same number>
}
```
##### The device answer
The device should answer with teh above message. The `error` value should be interpreted the following way:
- 0 : the command was performed as requested and the reults of the command is available in the `resultCode` and `resultText` parameters.
- 1 : the command will be performed in the future and `when` shows that time. The `resultCode` and `resultText` dod not contain anything relevant.
- 2 : the command cannot be performed as indicated. `resultCode` and `resultText` may contain some indication as to why.

#### Controller wants the device to perform a trace
Controller sends this command when it needs the device to perform a trace (i.e. tcpdump).
```
{    "jsonrpc" : "2.0" , 
     "method" : "trace" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : Optional - <UTC time when to reboot, 0 mean immediately, this is a suggestion>,
	        "duration" : <integer representing the number of seconds to run the trace>
	        "packets" : <integer for the number of packets to capture>
	        "network" : <string identifying the network to trace>
	        "interface" : <string identifying the interface to capture on>
	        "uri" : <complete URI where to upload the trace. This URI will be available for 30 minutes following a trace request start>
        },
     "id" : <some number>
}
```

The device should answer:
```
{   "jsonrpc" : "2.0" , 
    "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>
  	        }
        },
    "id" : <same number>
}
```

##### About tracing
###### 'packets' and 'duration'
The device can interpret the parameters `packets` and `duration` anyway it wants. Both are optional. The AP could decide to
interpret this as a maximum of `duration` or until `packets` have been received. Or perform an `and` between the two. This
is left to the implementer.

###### 'uri'
The `uri` for file upload is available for 30 minutes following the start of the capture. Once the file has been
uploaded or the timeout occurs, the upload will be rejected.

#### Controller wants the device to perform a WiFi Scan
Controller sends this command when it needs the device to perform a WiFi Scan.
```
{    "jsonrpc" : "2.0" , 
     "method" : "wifiscan" , 
     "params" : {
	        "serial" : <serial number> ,
	        "bands" : [ "2","5","5l","5u",6" ], <optional this is a list of bands to scan: on or more bands >
	        "channels" : [ 1,2,3...] , <optional list of discreet channels to scan >
	        "verbose" : <optional boolean: true or false> (by default false)
        },
     "id" : <some number>
}
```

The device should answer:
```
{   "jsonrpc" : "2.0" , 
    "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>
  	        }
          "scan" : <JSON document detailing the results of the scan>
        },
    "id" : <same number>
}
```

#### Controller requesting a specific message
Controller sends this command when it needs the device to provide a message back ASAP. The currently 
supported messages are "state" and "healthcheck". More messages maybe added later. The messages will
be returned the usual way. The RPC response to this message just says that the request has been accepted and the
message will be returned "soon".
```
{    "jsonrpc" : "2.0" , 
     "method" : "request" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : Optional - <UTC time when to reboot, 0 mean immediately, this is a suggestion>,
	        "message" : "state" or "healthcheck",
		"request_uuid" : <optional UUID string. If present during the request, the next message will also contains this field>
        },
     "id" : <some number>
}
```

The device should answer:
```
{   "jsonrpc" : "2.0" , 
    "result" : {
          "serial" : <serial number> ,
          "status" : {
            "error" : 0 or an error number,
            "text" : <description of the error or success>,
            "when" : <time when this will be performed as UTC seconds>
  	        }
        },
    "id" : <same number>
}
```

##### Scanning: bands or channels
In the command, bands and channels are mutually exclusive. If both parameters are omitted, then the scan will be performed for all bands and all channels.

### Message compression
Some messages may be several KB in size. If these messages repeat often, they may cause added data charges over time. 
As a result, the device may decide to compress and base64 outgoing messages. 
Only messages over 3K in size should be compressed. This should apply to the `state` event and possibly the `healtcheck` event. 
Should other messages get larger, the client may decide to compress the. Only messages from the device to the controller may use compression.

#### Identifying a compressed message
A compressed message has a single member to the `params` field. It's only parameter must be called `compress_64`. Any other elements under
params will be dropped. Additional compression schemes may be developed later.

#### How to compress
The original `params` element should be run through `zlib:compress` and then encoded using base64, and passed as a string. Here is an example
of the completed message. The following should how the `state` event could be compressed:

```
{   "jsonrpc" : "2.0" , 
    "method" : "state" , 
    "params" : {
	    "compress_64" : "kqlwhfoihffhwleihfi3uhfkjehfqlkwhfqkhfiu3hffhkjwehfqkwjehfqwiefkjehq.....qwjqkfhqjwk"
  }
}
```





