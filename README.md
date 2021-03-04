# uCentralGW

## What is this?
The uCentralGW is an added service for the TIP controller that allows integration with the 
uCentral protocol. It is meant to run as a pod inside a TIP controller container. It should 
have a single port open to provide connection from uCentral enabled devices.

## Building
In order to build the uCentralGW, you will need to install some of its dependencies, which includes 
the following

- cmake
- boost
- POCO 1.10.1 or later
- a C++20 compiler
- libyaml

### Ubuntu build
```
sudo apt install libboost-dev cmake libpoco-dev libssl-dev openssl libyaml-dev gcc
git clone <this repo name>
cd ucentralgw
mmkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
```

