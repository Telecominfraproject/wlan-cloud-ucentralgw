# Controller Configuration Parameters

## Websocket parameters
This is the crucial section. I bet that 97.4% of all your problems will come from here, and it's boring. So put some good music on,
give the kids the iPad, get a cup of coffee, and pay attention. Every field will be explained.

```properties
ucentral.websocket.host.0.backlog = 500
ucentral.websocket.host.0.rootca = $OWGW_ROOT/certs/root.pem
ucentral.websocket.host.0.issuer = $OWGW_ROOT/certs/issuer.pem
ucentral.websocket.host.0.cert = $OWGW_ROOT/certs/websocket-cert.pem
ucentral.websocket.host.0.key = $OWGW_ROOT/certs/websocket-key.pem
ucentral.websocket.host.0.clientcas = $OWGW_ROOT/certs/clientcas.pem
ucentral.websocket.host.0.cas = $OWGW_ROOT/certs/cas
ucentral.websocket.host.0.address = *
ucentral.websocket.host.0.port = 15002
ucentral.websocket.host.0.security = strict
ucentral.websocket.host.0.key.password = mypassword
ucentral.websocket.maxreactors = 20
```

### This is the end point for the devices to connect with
This is the crucial section. I bet that 97.4% of all your problems will come from here, and it's boring. So put some good music on,
give the kids the iPad, get a cup of coffee, and pay attention. Every field will be explained.

### ucentral.websocket.host.0.backlog
This is the number of concurrent devices you are expecting to call all at once. Not the current number of devices. This is how many will connect in the same exact second.
Take the total number of devices you have and divide by 100. That's a good rule of thumb. Never go above 500.
### ucentral.websocket.host.0.rootca
This is the root file as supplied by Digicert. You can find it [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/root.pem)
### ucentral.websocket.host.0.issuer
This is the issuer file as supplied by Digicert. You can find it [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/issuer.pem)
### ucentral.websocket.host.0.cert
This is a `pem` file that you will receive from Digicert for the gateway itself. This is the certificate for the gateway.
### ucentral.websocket.host.0.key
This is a `pem` file that you will receive from Digicert for the gateway itself. The is the private key for the gateway.
### ucentral.websocket.host.0.clientcas
This is a `pem` file that contains both the issuer and the root CA certificates. You can find it You can find it [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/certificates/clientcas.pem)
### ucentral.websocket.host.0.cas
This is a directory where you will copy your own `cert.pem`, the `root.pem`, and the `issuer.pem` files.
### ucentral.websocket.host.0.address
Leve this a `*` in the case you want to bind to all interfaces on your gateway host or select the address of a single interface.
### ucentral.websocket.host.0.port
Leave to 15002 for now.
### ucentral.websocket.host.0.security
Leave this as strict for now for devices.
### ucentral.websocket.host.0.key.password
If you key file uses a password, please enter it here.
### ucentral.websocket.maxreactors
A single reactor can handle between 1000-2000 devices. Never leave this smaller than 5 or larger than 50.

## REST API External parameters
These are the parameters required for the configuration of the external facing REST API server
```properties
openwifi.restapi.host.0.backlog = 100
openwifi.restapi.host.0.security = relaxed
openwifi.restapi.host.0.rootca = $OWGW_ROOT/certs/restapi-ca.pem
openwifi.restapi.host.0.address = *
openwifi.restapi.host.0.port = 16002
openwifi.restapi.host.0.cert = $OWGW_ROOT/certs/restapi-cert.pem
openwifi.restapi.host.0.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.restapi.host.0.key.password = mypassword
```

### openwifi.restapi.host.0.backlog
This is the number of concurrent REST API calls that maybe be kept in the backlog for processing. That's a good rule of thumb. Never go above 500.
### openwifi.restapi.host.0.rootca
This is the root file of your own certificate CA in `pem` format.
### openwifi.restapi.host.0.cert
This is your own server certificate in `pem` format..
### openwifi.restapi.host.0.key
This is the private key associated with your own certificate in `pem` format.
### openwifi.restapi.host.0.address
Leve this a `*` in the case you want to bind to all interfaces on your gateway host or select the address of a single interface.
### openwifi.restapi.host.0.port
The port on which the REST API server is listening. By default, this is 16002.
### openwifi.restapi.host.0.security
Leave this as `relaxed` for now for devices.
### openwifi.restapi.host.0.key.password
If you key file uses a password, please enter it here.


