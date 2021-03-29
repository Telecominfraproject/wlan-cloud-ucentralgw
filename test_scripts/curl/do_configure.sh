#!/bin/zsh

if [ $# -eq 0 ]; then
  if [ "$test_device" = "" ]; then
    echo "You must supply the serial number of device to test or set <test_device> to the serial number you want to test."
    exit 1
  else
    serial=$test_device
  fi
else
  serial=$1
fi

config='{ "log" : { "_log_hostname" : "foo", "_log_ip" : "192.168.11.23", "_log_port" : 12345, "_log_proto" : "udp", "_log_size" : 128 }, "network" : [ { "cfg" : { "proto" : "dhcp" }, "mode" : "wan" }, { "cfg" : { "peeraddr" : "50.210.104.108", "vid" : "50" }, "mode" : "gre" }, { "cfg" : { "dhcp" : { "leasetime" : "6h", "limit" : 100, "start" : 10 }, "ip6assign" : 60, "ipaddr" : "192.168.16.1", "leases" : [ { "hostname" : "test", "ip" : "192.168.100.2", "mac" : "00:11:22:33:44:55" }, { "hostname" : "test2", "ip" : "192.168.100.3", "mac" : "00:11:22:33:44:56" } ], "mtu" : 1500, "netmask" : "255.255.255.0", "proto" : "static" }, "mode" : "nat", "vlan" : 200 }, { "cfg" : { "dhcp" : { "leasetime" : "6h", "limit" : 100, "start" : 10 }, "ipaddr" : "192.168.12.11", "proto" : "static" }, "mode" : "guest" } ], "ntp" : { "enable_server" : 1, "enabled" : 1, "server" : [ "0.openwrt.pool.ntp.org", "1.openwrt.pool.ntp.org" ] }, "phy" : [ { "band" : "2", "cfg" : { "beacon_int" : 100, "chanbw" : 20, "channel" : 6, "country" : "DE", "disabled" : 0, "htmode" : "HE40", "hwmode" : "11g", "txpower" : 30 } }, { "band" : "5", "cfg" : { "channel" : 0, "country" : "DE", "disabled" : 0, "htmode" : "HE80", "mimo" : "4x4" } }, { "band" : "5u", "cfg" : { "channel" : 100, "country" : "DE", "disabled" : 0, "htmode" : "VHT80" } }, { "band" : "5l", "cfg" : { "channel" : 36, "country" : "DE", "disabled" : 0, "htmode" : "VHT80" } } ], "rtty" : { "host" : "websocket.usync.org", "interface" : "wan", "token" : "7049cb6b7949ba06c6b356d76f0f6275" }, "ssh" : { "Port" : 22, "enable" : 1 }, "ssid" : [ { "band" : [ "2" ], "cfg" : { "encryption" : "psk2", "ft_over_ds" : 1, "ft_psk_generate_local" : 1, "ieee80211k" : 1, "ieee80211r" : 1, "ieee80211v" : 1, "isolate" : 1, "key" : "OpenWifi", "mobility_domain" : "4f57", "mode" : "ap", "network" : "guest", "ssid" : "uCentral-Guest" } }, { "band" : [ "5l", "5" ], "cfg" : { "encryption" : "psk2", "ft_over_ds" : 1, "ft_psk_generate_local" : 1, "ieee80211k" : 1, "ieee80211r" : 1, "ieee80211v" : 1, "key" : "OpenWifi", "mobility_domain" : "4f51", "mode" : "ap", "network" : "nat200", "ssid" : "uCentral-NAT.200" } }, { "band" : [ "5l", "5" ], "cfg" : { "auth_secret" : "uSyncRad1u5", "encryption" : "wpa2", "ft_over_ds" : 1, "ft_psk_generate_local" : 1, "ieee80211k" : 1, "ieee80211r" : 1, "ieee80211v" : 1, "mobility_domain" : "4f51", "mode" : "ap", "network" : "lan", "port" : 1812, "server" : "148.251.188.218", "ssid" : "uCentral-EAP" } }, { "band" : [ "5l", "5" ], "cfg" : { "encryption" : "psk2", "ft_over_ds" : 1, "ft_psk_generate_local" : 1, "ieee80211k" : 1, "ieee80211r" : 1, "ieee80211v" : 1, "key" : "OpenWifi", "mobility_domain" : "4f51", "mode" : "ap", "network" : "wan", "ssid" : "uCentral" } } ], "stats" : { "interval" : 60, "lldp" : 1, "neighbours" : 1, "pids" : 1, "poe" : 1, "serviceprobe" : 1, "system" : 1, "traffic" : 1, "wifiiface" : 1, "wifistation" : 1 }, "steer" : { "debug_level" : 0, "enabled" : 1, "network" : "wan" }, "system" : { "timezone" : "CET-1CEST,M3.5.0,M10.5.0\\\/3" }, "uuid" : 1614919758 }'

payload="{ \"serialNumber\" : \"$serial\", \"UUID\" : 123456 , \"configuration\" : $config }"

webtoken=`./login.sh | jq -r '.access_token'`

curl -X POST "https://localhost:16001/api/v1/device/$serial/configure" \
      -H  "accept: application/json" \
      -H "Authorization: Bearer $webtoken" \
      --insecure -d "$payload"


