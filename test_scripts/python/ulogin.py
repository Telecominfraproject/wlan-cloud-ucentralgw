#!/usr/bin/env python3
#
# See ../../openapi/ucentral/ucentral.yaml

"""
# Example usage
#
# List all devices
 ./ulogin.py --ucentral_host tip-f34.candelatech.com

# Configure 2-ssid setup with psk2 (aka wpa2) in NAT/routed mode
#
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
   --network24 lan --network5 lan

# Configure 2 ssid setup with psk2 in bridge mode.
# Use local cert downloaded from a remote ucentralgw
 ./ulogin.py --serno c4411ef52d0f \
   --ucentral_host tip-f34.candelatech.com --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
   --network bridge

# Configure 2 ssid setup with psk2 in NAT/Routed mode.
# Use local cert downloaded from a remote ucentralgw
 ./ulogin.py --serno c4411ef53f23 --cert ~/lab-ctlr-ucentral-cert.pem \
   --ucentral_host test-controller-1 --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
   --network24 lan --network5 lan

# Request AP upgrade to specified firmware.
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action upgrade \
   --url http://192.168.100.195/tip/openwrt-mediatek-mt7622-linksys_e8450-ubi-squashfs-sysupgrade.itb 

# Send request to AP.
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action request \
   --request state

# Get AP capabilities
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action show_capabilities

# Get AP status
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action show_status

# Get AP logs
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action show_logs

# Get AP healthcheck
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action show_healthcheck

# Get ucentral commands
 ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
   --ucentral_host test-controller-1 --action show_commands
"""

import json
from urllib.parse import urlparse
from pathlib import Path
import ssl
import requests
import argparse
from pprint import pprint

username = "support@example.com"
password = "support"
access_token = ""
assert_bad_response = True

parser = argparse.ArgumentParser()

parser.add_argument('--ucentral_host', help="Specify ucentral host name/ip.", default="ucentral")
parser.add_argument('--user_name', help="Specify ucentral username.", default="tip@ucentral.com")
parser.add_argument('--password', help="Specify ucentral password.", default="openwifi")

parser.add_argument('--cert', help="Specify ucentral cert.", default="")
parser.add_argument("--action", help="Specify action: show_stats | blink | show_commands | show_devices | show_capabilities | show_healthcheck | show_status | show_logs | cfg | upgrade | request .", default="")
parser.add_argument("--serno", help="Serial number of AP, used for some action.", default="")

parser.add_argument("--ssid24", help="Configure ssid for 2.4 Ghz.", default="ucentral-24")
parser.add_argument("--ssid5", help="Configure ssid for 5 Ghz.", default="ucentral-5")
#http://ucentral.io/docs/ucentral-schema.html#ssid_items_cfg_encryption
parser.add_argument("--encryption24", help="Configure encryption for 2.4 Ghz: none | psk | psk2 | psk-mixed | sae ...", default="psk2")
parser.add_argument("--encryption5", help="Configure encryption for 5Ghz: none | psk | psk2 | psk-mixed | sae ...", default="psk2")
parser.add_argument("--key24", help="Configure key/password for 2.4 Ghz.", default="ucentral")
parser.add_argument("--key5", help="Configure key/password for 5Ghz.", default="ucentral")
parser.add_argument("--network", help="bridge | nat.", default="bridge")

# Phy config
parser.add_argument("--channel24", help="Channel for 2.4Ghz, 0 means auto.", default="0")
parser.add_argument("--channel5", help="Channel for 5Ghz, 0 means auto.", default="0")
parser.add_argument("--mode24", help="Mode for 2.4Ghz, AUTO | HE20 | HT20 ...", default="AUTO")
parser.add_argument("--mode5", help="Mode for 5Ghz, AUTO | HE80 | VHT80 ...", default="AUTO")


parser.add_argument("--url", help="Specify URL for upgrading a device.", default="")
parser.add_argument("--request", help="Specify request for request action:  state | healthcheck.", default="state")

parser.add_argument("--verbose", help="Enable verbose logging.", default=False, action='store_true')
parser.add_argument("--noverify", help="Disable ssl cert verification.", default=False, action='store_true')

args = parser.parse_args()

uri = "https://" + args.ucentral_host + ":16001/api/v1/oauth2"

