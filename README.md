<p align="center">
    <img src="images/project/logo.svg" width="200" alt="OpenWiFi Project"/>
</p>

# OpenWiFI Gateway (OWGW)

## What is it?
The OpenWiFi Gateway is a service for the TIP OpenWiFi CloudSDK (OWSDK). 
OWGW manages Access Points that implement the OpenWiFi uCentral protocol. OWGW, like all other OWSDK microservices, is
defined using an OpenAPI definition and uses the ucentral communication protocol to interact with Access Points. To use 
the OWGW, you either need to [build it](#building) or use the [Docker version](#docker).

## Building
To build the microservice from source, please follow the instructions in [here](./BUILDING.md)

## Docker
To use the CLoudSDK deployment please follow [here](https://github.com/Telecominfraproject/wlan-cloud-ucentral-deploy)

#### Expected directory layout
From the directory where your cloned source is, you will need to create the `certs`, `logs`, and `uploads` directories.
```bash
mkdir certs
mkdir certs/cas
mkdir logs
mkdir uploads
```
You should now have the following:
```text
--+-- certs
  |   +--- cas
  +-- cmake
  +-- cmake-build
  +-- logs
  +-- src
  +-- test_scripts
  +-- openapi
  +-- uploads
  +-- owgw.properties
```

### Certificates
The OWGW uses a number of certificates to provide security. There are 2 types of certificates required for 
a normal deployment:
- A Server Certificate to secure the OWGW<->AP channel
- A REST API Certificate to secure the Northbound API
- Device Certificates

#### The `certs` directory
For all deployments, you will need the following certs directory, populated with the proper files.

```text
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
These are the files you should install on your OWGW and devices. For your OWGW, you will need to provide tge following files in the directory above
- `root.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/root.pem).
- `issuer.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/issuer.pem).
- `clientcas.pem` is [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/clientcas.pem).

#### OWGW Server certificate (must be obtained from TIP)
The gateway requires its own DigiCert certificate. Once obtained, you need to identify the `key` and the `certificate` rename
them `websocket-key.pem` and `websocket-cert.pem`, and copy them in your `certs` directory. These files mus be obtained from TIP.

#### OWGW for certificate: for REST API (from your favourite Certificate Provider)
The gateway requires a key/certificate/ca for the REST interface. These files you need to obtain on your own or generate them. This is beyond the scope of this 
document. You, may choose to select LestEncrypt or any other Certificate Authority. Once you have these files, you need to renamed them `restapi-key.pem`, `restapi-cert.pem`, and `restapi-ca.pem`. 
This will guarantee proper HTTPS in your browser and RESTAPI. 

### OpenWiFi Device certificates
This may have already done at the factory. If not, you will need to get the following in order to point your devices to use the OWGW:
- A DigiCert certificate that you will call `cert.pem`
- A DigiCert key that goes with that certificate. Please call this `key.pem`
- The Digicert root certificate that you will find [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/root.pem). You must copy `root.pem`
  and rename it `cas.pem` on the device.
- A Device ID file called `dev-id` or something similar
- Copy all the 4 files to the `/certificates` directory of the AP (you must have firmware created Jun 15th or later).

You will need to upgrade your device to the latest firmware. Once updated, you will need to copy the 4 files mentioned above in
the `/certificates` directory. Please remove all old keys or certificates from the `/etc/ucentral` directory
(anything ending in `.pem`).

### Environment variables
The following environment variables should be set from the root directory of the service. They tell the OWGW process where to find 
the configuration and the root directory.
```bash
export OWGW_ROOT=`pwd`
export OWGW_CONFIG=`pwd`
```
You can run the shell script `set_env.sh` from the microservice root.

### OWGW Service Configuration
The configuration is kept in a file called `owgw.properties`. To understand the content of this file, 
please look [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONFIGURATION.md)

### Running the OWGW
Tu run the OWGW, you must run the executable `owgw`. You can use several command line options to run as a daemon or 
specify the configuration file location.

### systemd: owgw.service
`owgw.service` is a skeleton to allow to run the OWGW in a systemd based operating system (i.e. Debian). You will need 
to modify slightly to reflect your environment.

### Default device configuration
By default, the devices will receive a built-in default configuration. This built-in default configuration is probably not what you need. 
So there are 2 options in order to specify your own default configuration:
- In the OWGW data directory, you can create a `default_config.json` file that contains your own personal configuration
- Using the OWGW UI, on the left hand pane, there is a `configurations` choice. There you can do the same but apply it with more granularity ot each device type 
you may be deploying.

### Command line options
The current implementation supports the following. If you use the built-in configuration file, you do not need to use any command-line
options. However, you may decide to use the `--daemon` or `umask` options. 

```bash
./owgw --help
usage: owgw OPTIONS
A owgw gateway implementation for TIP.

--daemon        Run application as a daemon.
--umask=mask    Set the daemon's umask (octal, e.g. 027).
--pidfile=path  Write the process ID of the application to given file.
--help          display help information on command line arguments
--file=file     specify the configuration file
--debug         to run in debug, set to true
--logs=dir      specify the log directory and file (i.e. dir/file.log)
```

#### file
This allows you to point to another file without specifying the OWGW_CONFIG variable. The file name must end in `.properties`.
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
The communication protocol between the device and the OGWG is detailed in this [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/PROTOCOL.md).

## OpenAPI
The OWGW supports an OpenAPI REST based interface for management. You can find the [definition here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/openapi/ucentral/owgw.yaml).
And here is [how to use it](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/OPENAPI.md)

## Using the API
In the `test_scripts` directory, you will find a series of scripts that will show you how to use the API 
with [curl](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/CLI.md) 
or [python](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/TEST_PYTHON.md). 
More scripts will be added in the future.

## Firewall Considerations
| Port | Description | Configurable |
| :--- | :--- |:------------:|
| 15002 | Default port from the devices to the OWGW |     yes      |
| 16002 | Default port for REST API Access to the OWGW |     yes      |
| 5912 | Default port for RTTY connection |     yes      |
| 5913 | Defailt port for RTTY connection |     yes      |

## Kafka topics
Toe read more about Kafka, follow the [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/KAFKA.md)

## Contributions
We need more contributors. Should you wish to contribute, 
please follow the [contributions](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONTRIBUTING.md) document.

## Pull Requests
Please create a branch with the Jira addressing the issue you are fixing or the feature you are implementing. 
Create a pull-request from the branch into master. 

## Additional OWSDK Microservices
Here is a list of additional OWSDK microservices
| Name | Description | Link | OpenAPI |
| :--- | :--- | :---: | :---: |
| OWSEC | Security Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec/blob/main/openpapi/owsec.yaml) |
| OWGW | Controller Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/openapi/owgw.yaml) |
| OWFMS | Firmware Management Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms/blob/main/openapi/owfms.yaml) |
| OWPROV | Provisioning Service | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov) | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/openapi/owprov.yaml) |
| OWANALYTICS | Analytics Service | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics) | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics/blob/main/openapi/owanalytics.yaml) |
| OWSUB | Subscriber Service | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal) | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal/blob/main/openapi/userportal.yaml) |
