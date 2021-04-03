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
The following instructions have proven to work on Fedora 33
```
sudo yum install cmake g++ openssl-devel unixODBC-devel mysql-devel mysql apr-util-devel boost boost-devel
sudo yum install yaml-cpp-devel 
sudo dnf install postgresql.x86_64
sudo dnf install postgresql-devel

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

### Docker
So building this thing from scratch is not your thing? I can't blame you. It takes some patience and 
in the end, there's still more work. Here comes `docker` to the rescue. You can run a docker version following
these instructions. The following is the content of the `docker_run.sh` script you can find
[here](https://github.com/stephb9959/ucentralgw/blob/main/docker_run.sh):

```bash
#!/bin/sh

HUBNAME=tip-tip-wlan-cloud-ucentral.jfrog.io
IMAGE_NAME=ucentralgw
DOCKER_NAME=$HUBNAME/$IMAGE_NAME

CONTAINER_NAME=ucentralgw

#stop previously running images
docker container stop $CONTAINER_NAME
docker container rm $CONTAINER_NAME --force

if [[ ! -d logs ]]
then
    mkdir logs
fi

if [[ ! -d certs ]]
then
  echo "certs directory does not exist. Please create and add the proper certificates."
  exit 1
fi

if [[ ! -f ucentral.properties ]]
then
  echo "Configuration file ucentral.properties is missing in the current directory"
  exit 2
fi

docker run -d -p 15002:15002 \
              -p 16001:16001 \
              -p 16003:16003 \
              --init \
              --volume="$PWD:/ucentral-data" \
              -e UCENTRAL_ROOT="/ucentral-data" \
              -e UCENTRAL_CONFIG="/ucentral-data" \
              --name="ucentralgw" $DOCKER_NAME

```

Create yourself a directory and copy that script which you can also get from [here](https://github.com/stephb9959/ucentralgw/blob/main/docker_run.sh).
You must have the basic configuration file copied in the directory. This file must be called `ucentral.properties`. You can bring your own or
copy it from [here](https://github.com/stephb9959/ucentralgw/blob/main/ucentral.properties). You must create 
the certificates and copy them into your new `certs` directory. You need to make sure that the names match the content of the `ucentral.properties`
file. Once all this is done, you can simply run `docker_run.sh`.

### Configuration
The configuration for this service is kept in a properties file. Currently, this configuration file must be kept in the 
current directory of uCentral or one level up. This file is called `ucentral.properties` and you can see the latest version
[here](https://github.com/stephb9959/ucentralgw/blob/main/ucentral.properties). The file will be loaded from 
the directory set by the environment variable `UCENTRAL_CONFIG`. To use environment variables in the configuration,
you must use `$<varname>`. The path for the logs for the service must exist prior to starting the 
service. The path is defined under `logging.channels.c2.path`. Only `path names` support the use of 
environment variables. Here is a sample configuration:

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

## uCentral communication protocol
The communication protocol between the device and the controller is detailed in this [document](https://github.com/stephb9959/ucentralgw/blob/main/PROTOCOL.md).

## OpenAPI
The service supports an OpenAPI REST based interface for management. You can find the [definition here](https://github.com/stephb9959/ucentralgw/blob/main/tipapi/ucentral/ucentral.yaml).

## Using the API
In the `test_scripts` directory, you will find a series of scripts that will show you how to use the API with curl. More scripts will be added in the future.

## Certificates
Love'em of hate'em, we gotta use'em. So we tried to make this as easy as possible for you. Under the `cert_scripts` you 
can run a single command that will generate all the files you need. By default, this will generate the server side
of the certificates as well as certificates for 10 devices. You can change the variabla `howmany` in the script
to change that number. Once you run this script, you will get:

- `server-key.pem` : this file is the server key and should be used in the `ucentral.properties` file.
- `server-cert.pem` : this is the certificate to be used with the generated key. This should also be used in `ucentral.properties`.
- `dev-1-cert.pem` .. `dev-10-cert.pem` : certificates to be used on the actual devices in the `/etc/config` directories of the devices. These file must be renamed `cert.pem` on the devices.

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
 

