# curl based test script
This script is built on top of `curl` and `jq`. You must set the global variable `UCENTRALGW` in order to use it.

```asm
cli help
```

## The commands

### getdevice <serial_number>

### listdevices

### deletedevice <serial_number>

### createdevice <serial_number> <config> <mac>

### selectdevices <serial_number_list>

### deviceserialnumbers

### devicecount

### deviceswithstatus

### help

### getcommand <command_uuid>

### deletecommand <command_uuid>

### listcommands <serial_number>

### deletecommands <serial_number>

### getcapabilities <serial_number>

### deletecapabilities <serial_number>

### reboot <serial_number>

### upgrade <serial_number> <uri>

### factory <serial_number> <keep_redirector>

### leds <pattern> <duration>

### configure <serial_number> <config_file>

### request <serial_number> <message_type>

### wifiscan <serial_number> <verbose>

### trace <serial_number> <duration> <interface>

### setloglevel <subsystem> <level>

### getdevicestatus <serial_number>

### getstats <serial_number>

### getlogs <serial_number>

### deletelogs <serial_number>

### gethealthchecks <serial_number>

### createdefaultconfig <name> <models>> <config_file>

### listdefaultconfigs

### addblacklistdevice <serial_number> <reason>

### deleteblacklistdevice <serial_number>

### getblacklist

### eventqueue <serial_number> <types>


