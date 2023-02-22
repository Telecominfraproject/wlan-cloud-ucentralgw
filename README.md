<p align=center><img src=images/project/logo.svg?sanitize=true/ width="200px" height="200px"></p>

# OpenWiFI Gateway

## What is it?
The OpenWiFi Gateway  (a.k.a. the controller or uCentral Gateway) is a service for the TIP OpenWiFi CloudSDK. 
The Gateway manages Access Points that implement the OpenWiFi uCentral protocol. 
The management is done using OpenAPI definition and uses the ucentral communication protocol. To use the gateway,
you either need to [build it](#building) or use the [Docker version](#docker).

## Building
In order to build the uCentralGW, you will need to install its dependencies, which includes the following:
- cmake
- boost
- POCO 1.10.1 or later
- a C++17 compiler
- openssl
- libpq-dev (PortgreSQL development libraries)
- mysql-client (MySQL client)
- librdkafka
- cppkafka
- 

The build is done in 2 parts. The first part is to build a local copy of the framework tailored to your environment. This 
framework is called [Poco](https://github.com/pocoproject/poco). The version used in this project has a couple of fixes
from the master copy needed for cmake. Please use the version of this [Poco fix](https://github.com/AriliaWireless/poco). Building 
Poco may take several minutes depending on the platform you are building on.

### Ubuntu
These instructions have proven to work on Ubuntu 20.4.
```
sudo apt install git cmake g++ libssl-dev libmariadb-dev 
sudo apt install libpq-dev libaprutil1-dev apache2-dev libboost-all-dev
sudo apt install librdkafka-dev // default-libmysqlclient-dev
sudo apt install nlohmann-json-dev

cd ~
git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
cd cppkafka
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

git clone https://github.com/fmtlib/fmt --branch 9.0.0 /fmtlib
cd fmtlib
mkdir cmake-build
cd cmake-build
cmake ..
make
make install

cd ~
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make -j 8


```

### Fedora
The following instructions have proven to work on Fedora 33
```
sudo yum install cmake g++ openssl-devel mysql-devel mysql apr-util-devel boost boost-devel
sudo yum install yaml-cpp-devel lua-devel 
sudo dnf install postgresql.x86_64 librdkafka-devel
sudo dnf install postgresql-devel json-devel

git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
cd cppkafka
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make

```

### macOS Build
The following instructions have proven to work on macOS Big Sur. You need to install [Homebrew](https://brew.sh/). You must also have installed [XCode for OS X](https://www.freecodecamp.org/news/how-to-download-and-install-xcode/).
```
brew install openssl \
	cmake \
	libpq \
	mysql-client \
	apr \
	apr-util \
	boost \
	yaml-cpp \
	postgresql \
	librdkafka \
	nlohmann-json \
	fmt

git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
pushd poco
mkdir cmake-build
push cmake-build
cmake -DOPENSSL_ROOT_DIR=</path/to/openssl> -DENABLE_NETSSL=1 -DENABLE_JWT=1 -DENABLE_CRYPTO=1 ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
pushd cppkafka
mkdir cmake-build
pushd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
pushd wlan-cloud-ucentralgw
mkdir cmake-build
pushd cmake-build
cmake ..
make -j
popd
popd
```

### Raspberry
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
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
mkdir cmake-build
cd cmake-build
cmake -DSMALL_BUILD=1 ..
make
```

### After completing the build
After completing the build, you can remove the Poco source as it is no longer needed. 

#### Expected directory layout
From the directory where your cloned source is, you will need to create the `certs`, `logs`, and `uploads` directories.
```shell
mkdir certs
mkdir certs/cas
mkdir logs
mkdir uploads
```

You should now have the following:

```
-- cert_scripts
  |
  +-- certs
  |   +--- cas
  +-- cmake
  +-- cmake-build
  +-- logs (dir)
  +-- src
  +-- test_scripts
  +-- openapi
  +-- uploads
  +-- owgw.properties
```

### Certificates
Love'em or hate'em, we need to use'em. So we tried to make this as easy as possible for you. 

#### The `certs` directory
For all deployments, you will need the following certs directory, populated with the proper files.

```asm
certs ---+--- root.pem
         +--- issuer.pem
         +--- websocket-cert.pem
         +--- websocket-key.pem
         +--- clientcas.pem
         +--- +cas
         |    +--- issuer.pem
         |    +--- root.pem
         +--- restapi-ca.pem
         +--- restapi-cert.pem
         +--- restapi-key.pem
```

#### DigiCert files
These are the files you should install on your gateway and devices. For your gateway, you will need to provide tge following files in the directory above
- `root.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/root.pem).
- `issuer.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/issuer.pem).
- `clientcas.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/clientcas.pem).

#### Gateway certificates: TIP portion (must be obtained from TIP)
The gateway requires its own DigiCert certificate. Once obtained, you need to identify the `key` and the `certificate` rename
them `websocket-key.pem` and `websocket-cert.pem`, and copy them in your `certs` directory. These files mus be obtained from TIP.

#### Gateway certificates: for REST API
The gateway requires a key/certificate/ca for the REST interface. These files you need to obtain on your own or generate them. This is beyond the scope of this 
document. You, may choose to select LestEncrypt or any other Certificate Authority. Once you have these files, you need to renamed them `restapi-key.pem`, `restapi-cert.pem`, and `restapi-ca.pem`. 
This will guarantee proper HTTPS in your browser and RESTAPI. 

### Environment variables
The following environment variables should be set from the root directory of the service.
```bash
export OWGW_ROOT=`pwd`
export OWGW_CONFIG=`pwd`
```
### Configuration
The configuration is kept in a file called `owgw.properties`. To understand the content of this file, 
please look [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/CONFIGURATION.md)

### Running the gateway
Tu run the gateway, you must run the executable `ucentralgw`. You can use several command line options to run as a daemon or specify the configuration file location. 

### Device configuration
Once you have the gateway configured, you will need to have some devices coming to it. For now, you will need to get
the following in order to use the gateway:
- A DigiCert certificate that you will call `cert.pem`
- A DigiCert key that goes with that certificate. Please call this `key.pem`
- The Digicert root certificate that you will find [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/root.pem). You must copy `root.pem`
and rename it `cas.pem` on the device.
- A Device ID file called `dev-id` or something similar
- Copy all the 4 files to the `/certificates` directory of the AP (you must have firmware created Jun 15th or later).

You will need to upgrade your device to the latest firmware. Once updated, you will need to copy the 4 files mentioned above in 
the `/certificates` directory. Please remove all old keys or certificates from the `/etc/ucentral` directory 
(anything ending in `.pem`).

### Command line options
The current implementation supports the following. If you use the built-in configuration file, you do not need to use any command-line
options. However, you may decide to use the `--daemon` or `umask` options. 

```bash
./ucentralgw --help
usage: ucentralgw OPTIONS
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
This allows you to point to another file without specifying the UCENTRALGW_CONFIG variable. The file name must end in `.properties`.
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

## Docker
If you would rather launch the docker-compose or helm for the controller, please click [here](https://github.com/Telecominfraproject/wlan-cloud-ucentral-deploy).

## uCentral communication protocol
The communication protocol between the device and the controller is detailed in this [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/PROTOCOL.md).

## OpenAPI
The service supports an OpenAPI REST based interface for management. You can find the [definition here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/openapi/ucentral/owgw.yaml).
And here is [how to use it](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/OPENAPI.md)

## Using the API
In the `test_scripts` directory, you will find a series of scripts that will show you how to use the API 
with [curl](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/CLI.md) 
or [python](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/TEST_PYTHON.md). 
More scripts will be added in the future.

## Firewall Considerations
- The protocol uses TCP port 15002 between the devices and the gateway. This port must be opened.
- Devices use the TCP port 16003 to upload files. This port is configurable in the `owgw.properties` file. Look for `openwifi.fileuploader.host.0.port`.
- The RESTAPI is accessed through TCP port 16002 by default. This port is configurable in the `owgw.properties` file. Look for the entry `openwifi.restapi.host.0.port`.

## Kafka topics
Toe read more about Kafka, follow the [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/KAFKA.md)

## Contributions
We need more contributors. Should you wish to contribute, 
please follow the [contributions](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONTRIBUTING.md) document.

## Pull Requests
Please create a branch with the Jira addressing the issue you are fixing or the feature you are implementing. 
Create a pull-request from the branch into master. 
