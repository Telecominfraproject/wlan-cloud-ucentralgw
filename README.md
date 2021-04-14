# uCentralGW

## What is this?
The uCentralGW is an added service for the TIP controller that allows integration with the 
uCentral protocol. It supports a complete OpenAPI definition and uses the ucentral communication protocol. To use the uCentralGW,
you either need to [build it](#building) or use the [Docker version](#docker).

## Building
In order to build the uCentralGW, you will need to install its dependencies, which includes the following:
- cmake
- boost
- POCO 1.10.1 or later
- a C++17 compiler
- libyaml
- openssl
- libpq-dev (PortgreSQL development libraries)
- mysql-client (MySQL client)

The build is done in 2 parts. The first part is to build a local copy of the framework tailored to your environment. This 
framework is called [Poco](https://github.com/pocoproject/poco). The version used in this project has a couple of fixes
from the master copy needed for cmake. Please use the version of this [Poco fix](https://github.com/stephb9959/poco). Building 
Poco may take several minutes depending on the platform you are building on.

### Ubuntu
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
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make
```

### Fedora
The following instructions have proven to work on Fedora 33
```
sudo yum install cmake g++ openssl-devel unixODBC-devel mysql-devel mysql apr-util-devel boost boost-devel
sudo yum install yaml-cpp-devel lua-devel
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
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
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
git clone https://github.com/Telecominfraproject/wlan-cloud-ucentralgw
cd wlan-cloud-ucentralgw
mkdir cmake-build
cd cmake-build
cmake ..
make -j
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

### After the build step is completed
Once your build is done. You can remove the Poco source as it is no longer needed. 

#### Expected directory layout
From the directory where your cloned source is, you will need to create the `certs`, `logs`, and `uploads` directories.
```shell
mkdir certs
mkdir logs
mkdir uploads
```

You should now have the following:

```
-- cert_scripts
  |
  +-- certs
  |
  +-- cmake
  |
  +-- cmake-build
  |
  +-- logs (dir)
  |
  +-- src
  |
  +-- test_scripts
  |
  +-- openapi
  |
  +-- uploads
  |
  +-- ucentral.properties
```

#### Certificates for your gateway
If you have not been provided with certificates, you need to [generate your own certificates](#certificates) using the procedure 
in this document. When done, copy the `server-cert.pem` and `server-key.pem` files in the `certs` directory. If you generate your own,
you must remember to copy the generated devices certificates on the devices. You should now have:

```
-- cert_scripts
  |
  +-- certs
  |     +---- server-key.pem
  |     +---- server-cert.pem
  |
  +-- cmake
  |
  +-- cmake-build
  |
  +-- logs (dir)
  |
  +-- src
  |
  +-- test_scripts
  |
  +-- openapi
  |
  +-- uploads
  |
  +-- ucentral.properties
```

#### Configuration
The configuration for this service is kept in a properties file. This file is called `ucentral.properties` and you can 
see the latest version [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/ucentral.properties). The file will be loaded from
the directory set by the environment variable `UCENTRAL_CONFIG`. To use environment variables in the configuration,
you must use `$<varname>`. Only `path names` support the use of environment variables. The sample configuration requires very 
little changes if you keep the suggested directory structure. For the sample configuration to work, you need to define 2 
environment variables. 
```
export UCENTRAL_ROOT=`pwd`
export UCENTRAL_CONFIG=`pwd`
```
If you current working directory is the root of the project, this will set the variables properly. Otherwise, you can set the variables 
to point to wherever is necessary.

##### Important config entries
###### This is the logging directory
```
logging.channels.c2.path = $UCENTRAL_ROOT/logs/sample.log
```

###### This is the type of storage in use
```asm
storage.type = sqlite
```

###### Autoprovisioning settings
```asm
ucentral.autoprovisioning = true
ucentral.autoprovisioning.type.0 = AP:ea8300,edge
ucentral.autoprovisioning.type.1 = IOT:ea8301,edge2
ucentral.autoprovisioning.type.2 = AP:ea8302,edge6
```

###### This is the RESTAPI endpoint
```asm
ucentral.restapi.host.0.address = *
ucentral.restapi.host.0.port = 16001
ucentral.restapi.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.restapi.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
```

###### This is the end point for the devices to connect with
```asm
ucentral.websocket.host.0.address = *
ucentral.websocket.host.0.port = 15002
ucentral.websocket.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.websocket.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
```

###### This is the end point for the devices when uploading files
```asm
ucentral.fileuploader.host.0.address = *
ucentral.fileuploader.host.0.name = 192.168.1.176       <<<<<<< Replace this IP with the IP of your gateway or its FQDN
ucentral.fileuploader.host.0.port = 16003
ucentral.fileuploader.host.0.cert = $UCENTRAL_ROOT/certs/server-cert.pem
ucentral.fileuploader.host.0.key = $UCENTRAL_ROOT/certs/server-key.pem
ucentral.fileuploader.host.0.key.password = mypassword
ucentral.fileuploader.path = $UCENTRAL_ROOT/uploads
```

###### host.0.address entries
If you want to limit traffic to a specific interface, you should specify the IP address of that interface instead of 
the `*`. Using the `*` means all interfaces will be able to accept connections. You can add multiple interfaces 
by changing the `0` to another index. You need to repeat the whole configuration block for each index. Indexes must be sequential
start at `0`.

###### ucentral.fileuploader.host.0.name
This must point to the IP or FQDN of your uCentralGW.

#### Running the gateway
Tu run the gateway, you must run the executable `ucentralgw`. You can use several command line options to run as a daemon or specify the configuration file location. 

#### Command line options
The current implementation supports the following. If you use the built-in configuration file, you do not need to use any command-line
options. However, you may decide to use the `--daemon` or `umask` options. 

```
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

##### file
This allows you to point to another file without specifying the UCENTRAL_CONFIG variable. The file name must end in `.properties`.
##### daemon
Run this as a UNIX service
##### pidfile
When running as a daemon, the pid of the running service will be set in the speficied file
##### debug
Run the service in debug mode.
##### logs
Speficy where logs should be kept. You must include an existing directory and a file name. For example `/var/ucentral/logs/log.0`.
##### umask
Seet the umask for the running service.


### Docker
So building this thing from scratch is not your thing? I can't blame you. It takes some patience and 
in the end, there's still more work. Here comes `docker` to the rescue. You can run a docker version following
these instructions. The following is the content of the `docker_run.sh` script you can find
[here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/docker_run.sh):

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

Create yourself a directory and copy that script which you can also get from [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/docker_run.sh).
You must have the basic configuration file copied in the directory. This file must be called `ucentral.properties`. You can bring your own or
copy it from [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/ucentral.properties). Please look at [this](#certificates-with-docker) to have the right 
certificates. You need to make sure that the names match the content of the `ucentral.properties`
file. Once all this is done, you can simply run `docker_run.sh`.

#### Docker installation directory layout
Here is the layout expected for your Docker installation

```
Run-time root
    |
    |
    ----- certs (dir)
    |        |
    |        +-- server-key.pem
    |        --- server-cert.pem
    +---- logs  (dir)
    +---- uploads  (dir)
    +---- ucentral.properties (file)
```

#### `ucentral.properties` for Docker
If you use the pre-made configuration file, and you follow the directory layout, the only line you must change 
is the following line:

```asm
ucentral.fileuploader.host.0.name = 192.168.1.176
```

This line should reflect the IP of your gateway or its FQDN. You must make sure that this name or IP is accessible
from your devices. This is used during file uploads from the devices.

#### Certificates with Docker
If you have not been provided with certificates, you should generate some using the procedure in this [section](#certificates). When done, 
copy the `server-cert.pem` and `server-key.pem` files in the `certs` directory. If you generate your own certificates,
you must remember to copy the generated devices certificates on the devices.

#### Configuration with Docker
The configuration for this service is kept in a properties file. Currently, this configuration file must be kept in the 
current directory of uCentral or one level up. This file is called `ucentral.properties` and you can see the latest version
[here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/ucentral.properties). The file will be loaded from 
the directory set by the environment variable `UCENTRAL_CONFIG`. To use environment variables in the configuration,
you must use `$<varname>`. The path for the logs for the service must exist prior to starting the 
service. The path is defined under `logging.channels.c2.path`. Only `path names` support the use of 
environment variables. Here is a sample configuration:

### Certificates
Love'em of hate'em, we gotta use'em. So we tried to make this as easy as possible for you. Under the `cert_scripts` you 
can run a single command (`create_sertificates.sh`) that will generate all the files you need. By default, this will generate the server side
of the certificates as well as certificates for 10 devices. You can change the variable `howmany` in the script
to change that number. 

```shell
cd cert_scripts
./create_certificates.sh
ls
clean.sh                dev-1-cert.pem          dev-2-cert.pem          dev-4-cert.pem          dev-6-cert.pem          dev-8-cert.pem          more_devices.sh         server-key.pem
create_certificates.sh  dev-10-cert.pem         dev-3-cert.pem          dev-5-cert.pem          dev-7-cert.pem          dev-9-cert.pem          server-cert.pem         server.csr
```

You have now created: 
- `server-key.pem`: this file is the server key and should be used in the `ucentral.properties` file.
- `server-cert.pem`: this is the certificate to be used with the generated key. This should also be used in `ucentral.properties`.
- `dev-1-cert.pem` .. `dev-10-cert.pem` : certificates to be used on the actual devices in the `/etc/config` directories of the devices. These files 
  must be renamed `cert.pem` on the devices.

The script `more_devices` can be used to generate more devices without regenerating the original key. Just change the `finish` variable to the number you need. The script `clean.sh` 
let's you start from scratch again.

## uCentral communication protocol
The communication protocol between the device and the controller is detailed in this [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/PROTOCOL.md).

## OpenAPI
The service supports an OpenAPI REST based interface for management. You can find the [definition here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/openapi/ucentral/ucentral.yaml).
And here is [how to use it](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/OPENAPI.md)

## Using the API
In the `test_scripts` directory, you will find a series of scripts that will show you how to use the API with curl or Python3. 
More scripts will be added in the future.

 
## Firewall:
You must open port 15002 so that AP can communicate with ucentralgw
You must open port 16001 so that JSON cfg API can communicate with ucentralgw
You must open port 16003 so that AP can upload to ucentralgw