username = args.user_name
password = args.password
host = urlparse(uri)
access_token = ""
cert = args.cert

if Path(cert).is_file():
    print("Using local self-signed cert: ", cert)
    sslcontext = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    sslcontext.check_hostname = False
    if args.noverify:
        sslcontext.verify_mode = ssl.CERT_NONE
    else:
        sslcontext.verify_mode = ssl.CERT_REQUIRED
    sslcontext.load_verify_locations(cert)
else:
    context = True


def check_response(cmd, response, headers, data_str, url):
    if response.status_code >= 400 or args.verbose:
        if response.status_code >= 400:
            print("check-response: ERROR, url: ", url)
        else:
            print("check-response: url: ", url)
        print("Command: ", cmd)
        print("response-status: ", response.status_code)
        print("response-headers: ", response.headers)
        print("response-content: ", response.content)
        print("headers: ", headers)
        print("data-str: ", data_str)

    if response.status_code >= 400:
        if assert_bad_response:
            raise NameError("Invalid response code.")
        return False
    return True

def build_uri(path):
    global host
    new_uri = 'https://%s:%d/api/v1/%s' % (host.hostname, host.port, path)
    return new_uri


def make_headers():
    global access_token
    headers = {'Authorization': 'Bearer %s' % access_token}
    return headers


def login():
    global access_token, sslcontext
    uri = build_uri("oauth2")
    payload = json.dumps({"userId": username, "password": password})
    resp = requests.post(uri, data=payload, verify=cert)
    check_response("POST", resp, "", payload, uri)
    token = resp.json()
    access_token = token["access_token"]


def show_capabilities(serno):
    uri = build_uri("device/" + serno + "/capabilities")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    #pprint(data)
    # Parse the config before pretty-printing to make it more legible
    data = resp.json()
    pprint(data)
    #cfg = data['configuration']
    #pprint(cfg)
    #return cfg

def show_status(serno):
    uri = build_uri("device/" + serno + "/status")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    #pprint(data)
    # Parse the config before pretty-printing to make it more legible
    data = resp.json()
    pprint(data)
    #cfg = data['configuration']
    #pprint(cfg)
    #return cfg

def show_logs(serno):
    uri = build_uri("device/" + serno + "/logs")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    #pprint(data)
    # Parse the config before pretty-printing to make it more legible
    data = resp.json()
    pprint(data)
    #cfg = data['configuration']
    #pprint(cfg)
    #return cfg

def list_devices(serno):
    if serno != "":
        uri = build_uri("device/" + serno)
        resp = requests.get(uri, headers=make_headers(), verify=False)
        check_response("GET", resp, make_headers(), "", uri)
        #pprint(data)
        # Parse the config before pretty-printing to make it more legible
        data = resp.json()
        cfg = data['configuration']
        pprint(cfg)
        return cfg
    else:
        # Get all
        uri = build_uri("devices")
        resp = requests.get(uri, headers=make_headers(), verify=False)
        check_response("GET", resp, make_headers(), "", uri)
        data = resp.json()
        devices = data["devices"]
        print("Devices:\n")
        for d in devices:
            # Parse the config before pretty-printing to make it more legible
            #cfg = d['configuration']
            #d['configuration'] = json.loads(cfg)
            pprint(d)
        return devices

def list_device_stats(serno):
    uri = build_uri("device/" + serno + "/statistics")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    stats = data['data']
    for s in stats:
        print("Recorded: ", s['recorded'])
        pprint(s)
    return stats

def show_healthcheck(serno):
    uri = build_uri("device/" + serno + "/healthchecks")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    pprint(data)
    return data

def show_commands(serno):
    uri = build_uri("commands")
    if serno != "":
        uri += "?serialNumber="
        uri += serno

    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    pprint(data)
    return data

def upgrade_device(serno, url):
    uri = build_uri("device/" + serno + "/upgrade")
    payload = json.dumps({ "serialNumber": serno, "uri": url, "digest": "1234567890" })
    resp = requests.post(uri, data=payload, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), "", uri)

def do_request(serno, req):
    uri = build_uri("device/" + serno + "/request")
    payload = json.dumps({ "serialNumber": serno, "message": req})
    resp = requests.post(uri, data=payload, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), "", uri)

