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
- cpprest
- a C++20 compiler

### Ubuntu build
```
sudo apt install libboost-dev cpprestsdk-dev cmake gcc
git clone <this repo name>
cd ucentralgw
mmkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
```

