# uCentralGW

## What is this?
The uCentralGW is an added service for the TIP controller that allows integration with the 
uCentral protocol. It is meant to run as a pod inside a TIP controller container. It should 
have a single port open to provide connection from uCentral enabled devices.

## Building
In order to build the uCentralGW, you will need to install some of its dependencies, which includes 
the following:

- cmake
- boost
- POCO 1.10.1 or later
- a C++17 compiler
- libyaml
- openssl
- libpq-dev (PortgreSQL development libraries)
- mysql-client (MySQL client)

## About the build
The build is done in 2 parts. The first part is to build a local copy of the framework tailored to your environment. This 
framework is called [Poco](https://github.com/pocoproject/poco). The version used in this project has a couple of fixes
from the master copy needed for cmake. Please use the version of this [Poco fix](https://github.com/stephb9959/poco).

### Ubuntu build
These instructions have proven to work on Ubuntu 20.4.
```
sudo apt install git cmake g++ libssl-dev libmysqlclient-dev unixodbc-dev postgesql-client-dev 
sudo apt install libpq-dev libaprutil1-dev apache2-dev libboost-all-dev libyaml-cpp-dev

git clone https://github.com/stephb9959/poco
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/stephb9959/ucentralgw
cd ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make
```

### Fedora Build
The following instructions have proven to wotk on Fedora 33
```
yum install cmake g++ openssl-devel unixODBC-devel mysql-devel mysql apr-util-devel boost boost-devel
yum install yaml-cpp-devel 
sudo dnf install postgresql.x86_64
sudo dns install postgresql-devel

git clone https://github.com/stephb9959/poco
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/stephb9959/ucentralgw
cd ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make

```
 
### OSX Build
The following instructions have proven to work on OSX Big Sur. You need to install [Homebrew](https://brew.sh/). You must also have installed [XCode for OS X](https://www.freecodecamp.org/news/how-to-download-and-install-xcode/).
```
brew install openssl
brew install cmake
brew install libpq
brew install mysql-client
brew install apr
brew install apr-util
brew install boost
brew install yaml-cpp
brew install postgresql
brew install unixodbc

git clone https://github.com/stephb9959/poco
cd poco
mkdir cmake-build 
cd cmake-build
cmake ..
cmake --build . --config Release -j
sudo cmake --build . --target install

cd ~
git clone https://github.com/stephb9959/ucentralgw
cd ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make -j
```

### Raspberry PI Build
The build on a rPI takes a while. You can shorten that build time and requirements by disabling all the larger database 
support. You can build with only SQLite support by not installing the packages for ODBC, PostgreSQL, and MySQL by 
adding -DSMALL_BUILD=1 on the cmake build line.

```
sudo apt install git cmake g++ libssl-dev libaprutil1-dev apache2-dev libboost-all-dev libyaml-cpp-dev
git clone https://github.com/stephb9959/poco
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/stephb9959/ucentralgw
cd ucentralgw
mkdir cmake-build
cd cmake-build
cmake -DSMALL_BUILD=1 ..
make
```


### Configuration
The configuration for this service is kept in a properties file. Currently, this configuration file must be kept in the 
current directory of uCentral or one level up. This file is called `ucentral.properties`. The file will be loaded from 
the directory set by the environment variable `UCENTRAL_CONFIG`. To use environment variables in the configuration,
you must use `$<varname>`. The path for the logs for the service must exist prior to starting the 
service. The path is defined under `logging.channels.c2.path`. Only `path names` support the use of 
environment variables. Here is a sample configuration:

```
########################################################################
########################################################################
########################################################################
########################################################################
#
# TIP Portal API access. To be ignored in non TIP uCentral deployments
#
########################################################################
########################################################################
########################################################################
tip.certs.key = $UCENTRAL_ROOT/certs/clientkey.pem
tip.certs.cert = $UCENTRAL_ROOT/certs/clientcert.pem
tip.certs.ca = $UCENTRAL_ROOT/certs/clientcert.pem
tip.certs.password = mypassword
tip.api.login.username = support@example.com
tip.api.login.password = support
tip.api.host = debfarm1-node-a1.arilia.com
tip.api.port = 9051
tip.gateway.host.0.address = *
tip.gateway.host.0.port = 9031
tip.gateway.host.0.key = $UCENTRAL_ROOT/certs/ws-key.pem
tip.gateway.host.0.cert = $UCENTRAL_ROOT/certs/ws-cert.pem
tip.gateway.host.0.password = mypassword
#
# uCentral - TIP Gateway Bridge
#
ucentral.tipgateway.host.0.address = *
ucentral.tipgateway.host.0.port = 14001
ucentral.tipgateway.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.tipgateway.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
ucentral.tipgateway.host.0.key.password = mypassword
########################################################################
########################################################################
########################################################################

########################################################################
########################################################################
#
# Thw following sections apply to the uCentral service
#
# Logging: please leave as is for now.
#
########################################################################
########################################################################

logging.formatters.f1.class = PatternFormatter
logging.formatters.f1.pattern = %s: [%p] %t
logging.formatters.f1.times = UTC
logging.channels.c1.class = ConsoleChannel
logging.channels.c1.formatter = f1
logging.channels.c2.class = FileChannel
# This is where the logs will be written. This path MUST exist
logging.channels.c2.path = $UCENTRAL_ROOT/logs/sample.log
logging.channels.c2.formatter.class = PatternFormatter
logging.channels.c2.formatter.pattern = %Y-%m-%d %H:%M:%S %s: [%p] %t
logging.channels.c3.class = ConsoleChannel
logging.channels.c3.pattern = %s: [%p] %t
# External Channel
logging.loggers.root.channel = c2
logging.loggers.root.level = information
# Inline Channel with PatternFormatter
# logging.loggers.l1.name = logger1
# logging.loggers.l1.channel.class = ConsoleChannel
# logging.loggers.l1.channel.pattern = %s: [%p] %t
# logging.loggers.l1.level = information
# SplitterChannel
# logging.channels.splitter.class = SplitterChannel
# logging.channels.splitter.channels = l1,l2
# logging.loggers.l2.name = logger2
# logging.loggers.l2.channel = splitter
openSSL.client.privateKeyFile = $UCENTRAL_ROOT/certs/clientkey.pem
openSSL.client.certificateFile = $UCENTRAL_ROOT/certs/clientcert.pem
openSSL.client.caConfig = $UCENTRAL_ROOT/certs/cacert.pem
openSSL.client.verificationMode = once
openSSL.client.verificationDepth = 9
openSSL.client.loadDefaultCAFile = true
openSSL.client.cypherList = ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH
openSSL.client.privateKeyPassphraseHandler.name = KeyFileHandler
openSSL.client.privateKeyPassphraseHandler.options.password = mypassword
openSSL.client.invalidCertificateHandler = AcceptCertificateHandler
openSSL.client.invalidCertificateHandler.options.ignoreError = true
openSSL.client.extendedVerification = false
openSSL.client.cacheSessions = true
openSSL.client.requireTLSv1 = true

#
# uCentral protocol server for devices. This is where you point
# all your devices.
#
ucentral.websocket.host.0.address = *
ucentral.websocket.host.0.port = 15002
ucentral.websocket.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.websocket.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
ucentral.websocket.host.0.key.password = mypassword
ucentral.websocket.maxreactors = 5

#
# REST API access
#
ucentral.restapi.host.0.address = *
ucentral.restapi.host.0.port = 16001
ucentral.restapi.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.restapi.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
ucentral.restapi.host.0.key.password = mypassword

#
# This section descrive how to do autoprovisioning
# When enabled, it will allow devices that are not in the system
# to be managed and serviced
#
ucentral.autoprovisioning = true
ucentral.autoprovisioning.type.0 = AP:ea8300,edge
ucentral.autoprovisioning.type.1 = IOT:ea8301,edge2
ucentral.autoprovisioning.type.2 = AP:ea8302,edge6


#
# This section select which form of persistence you need
# Only one selected at a time. If you select multiple, this service will die if a horrible
# death and might make your beer flat.
#
storage.type = sqlite
#storage.type = postgresql
#storage.type = mysql
#storage.type = odbc

storage.type.sqlite.db = $UCENTRAL_ROOT/devices.db
storage.type.sqlite.idletime = 120
storage.type.sqlite.maxsessions = 128

storage.type.postgresql.maxsessions = 64
storage.type.postgresql.idletime = 60
storage.type.postgresql.host = localhost
storage.type.postgresql.username = stephb
storage.type.postgresql.password = snoopy99
storage.type.postgresql.database = ucentral
storage.type.postgresql.port = 5432
storage.type.postgresql.connectiontimeout = 60

storage.type.mysql.maxsessions = 64
storage.type.mysql.idletime = 60
storage.type.mysql.host = localhost
storage.type.mysql.username = stephb
storage.type.mysql.password = snoopy99
storage.type.mysql.database = ucentral
storage.type.mysql.port = 3306
storage.type.mysql.connectiontimeout = 60

#
# Authentication
#
authentication.enabled = true
authentication.default.username = support@example.com
authentication.default.password = support
authentication.service.type = internal
```

#### Important config entries
##### This is the logging directory
- logging.channels.c2.path = $UCENTRAL_ROOT/logs/sample.log

##### This is the type of storage in use
- storage.type = sqlite

##### Autoprovisioning settings
- ucentral.autoprovisioning = true
- ucentral.autoprovisioning.type.0 = AP:ea8300,edge
- ucentral.autoprovisioning.type.1 = IOT:ea8301,edge2
- ucentral.autoprovisioning.type.2 = AP:ea8302,edge6

##### This is the RESTAPI endpoint
- ucentral.restapi.host.0.address = *
- ucentral.restapi.host.0.port = 16001
- ucentral.restapi.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
- ucentral.restapi.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem

##### This is the end point for the devices
- ucentral.websocket.host.0.address = *
- ucentral.websocket.host.0.port = 15002
- ucentral.websocket.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
- ucentral.websocket.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem

## Device connection to the controller
The devices use the WebSocket protocol to establish a connection to the uCentral controller. While establishing the connection, the device mu include the following header in its connection: `Sec-WebSocket-Protocol: ucentral-broker`. In the [JSON-RPC](https://www.jsonrpc.org/specification) scenario, the AP is considered the server. So the Controller sends commands to the AP using JSON-RPC, and the AP will send notifications to the controller. 

### Event Messages
In this RPC, here are some common interpretations:
- `when` : In a command, this is a suggestion as to when to perform somthing. 0 means right now, or otherwise the UTC time in seconds.
- `serial` : This is the text representation of the serial number the device is using. Usually will be the MAC address of the device without an separator.
- `uuid` : Is an int64 representing the current configuration ID.
- `JSON documents` : when a field requires an JSON document, this is free form JSON and the controller and the AP agree on its content.
- 
#### Connection event
AP Sends connection notification to the controller after establishing a connection. The controller
my decide to send the AP a newer configuration if it has a newer one. 
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
AP Sends device state periodically. If the Contoller detects that it has a newer configuration, it 
may decide to send this new configuration to the AP.
```
{   "jsonrpc" : "2.0" , 
    "method" : "state" , 
    "params" : {
	"serial" : <serial number> ,
	"uuid" : <current active configuration uuid>,
	"state" : <JSON Document: current device state.>
  }
}
```

#### Healthcheck event
AP Sends `healthcheck` periodically. This message contains information how vital AP subsystems are operating.
```
{   "jsonrpc" : "2.0" , 
    "method" : "healthcheck" , 
    "params" : {
	"serial" : <serial number> ,
	"uuid" : <current active configuration uuid>,
	"sanity: <integer representing a percentage level of operation. 0 - device is dead 100 - all perfect.>
	"data" : <JSON Document: current device healthcheck.>
  }
}
```

#### Log event
AP Sends a log whenever necessary. The controller will log this message.
```
{   "jsonrpc" : "2.0" , 
    "method" : "log" , 
    "params" : {
	"serial" : <serial number> ,
	"log" : <text to appear in the logs>
	"severity" : <as stated below>,
	"data" : <JSON Document (optional): related to this log message>
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

##### `data`
This is optional data that may be added in the log message. It must be a JSON document. 

#### Config change pending event
AP Sends a log whenever necessary. This message is intended to tell the controller that the AP 
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
AP Sends a keepalive whenever necessary. The AP will send this message to tell the controller 
which version it is running. The Controller may decide to send the AP a newer configuration.
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

#### Controller wants the AP to apply a given configuration
Controller sends this command when it believes the AP should load a new configuration. The AP
should send messages with `pending change` events until this version has been applied.
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

The AP should answer:
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
	     			{ "parameter" : <JSON Document: text that caused the rejection> ,
				  "reason" : <why it was rejected>,
				  "substitution" : <JSON Document: replaced by this JSON. Optional> } ...
			  ]
         },
     "id" : <same number>
}
```
##### The AP Answer
The AP can answer and tell the controller it has rejected certain parts of the config and potentially replaced them with 
appropriate values. This could be used to allow an AP to replace frequencies for the regions it is localted. The AP can provide an
array of these rejections. The substitution JSON is optional.

###### Error codes
- 0 : configuration was applied as-is.
- 1 : configuration was applied with the included substitutions in the `rejected` section. The device is operating with the new modified config.
- 2 : configuration was rejected and will not be applied at all. The `rejected` section can be used to tell the controller why. 

###### The `rejected` section
The rejected section is an array containing the following:
- `parameter` : the JSON code in the config that is causing this rejection
- `reason` : anything to explain the rejection.
- `substution` : the JSON code that `parameter` was replaced with. This could be absent meaning that the `parameter` code was simply removed from the configuration. 

#### Controller wants the AP to reboot
Controller sends this command when it believes the AP should reboot.
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

The AP should answer:
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

#### Controller wants the AP to upgrade its firmware
Controller sends this command when it believes the AP should upgrade its firmware.
```
{    "jsonrpc" : "2.0" , 
     "method" : "upgrade" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : Optional - <UTC time when to upgrade the firmware, 0 mean immediate, this is a suggestion>,
		"uri" : <URI to download the firmware>
     },
     "id" : <some number>
}
```

The AP should answer:
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

#### Controller wants the AP to perform a factory reset
Controller sends this command when it believes the AP should upgrade its firmware.
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

The AP should answer:
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

#### Controller wants the AP to flash its LEDs
Controller sends this command when it wants the AP to flash its LEDs.
```
{    "jsonrpc" : "2.0" , 
     "method" : "factory" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : Optional - <UTC time when to upgrade the firmware, 0 mean immediate, this is a suggestion>,
		"duration" : number in milliseconds
     },
     "id" : <some number>
}
```

The AP should answer:
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
- 0 : device will perform blink at `when` seconds.
- 1 : device cannot flash LEDs because it does not have any.
- 2 : device rejects the request. `text` should include information as to why. 

#### Controller sends a device specific command
Controller sends this command specific to this AP. The command is proprietary and must be agreed upon by the AP and the Controller. 
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

The AP should answer:
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
      },
      "id" : <same number>
}
```
##### The AP answer
The AP should answer with teh above message. The `error` value should be interpreted the following way:
- 0 : the command was performed as requested and the reults of the command is available in the `resultCode` and `resultText` parameters.
- 1 : the command will be performed in the future and `when` shows that time. The `resultCode` and `resultText` dod not contain anything relevant.
- 2 : the command cannot be performed as indicated. `resultCode` and `resultText` may contain some indication as to why.

### Message compression
Some messages may be several KB in size. If these messages repeat often, they may cause added data charges over time. As a result, the AP may decide to compress and base64 outgoing messages. Only messages over 3K in size should be compressed. This should apply to the `state` event and possibly the `healtcheck` event. Should other messages get larger, the client may decide to compress the. Only messages from the AP to the controller may use compression.

#### Identifying a compressed message
A compressed message has a single member to the `params` field. It's only parameter must be called `compress_64`. Any other elements under
params will be dropped.

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



## OpenAPI
The service supports an OpenAPI REST based interface for management. You can find the [definition here](https://github.com/stephb9959/ucentralgw/blob/main/tipapi/ucentral/ucentral.yaml).

## Using the API
In the `test_scripts`, you will find a series of scripts that will show you how to use the API with curl. More scripts will be added in the future.

## Certificates
Love'em of hate'em, we gotta use'em. So we tried to make this as easy as possible for you. Under the `cert_scripts` you 
can run a single command that will generate all the files you need. By default, this will generate the server side
of the certificates as well as certificates for 10 devices. You can change the variabla `howmany` in the script
to change that number. Once you run this script, you will get:

- `server-key.pem` : this file is the server key and should be used in the `ucentral.properties` file.
- `server-cert.pem` : this is the certificate to be used with the generated key. This should also be used in `ucentral.properties`.
- `dev-1-cert.pem` .. `dev-10-cert.pem` : certificates ot be used on the actual devices in the `/etc/config` directories of the devices.

The script `more_devices` can be used to generate more devices without regenerating the original key. Just change the `finish` variable to the number you need.

## Command line options
The current implementation supports the following

```
./ucentral --help
usage: ucentral OPTIONS
A uCentral gateway implementation for TIP.

--daemon        Run application as a daemon.
--umask=mask    Set the daemon's umask (octal, e.g. 027).
--pidfile=path  Write the process ID of the application to given file.
--help          display help information on command line arguments
--file=file     specify the configuration file
--debug         to run in debug, set to true
--logs=dir      specify the log directory and file (i.e. dir/file.log)
```

#### file
This allows you to point to another file without specifying the UCENTRAL_CONFIG variable. The file name must end in `.properties`.
#### daemon
Run this as a UNIX service
#### pidfile
When running as a daemon, the pid of the running service will be set in the speficied file
#### debug
Run the service in debug mode.
#### logs
Speficy where logs should be kept. You must include an existing directory and a file name. For example `/var/ucentral/logs/log.0`.
#### umask
Seet the umask for the running service.
 