# Not sure this is working properly, it won't blink my e8450
def blink_device(serno):
    uri = build_uri("device/" + serno + "/blink")
    resp = requests.post(uri, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), "", uri)
    pprint(resp)

def cfg_device(args):
    # See also: https://github.com/Telecominfraproject/wlan-ap/tree/uCentral-staging-john/feeds/ucentral/ucentral-schema/files/etc/ucentral/examples
    # And http://ucentral.io/docs/ucentral-schema.html

    # Create json cfg file
    basic_cfg_text = """
{
  "uuid": 1,
  "radios": [
    {
      "band": "2G",
      "country": "US",
      "channel-mode": "HE",
      "channel-width": 20,
      "channel": 11
    },
    {
      "band": "5G",
      "country": "US",
      "channel-mode": "HE",
      "channel-width": 80,
      "channel": 36
    }
  ],

  "interfaces": [
    {
      "name": "WAN",
      "role": "upstream",
      "services": [ "lldp" ],
      "ethernet": [
        {
          "select-ports": [
            "WAN*"
          ]
        }
      ],
      "ipv4": {
        "addressing": "dynamic"
      },
      "ssids": [
        {
          "name": "OpenWifi",
          "wifi-bands": [
            "2G"
          ],
          "bss-mode": "ap",
          "encryption": {
            "proto": "psk2",
            "key": "OpenWifi",
            "ieee80211w": "optional"
          }
        },
        {
          "name": "OpenWifi",
          "wifi-bands": [
            "5G"
          ],
          "bss-mode": "ap",
          "encryption": {
            "proto": "psk2",
            "key": "OpenWifi",
            "ieee80211w": "optional"
          }
        }
      ]
    },
    {
      "name": "LAN",
      "role": "downstream",
      "services": [ "ssh", "lldp" ],
      "ethernet": [
        {
          "select-ports": [
            "LAN*"
          ]
        }
      ],
      "ipv4": {
        "addressing": "static",
        "subnet": "192.168.1.1/24",
        "dhcp": {
          "lease-first": 10,
          "lease-count": 100,
          "lease-time": "6h"
        }
      },
      "ssids": [
        {
          "name": "OpenWifi",
          "wifi-bands": [
            "2G"
          ],
          "bss-mode": "ap",
          "encryption": {
            "proto": "psk2",
            "key": "OpenWifi",
            "ieee80211w": "optional"
          }
        },
        {
          "name": "OpenWifi",
          "wifi-bands": [
            "5G"
          ],
          "bss-mode": "ap",
          "encryption": {
            "proto": "psk2",
            "key": "OpenWifi",
            "ieee80211w": "optional"
          }
        }
      ]

    }
  ],
  "metrics": {
    "statistics": {
      "interval": 120,
      "types": [ "ssids", "lldp", "clients" ]
    },
    "health": {
      "interval": 120
    }
  },
  "services": {
    "lldp": {
      "describe": "uCentral",
      "location": "universe"
    },
    "ssh": {
      "port": 22
    }
  }
}
"""
    basic_cfg = json.loads(basic_cfg_text)

    # And now modify it accordingly.
    if args.channel24 == "AUTO":
        if 'channel' in basic_cfg['radios'][0].keys():
            del basic_cfg['radios'][0]['channel']
    else:
        basic_cfg['radios'][0]['channel'] = int(args.channel24)

    if args.channel5 == "AUTO":
        if 'channel' in basic_cfg['radios'][1].keys():
            del basic_cfg['radios'][1]['channel']
    else:
        basic_cfg['radios'][1]['channel'] = int(args.channel5)

    if args.mode24 == "AUTO":
        if 'channel-mode' in basic_cfg['radios'][0].keys():
            del basic_cfg['radios'][0]['channel-mode']
    else:
        basic_cfg['radios'][0]['channel-mode'] = int(args.mode24)

    if args.mode5 == "AUTO":
        if 'channel-mode' in basic_cfg['radios'][1].keys():
            del basic_cfg['radios'][1]['channel-mode']
    else:
        basic_cfg['radios'][1]['channel-mode'] = int(args.mode5)

    if args.network == "bridge":
        # Remove LAN section.
        del basic_cfg['interfaces'][1]

        # Add lan ports to WAN section.
        basic_cfg['interfaces'][0]['ethernet'][0]['select-ports'].append("LAN*")

        basic_cfg['interfaces'][0]['ssids'][0]['name'] = args.ssid24
        basic_cfg['interfaces'][0]['ssids'][0]['encryption']['proto'] = args.encryption24
        basic_cfg['interfaces'][0]['ssids'][0]['encryption']['key'] = args.key24

        basic_cfg['interfaces'][0]['ssids'][1]['name'] = args.ssid5
        basic_cfg['interfaces'][0]['ssids'][1]['encryption']['proto'] = args.encryption5
        basic_cfg['interfaces'][0]['ssids'][1]['encryption']['key'] = args.key5

    if args.network == "nat":
        # Remove ssids from WAN sections.
        del basic_cfg['interfaces'][0]['ssids']

        basic_cfg['interfaces'][1]['ssids'][0]['name'] = args.ssid24
        basic_cfg['interfaces'][1]['ssids'][0]['encryption']['proto'] = args.encryption24
        basic_cfg['interfaces'][1]['ssids'][0]['encryption']['key'] = args.key24

        basic_cfg['interfaces'][1]['ssids'][1]['name'] = args.ssid5
        basic_cfg['interfaces'][1]['ssids'][1]['encryption']['proto'] = args.encryption5
        basic_cfg['interfaces'][1]['ssids'][1]['encryption']['key'] = args.key5

    payload = {}
    payload["configuration"] = basic_cfg
    payload['serialNumber'] = args.serno
    payload['UUID'] = 0


    print("Submitting config: ")
    pprint(payload)
    print("\n\n")

    basic_cfg_str = json.dumps(payload)

    print("data-string: ")
    print(basic_cfg_str)
    print("\n\n")

    uri = build_uri("device/" + args.serno + "/configure")
    resp = requests.post(uri, data=basic_cfg_str, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), basic_cfg_str, uri)
    pprint(resp)

