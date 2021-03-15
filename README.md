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
The following instructions have proven to work on OSX Big Sur. You need to install [Homebrew](https://brew.sh/)
```
brew install openssl
brew install libyaml
brew install cmake
brew install libpq
brew install mysql-client
brew install apr
brew install apr-util
brew install boost

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

### Configuration
The configuration for this service is kept in a properties file. Currently, this configuration file must be kept in the 
current directory of uCentral or one level up. This file is called `ucentral.properties`. The file will be loaded from 
the directory set by the environment variable `UCENTRAL_CONFIG`. To use environment variables in the configuration,
you must use `${<varname>}`. The path for the logs for the service must exist prior to starting the 
service. The path is defined under `logging.channels.c2.path`. Only `path names` support the use of 
environment variables. Here is a sample and the important entries:

```
tip.certs.key = ${UCENTRAL_ROOT}/certs/clientkey.pem
tip.certs.cert = ${UCENTRAL_ROOT}/certs/clientcert.pem
tip.certs.ca = ${UCENTRAL_ROOT}/certs/clientcert.pem
tip.certs.password = mypassword
tip.api.login.username = support@example.com
tip.api.login.password = support
tip.api.host = debfarm1-node-a1.arilia.com
tip.api.port = 9051

tip.gateway.host.0.address = *
tip.gateway.host.0.port = 9031
tip.gateway.host.0.key = ${UCENTRAL_ROOT}/certs/ws-key.pem
tip.gateway.host.0.cert = ${UCENTRAL_ROOT}/certs/ws-cert.pem
tip.gateway.host.0.password = mypassword

logging.formatters.f1.class = PatternFormatter
logging.formatters.f1.pattern = %s: [%p] %t
logging.formatters.f1.times = UTC
logging.channels.c1.class = ConsoleChannel
logging.channels.c1.formatter = f1
logging.channels.c2.class = FileChannel
logging.channels.c2.path = ${UCENTRAL_ROOT}/logs/sample.log
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
openSSL.client.privateKeyFile = ${UCENTRAL_ROOT}/certs/clientkey.pem
openSSL.client.certificateFile = ${UCENTRAL_ROOT}/certs/clientcert.pem
openSSL.client.caConfig = ${UCENTRAL_ROOT}/certs/cacert.pem
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

ucentral.websocket.host.0.address = *
ucentral.websocket.host.0.port = 15002
ucentral.websocket.host.0.cert = ${UCENTRAL_ROOT}/certs/ws-cert.pem
ucentral.websocket.host.0.key = ${UCENTRAL_ROOT}/certs/ws-key.pem
ucentral.websocket.host.0.key.password = mypassword
ucentral.websocket.maxreactors = 5

ucentral.restapi.host.0.address = *
ucentral.restapi.host.0.port = 16001
ucentral.restapi.host.0.cert = ${UCENTRAL_ROOT}/certs/ws-cert.pem
ucentral.restapi.host.0.key = ${UCENTRAL_ROOT}/certs/ws-key.pem
ucentral.restapi.host.0.key.password = mypassword

ucentral.tipgateway.host.0.address = *
ucentral.tipgateway.host.0.port = 14001
ucentral.tipgateway.host.0.cert = ${UCENTRAL_ROOT}/certs/ws-cert.pem
ucentral.tipgateway.host.0.key = ${UCENTRAL_ROOT}/certs/ws-cert.pem
ucentral.tipgateway.host.0.key.password = mypassword

storage.type = sqlite
#storage.type = postgresql
#storage.type = mysql
#storage.type = odbc

storage.type.sqlite.db = ${UCENTRAL_ROOT}/devices.db
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

authentication.enabled = true
authentication.default.username = support@example.com
authentication.default.password = support
authentication.service.type = internal
```

## JSON-RPC based protocol

In the [JSON-RPC](https://www.jsonrpc.org/specification) scenario, the AP is considered the server. So the Controller sends commands to the AP using JSON-RPC, and the AP will send notifications to the controller. 

### Event Messages
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
	  "capabilities" : <current device capabilities in JSON document>
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
	"state" : <current device state in JSON document>
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
    }
}
```

#### Config change pending event
AP Sends a log whenever necessary. This message is intended to tell the controller that the AP 
has received a configuration but is still running an older configuration. The controller will not 
reply to this message.
```
{   "jsonrpc" : "2.0" , 
    "method" : "cfgpending" , 
    "params" : {
        "serial" : <serial number> ,
	"active" : <current active configuration uuid>,
	"uuid" : <waiting to apply this configuration>
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
	"when" : UTC time when to apply this config, 0 mean immediate, this is a suggestion
        "config" : <New configuration as a JSON document”
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
         },
     "id" : <same number>
}
```

#### Controller wants the AP to reboot
Controller sends this command when it believes the AP should reboot.
```
{    "jsonrpc" : "2.0" , 
     "method" : "reboot" , 
     "params" : {
	        "serial" : <serial number> ,
	        "when" : <UTC time when to apply this config, 0 mean immediate, this is a suggestion>
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
	    "text" : <description of the error or success>
  	},
  "id" : <same number>
}
```

#### Controller sends a device specific command
Controller sends this command when it believes the AP should reboot
```
{     "jsonrpc" : "2.0" , 
      "method" : "perform" , 
      "params" : {
          "serial" : <serial number> ,
	  "when" : <UTC time when to apply this config, 0 mean immediate, this is a suggestion>,
	  "command" : <this is device specific>
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
	    "resultCode" : <0 or an appropriate error code>,
	    "resultText" : <any text resulting from the command. This is propeirtary to each command>
      },
      "id" : <same number>
}
```


