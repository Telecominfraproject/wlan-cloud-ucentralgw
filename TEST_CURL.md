# curl based test script
This script is built on top of `curl` and `jq`. You must set the global variable `UCENTRALGW` in order to use it.

```asm
cli help
```

## The commands

### getdevice <serial>                
Get the device JSON document.

### deletedevice <serial>             
Delete the device.

### createdevice <serial> <cfg> <MAC> 
Create a device using the default configuration. 
- `serial`: device serial number
- `cfg`: JSON config file name
- `MAC`: string MAC Address

### getdevicestatus <serial>
Get the device status JSON document.
- `serial`: device serial number

### getstats <serial>
Get statistics for the device.
- `serial`: device serial number

### gethealthchecks <serial>
Get healthchecks for a device.
- `serial`: device serial number

### newesthealthchecks <serial>
Get newest healthchecks for a device.
- `serial`: device serial number

### lasthealthcheck <serial>
Get the last healthcheck
- `serial`: device serial number

### getcapababilities <serial>
Get the device capabilities JSON document.
- `serial`: device serial number

### deletecapababilities <serial>
Delete the device capabilities JSON.
- `serial`: device serial number

### reboot <serial>                   
Reboot the device.
- `serial`: device serial number

### upgrade <serial> <URI> 
Do firmware upgrade for a device.
- `serial`: device serial number
- `URI`: complete URI where the upgrade file exists. No validation is performed.

### leds <serial> <pattern> <duration>       
Activate LEDs a device.
- `serial`: device serial number
- `pattern`: on/off/blink
- `duration`: number in seconds

### configure <serial> <cfg>          
Change configuration for a device.
- `serial`: device serial number
- `cfg`: JSON config file name

### factory <serial> <keep_redirector>     
Do factory reset for device. 
- `serial`: device serial number
- `keep_redirector`: true=keep redirector, false=reset redirector

### request <serial> <message>        
Force a message from the device.
- `serial`: device serial number
- `message`: state or healthcheck
                                 
### wifiscan <serial> <verbose>       
Do wifiscan for a device.
- `serial`: device serial number
- `verbose`: verbose=true/false

### trace <serial> <duration> <network>            
Launch a remote trace for a device.
- `serial`: device serial number
- `duration`: number in seconds
- `network`: which network to perform trace on: lan or wan  
                                  
### getcommand <command-uuid>
Get the command JSON document.
- `command-uuid`: command UUID

### deletecommand <command-uuid>      
Delete the command.
- `command-uuid`: command UUID

### newestcommands <serial>
Get the newest commands for a device.
- `serial`: device serial number

### listdevices
List devices.

### listcommands <serial>
List commands for a specific device.
- `serial`: device serial number

### deletecommands <serial>
Delete commands for a device.
- `serial`: device serial number

### getlogs <serial>                  
Get logs for the device.
- `serial`: device serial number

### newestlogs <serial>               
Get the latest logs for the device.
- `serial`: device serial number

### deletelogs <serial>               
Delete logs for the device.
- `serial`: device serial number

### eventqueue <serial>
Request event queue for the device.
- `serial`: device serial number

### listdefaultconfigs
List default configurations.

### createdefaultconfig <name> <ids> <cfg> 
Create a default configuration
- `name`: unique name, no spaces
- `ids`: comma separated list of models
- `cfg`: JSON config file name

### addblacklistdevice <serial> <reason>   
Add a device to the black list
- `serial`: serial number of the device to add
- `reason`: reason for blacklisting

### getblacklist
List all blacklisted devices

### deleteblacklistdevice  <serial>   
Add a device to the black list.
- `serial`: device serial number

### devicecount                       
Get the number of devices in the DB.

### deviceserialnumbers               
Get only the serial numbers.

### selectdevices <serial_list>       
Get a list of devices based on a list.
- `serial_list`: serial numbers (must be comma separated).

### deviceswithstatus                 
Get devices with their status.

### setloglevel <sys> <level>         
Set the logging system level for individual subsystems.
- `sys`: ufileuploader/websocket/storage/restapi/commandmanager/auth/deviceregistry/all
- `level`: level:none/fatal/critical/error/warning/notice/information/debug/trace

### getfile <uuid>					  
Get the file associated with trace command <uuid>.
- `uuid`: UUID of file to retrieve

### "rtty <serial>					  
Get the details for an rtty session.
- `serial`: device serial number

### lifetimestats <serial>			  
Get the lifetime stats counters for a device
- `serial`: device serial number

### laststats <serial>			      
Get the last statistics for a device.
- `serial`: device serial number

### neweststats <serial>			  
Get the newest statistics for a device.
- `serial`: device serial number

## Notes
To pass additional flags to the CURL command, create an environment variable called FLAGS and git ve the values you
want. For example, for force all call to use IPv6, set FLAGS=\"-6\", for verbose mode and IPv6, set FLAGS=\"-6 -v\"