def get_device_stats(serno):
    uri = build_uri("device/" + serno + "/statistics")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    return resp.json()

def get_devices():
    uri = build_uri("devices")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    devices = data["devices"]
    return devices


def logout():
    global access_token
    uri = build_uri('oauth2/%s' % access_token)
    resp = requests.delete(uri, headers=make_headers(), verify=False)
    check_response("DELETE", resp, make_headers(), "", uri)
    print('Logged out:', resp.status_code)


login()

if args.action == "" or args.action == "show_devices":
    list_devices(args.serno)
elif args.action == "show_stats":
    if args.serno == "":
        print("ERROR:  get_stats action needs serno set.\n")
    list_device_stats(args.serno)
elif args.action == "show_healthcheck":
    if args.serno == "":
        print("ERROR:  show_healthcheck action needs serno set.\n")
    show_healthcheck(args.serno)
elif args.action == "show_commands":
    show_commands(args.serno)
elif args.action == "show_capabilities":
    if args.serno == "":
        print("ERROR:  show_capabilities action needs serno set.\n")
    show_capabilities(args.serno)
elif args.action == "show_status":
    if args.serno == "":
        print("ERROR:  show_status action needs serno set.\n")
    show_status(args.serno)
elif args.action == "show_logs":
    if args.serno == "":
        print("ERROR:  show_logs action needs serno set.\n")
    show_logs(args.serno)
elif args.action == "upgrade":
    if args.serno == "":
        print("ERROR:  upgrade action needs serno set.\n")
    if args.url == "":
        print("ERROR:  upgrate needs URL set.\n")
    upgrade_device(args.serno, args.url)
elif args.action == "request":
    if args.serno == "":
        print("ERROR:  request action needs serno set.\n")
    if args.request == "":
        print("ERROR:  request action needs --request set.\n")
    do_request(args.serno, args.request)
elif args.action == "blink":
    if args.serno == "":
        print("ERROR:  blink action needs serno set.\n")
    blink_device(args.serno)
elif args.action == "cfg":
    if args.serno == "":
        print("ERROR:  cfg action needs serno set.\n")
    cfg_device(args)
else:
    print("Unknown action: ", args.action)

logout()