## REST API Intra microservice parameters
The following parameters describe the configuration for the inter-microservice HTTP server. You may use the same certificate/key 
you are using for your extenral server or another certificate.
```properties
openwifi.internal.restapi.host.0.backlog = 100
openwifi.internal.restapi.host.0.security = relaxed
openwifi.internal.restapi.host.0.rootca = $OWGW_ROOT/certs/restapi-ca.pem
openwifi.internal.restapi.host.0.address = *
openwifi.internal.restapi.host.0.port = 17002
openwifi.internal.restapi.host.0.cert = $OWGW_ROOT/certs/restapi-cert.pem
openwifi.internal.restapi.host.0.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.internal.restapi.host.0.key.password = mypassword
```

### openwifi.internal.host.0.backlog
This is the number of concurrent REST API calls that maybe be kept in the backlog for processing. That's a good rule of thumb. Never go above 500.
### openwifi.internal.host.0.rootca
This is the root file of your own certificate CA in `pem` format.
### openwifi.internal.host.0.cert
This is your own server certificate in `pem` format..
### openwifi.internal.host.0.key
This is the private key associated with your own certificate in `pem` format.
### openwifi.internal.host.0.address
Leve this a `*` in the case you want to bind to all interfaces on your gateway host or select the address of a single interface.
### openwifi.internal.host.0.port
The port on which the REST API server is listening. By default, this is 17002.
### openwifi.internal.host.0.security
Leave this as `relaxed` for now for devices.
### openwifi.internal.host.0.key.password
If you key file uses a password, please enter it here.

## File uploader parameters
Certain commands may require the Access Point to upload a file into the Controller. For this reason, there is a special embedded HTTP 
server to receive these files.

```properties
openwifi.fileuploader.host.0.backlog = 100
openwifi.fileuploader.host.0.rootca = $OWGW_ROOT/certs/restapi-ca.pem
openwifi.fileuploader.host.0.security = relaxed
openwifi.fileuploader.host.0.address = *
openwifi.fileuploader.host.0.name = ucentral.dpaas.arilia.com
openwifi.fileuploader.host.0.port = 16003
openwifi.fileuploader.host.0.cert = $OWGW_ROOT/certs/restapi-cert.pem
openwifi.fileuploader.host.0.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.fileuploader.host.0.key.password = mypassword
openwifi.fileuploader.path = $OWGW_ROOT/uploads
openwifi.fileuploader.maxsize = 10000
openwifi.fileuploader.uri = https://ucentral.dpaas.arilia.com:16003
```

### openwifi.fileuploader.host.0.backlog
This is the number of concurrent REST API calls that maybe be kept in the backlog for processing. That's a good rule of thumb. Never go above 500.
### openwifi.fileuploader.host.0.rootca
This is the root file of your own certificate CA in `pem` format.
### openwifi.fileuploader.host.0.cert
This is your own server certificate in `pem` format..
### openwifi.fileuploader.host.0.key
This is the private key associated with your own certificate in `pem` format.
### openwifi.intfileuploaderernal.host.0.address
Leve this a `*` in the case you want to bind to all interfaces on your gateway host or select the address of a single interface.
### openwifi.fileuploader.host.0.port
The port on which the REST API server is listening. By default, this is 16003.
### openwifi.fileuploader.host.0.security
Leave this as `relaxed` for now for devices.
### openwifi.fileuploader.host.0.key.password
If you key file uses a password, please enter it here.
### openwifi.fileuploader.path
This is the location where the files will be stored temporarily before processing. This `path` must exist.
### openwifi.fileuploader.maxsize 
This is the maximum uploaded file size. The default maximum size if 10MB. This size is in KB.
### openwifi.fileuploader.uri
This is the URI that will be passed to the AP. You must make sure that the AP can resolve this URI.

## Microservice information
openwifi.service.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.service.key.password = mypassword
openwifi.system.data = $OWGW_ROOT/data
openwifi.system.debug = true
openwifi.system.uri.private = https://localhost:17002
openwifi.system.uri.public = https://ucentral.dpaas.arilia.com:16002
openwifi.system.uri.ui = https://ucentral-ui.arilia.com
openwifi.security.restapi.disable = false
openwifi.system.commandchannel = /tmp/app.ucentralgw
openwifi.autoprovisioning = true
openwifi.devicetypes.0 = AP:linksys_ea8300,edgecore_eap101,linksys_e8450-ubi
openwifi.devicetypes.1 = SWITCH:edgecore_ecs4100-12ph
openwifi.devicetypes.2 = IOT:esp32

## OUI Service
The controller has a built-in OUI resolver for MAC addresses. The GW will periodically load this file to obtain the latest. 
This is ths URI for this file.
```properties
oui.download.uri = https://standards-oui.ieee.org/oui/oui.txt
```

## Data-model Source
The gateway can make use of the latest uCentral data-model or use the built-in model. These 2 parameters allow you to 
choose which method you want. If you select the internal method, the URI is ignored. If for some reason you choose 
the on-line data-model from the URI and the URI is not reachable, the system will fall back on the internal model.
```properties
ucentral.datamodel.internal = true
ucentral.datamodel.uri = https://raw.githubusercontent.com/Telecominfraproject/wlan-ucentral-schema/main/ucentral.schema.json
```

