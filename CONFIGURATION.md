# Here the configuration parameters for the Controller
## Websocket parameters
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

## REST API External parameters
openwifi.restapi.host.0.backlog = 100
openwifi.restapi.host.0.security = relaxed
openwifi.restapi.host.0.rootca = $OWGW_ROOT/certs/restapi-ca.pem
openwifi.restapi.host.0.address = *
openwifi.restapi.host.0.port = 16002
openwifi.restapi.host.0.cert = $OWGW_ROOT/certs/restapi-cert.pem
openwifi.restapi.host.0.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.restapi.host.0.key.password = mypassword

## REST API Intra microservice parameters
openwifi.internal.restapi.host.0.backlog = 100
openwifi.internal.restapi.host.0.security = relaxed
openwifi.internal.restapi.host.0.rootca = $OWGW_ROOT/certs/restapi-ca.pem
openwifi.internal.restapi.host.0.address = *
openwifi.internal.restapi.host.0.port = 17002
openwifi.internal.restapi.host.0.cert = $OWGW_ROOT/certs/restapi-cert.pem
openwifi.internal.restapi.host.0.key = $OWGW_ROOT/certs/restapi-key.pem
openwifi.internal.restapi.host.0.key.password = mypassword

## File uploader parameters
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
oui.download.uri = https://standards-oui.ieee.org/oui/oui.txt

## Datamodel Source
ucentral.datamodel.uri = https://raw.githubusercontent.com/Telecominfraproject/wlan-ucentral-schema/main/ucentral.schema.json

## Command Manager
command.timeout = 14400
command.retry = 120
command.janitor = 3600
command.queue = 30

## IP to Country Parameters
iptocountry.default = US
iptocountry.provider = ipinfo
#iptocountry.provider = ipdata
iptocountry.ipinfo.token =
iptocountry.ipdata.apikey =

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