## Command Manager
The command manager is responsible for managing command sent and responses received with the APs. Several parameters allow you
to fine tune its behaviour. Unless you have some particular reasons to change tem the defaults are usually just fine.
```properties
command.timeout = 14400
command.retry = 120
command.janitor = 120
command.queue = 30
```
### command.timeout
How long will the GW wait in seconds before considering a commands has timed out. 

### command.retry
How long between command retries.

### command.janitor
How long between outstanding RPC clean-ups.

### command.queue
How long should te gateway wait between running its queue.

## IP to Country Parameters
The controller has the ability to find the location of the IP of each Access Points. This uses an external IP location service. Currently,
the controller supports 3 services. Please not that these services will require to obtain an API key or token, and these may cause you to incur 
additional fees. Here is the list of the services supported:
- ip2location: ip2location.com
- ipdata: ipdata.co
- ipinfo: ipinfo.io


```properties
iptocountry.default = US
iptocountry.provider = ipinfo
#iptocountry.provider = ipdata
#iptocountry.provider = ip2location
iptocountry.ipinfo.token =
iptocountry.ipdata.apikey =
iptocountry.ip2location.apikey =
```

### iptocountry.default
This is the country code to be used if no information can be found at one of the providers or you have not configured any of the providers.

### iprocountry.provider
You must select onf of the possible services and the fill the appropriate token or api key parameter.

## Provisioning link
autoprovisioning.process = prov,default

## Restricted Device Signature Manager 
signature.manager.0.key.public = $OWGW_ROOT/certs/signatures/test1-public-key.pem
signature.manager.0.key.private = $OWGW_ROOT/certs/signatures/test1-private-key.pem
signature.manager.0.vendor = test1
signature.manager.1.key.public = $OWGW_ROOT/certs/signatures/test2-public-key.pem
signature.manager.1.key.private = $OWGW_ROOT/certs/signatures/test2-private-key.pem
signature.manager.1.vendor = test2

## OWLS Simulator ID
simulatorid = 53494d

## RTTY parameters
rtty.internal = true
rtty.enabled = true
rtty.server = rtty-tip.arilia.com
rtty.port = 5912
rtty.token = 96181c567b4d0d98c50f127230068fa8
rtty.timeout = 60
rtty.viewport = 5913
rtty.assets = $OWGW_ROOT/rtty_ui

## RADIUS proxy config
radius.proxy.enable = false
radius.proxy.accounting.port = 1813
radius.proxy.authentication.port = 1812
radius.proxy.coa.port = 3799

## NLB Support
alb.enable = true
alb.port = 16102

## Kafka
openwifi.kafka.group.id = gateway
openwifi.kafka.client.id = gateway1
openwifi.kafka.enable = true
openwifi.kafka.brokerlist = a1.arilia.com:9092
openwifi.kafka.auto.commit = false
openwifi.kafka.queue.buffering.max.ms = 50
penwifi.kafka.ssl.ca.location =
openwifi.kafka.ssl.certificate.location =
openwifi.kafka.ssl.key.location =
openwifi.kafka.ssl.key.password =

## DB Type
storage.type = sqlite
#storage.type = postgresql
#storage.type = mysql
#storage.type = odbc

## Storage SQLite
storage.type.sqlite.db = devices.db
storage.type.sqlite.idletime = 120
storage.type.sqlite.maxsessions = 128

## Storage Postgres
storage.type.postgresql.maxsessions = 64
storage.type.postgresql.idletime = 60
storage.type.postgresql.host = localhost
storage.type.postgresql.username = stephb
storage.type.postgresql.password = snoopy99
storage.type.postgresql.database = ucentral
storage.type.postgresql.port = 5432
storage.type.postgresql.connectiontimeout = 60

## Storage MySQL/MariaDB
storage.type.mysql.maxsessions = 64
storage.type.mysql.idletime = 60
storage.type.mysql.host = localhost
storage.type.mysql.username = stephb
storage.type.mysql.password = snoopy99
storage.type.mysql.database = ucentral
storage.type.mysql.port = 3306
storage.type.mysql.connectiontimeout = 60

## Auto Archiver Parameters
archiver.enabled = true
archiver.schedule = 03:00
archiver.db.0.name = healthchecks
archiver.db.0.keep = 7
archiver.db.1.name = statistics
archiver.db.1.keep = 7
archiver.db.2.name = devicelogs
archiver.db.2.keep = 7
archiver.db.3.name = commandlist
archiver.db.3.keep = 7

## Logging Parameters
logging.type = file
logging.path = $OWGW_ROOT/logs
logging.level = information
logging.asynch = true
logging.websocket = false
