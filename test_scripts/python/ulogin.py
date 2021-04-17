#!/usr/bin/env python3

# Example
# ./test_scripts/python/ulogin.py --ucentral_host bendt7 --cert ~/git/tip/ucentral-local/certs/server-cert.pem
#
# Configure 2-ssid setup with psk2 (aka wpa2) in NAT/routed mode
#
# ./ulogin.py --serno c4411ef53f23 --cert ~/git/tip/ucentral-local/certs/server-cert.pem \
#   --ucentral_host test-controller-1 --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
#   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
#   --network24 lan --network5 lan
#
# Configure 2 ssid setup with psk2 in bridge mode.
# Use local cert downloaded from a remote ucentralgw
# ./ulogin.py --serno c4411ef53f23 --cert ~/lab-ctlr-ucentral-cert.pem \
#   --ucentral_host test-controller-1 --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
#   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
#   --network24 wan --network5 wan

# Configure 2 ssid setup with psk2 in NAT/Routed mode.
# Use local cert downloaded from a remote ucentralgw
# ./ulogin.py --serno c4411ef53f23 --cert ~/lab-ctlr-ucentral-cert.pem \
#   --ucentral_host test-controller-1 --ssid24 Default-SSID-2g --ssid5 Default-SSID-5gl \
#   --key24 12345678 --key5 12345678 --encryption24 psk2 --encryption5 psk2 --action cfg \
#   --network24 lan --network5 lan


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
parser.add_argument('--cert', help="Specify ucentral cert.", default="cert.pem")
parser.add_argument("--action", help="Specify action: show_stats | blink | show_devices | cfg .", default="")
parser.add_argument("--serno", help="Serial number of AP, used for some action.", default="")

parser.add_argument("--ssid24", help="Configure ssid for 2.4 Ghz.", default="ucentral-24")
parser.add_argument("--ssid5", help="Configure ssid for 5 Ghz.", default="ucentral-5")
#http://ucentral.io/docs/ucentral-schema.html#ssid_items_cfg_encryption
parser.add_argument("--encryption24", help="Configure encryption for 2.4 Ghz: none | psk | psk2 | psk-mixed | sae ...", default="psk2")
parser.add_argument("--encryption5", help="Configure encryption for 5Ghz: none | psk | psk2 | psk-mixed | sae ...", default="psk2")
parser.add_argument("--key24", help="Configure key/password for 2.4 Ghz.", default="ucentral")
parser.add_argument("--key5", help="Configure key/password for 5Ghz.", default="ucentral")
parser.add_argument("--network24", help="'lan' for NAT mode, 'wan' for bridged.", default="lan")
parser.add_argument("--network5", help="'lan' for NAT mode, 'wan' for bridged.", default="lan")

# Phy config
parser.add_argument("--channel24", help="Channel for 2.4Ghz, 0 means auto.", default="0")
parser.add_argument("--channel5", help="Channel for 5Ghz, 0 means auto.", default="0")
parser.add_argument("--mode24", help="Mode for 2.4Ghz, AUTO | HE20 | HT20 ...", default="AUTO")
parser.add_argument("--mode5", help="Mode for 5Ghz, AUTO | HE80 | VHT80 ...", default="AUTO")


parser.add_argument("--verbose", help="Enable verbose logging.", default=False, action='store_true')

args = parser.parse_args()

uri = "https://" + args.ucentral_host + ":16001/api/v1/oauth2"
username = "support@example.com"
password = "support"
host = urlparse(uri)
access_token = ""
cert = args.cert

if Path(cert).is_file():
    print("Using local self-signed cert: ", cert)
    sslcontext = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    sslcontext.verify_mode = ssl.CERT_REQUIRED
    sslcontext.check_hostname = False
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
    new_uri = 'https://%s:%d/%s' % (host.hostname, host.port, path)
    return new_uri


def make_headers():
    global access_token
    headers = {'Authorization': 'Bearer %s' % access_token}
    return headers


def login():
    global access_token, sslcontext
    uri = build_uri("api/v1/oauth2")
    payload = json.dumps({"userId": username, "password": password})
    resp = requests.post(uri, data=payload, verify=cert)
    check_response("POST", resp, "", payload, uri)
    token = resp.json()
    access_token = token["access_token"]


def list_devices(serno):
    if serno != "":
        uri = build_uri("api/v1/device/" + serno)
        resp = requests.get(uri, headers=make_headers(), verify=False)
        check_response("GET", resp, make_headers(), "", uri)
        #pprint(data)
        # Parse the config before pretty-printing to make it more legible
        data = resp.json()
        cfg = data['configuration']
        data['configuration'] = json.loads(cfg)
        pprint(data)
    else:
        # Get all
        uri = build_uri("api/v1/devices")
        resp = requests.get(uri, headers=make_headers(), verify=False)
        check_response("GET", resp, make_headers(), "", uri)
        data = resp.json()
        devices = data["devices"]
        print("Devices:\n")
        for d in devices:
            # Parse the config before pretty-printing to make it more legible
            cfg = d['configuration']
            d['configuration'] = json.loads(cfg)
            pprint(d)

def list_device_stats(serno):
    uri = build_uri("api/v1/device/" + serno + "/statistics")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    stats = data['data']
    for s in stats:
        print("Recorded: ", s['recorded'])
        stats_data = json.loads(s['data'])
        pprint(stats_data)

# Not sure this is working properly, it won't blink my e8450
def blink_device(serno):
    uri = build_uri("api/v1/device/" + serno + "/blink")
    resp = requests.post(uri, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), "", uri)
    pprint(resp)

def cfg_device(args):
    # Create json cfg file
    basic_cfg_text = """
{
  "serialNumber": "FILL_ME_IN",
  "UUID": 0,
  "configuration":
{
 "uuid": 1,
 "stats": {
  "interval": 10,
  "neighbours": 1,
  "traffic": 1,
  "wifiiface": 1,
  "wifistation": 1,
  "pids": 1,
  "serviceprobe": 1,
  "lldp": 1,
  "system": 1,
  "poe": 1
 },
"phy": [
  {
   "band": "2",
   "cfg": {
    "disabled": 0,
    "channel": 6
   }
  },
  {
   "band": "5",
   "cfg": {
    "disabled": 0,
    "country": "US",
    "channel": 0,
    "htmode": "HE80"
   }
  }
 ],
 "ssid": [
  {
   "band": [
    "2"
   ],
   "cfg": {
    "ssid": "uCentral",
    "encryption": "psk2",
    "key": "aaaaaaaa",
    "mode": "ap",
    "network": "lan"
  }
 },
  {
   "band": [
    "5u",
    "5"
   ],
   "cfg": {
    "ssid": "uCentral",
    "encryption": "psk2",
    "key": "aaaaaaaa",
    "mode": "ap",
    "network": "lan"
   }
  }
 ],
 "network": [
  {
   "mode": "wan",
   "cfg": {
    "proto": "dhcp"
   }
  },
  {
   "mode": "lan",
   "cfg": {
    "proto": "static",
    "ipaddr": "192.168.1.1",
    "dhcp": {
     "start": 10,
     "limit": 240,
     "leasetime": "6h"
    }
   }
  }
 ]
}
}
"""
    basic_cfg = json.loads(basic_cfg_text)

    basic_cfg['serialNumber'] = args.serno

    # And now modify it accordingly.
    if args.channel24 == "AUTO":
        basic_cfg['configuration']['phy'][0]['cfg']['channel'] = 0
    else:
        basic_cfg['configuration']['phy'][0]['cfg']['channel'] = int(args.channel24)

    if args.channel5 == "AUTO":
        basic_cfg['configuration']['phy'][1]['cfg']['channel'] = 0
    else:
        basic_cfg['configuration']['phy'][1]['cfg']['channel'] = int(args.channel5)

    if args.mode24 == "AUTO":
        if 'htmode' in basic_cfg['configuration']['phy'][0]['cfg'].keys():
            del basic_cfg['configuration']['phy'][0]['cfg']['htmode']
    else:
        basic_cfg['configuration']['phy'][0]['cfg']['htmode'] = int(args.mode24)

    if args.mode5 == "AUTO":
        if 'htmode' in basic_cfg['configuration']['phy'][1]['cfg'].keys():
            del basic_cfg['configuration']['phy'][1]['cfg']['htmode']
    else:
        basic_cfg['configuration']['phy'][1]['cfg']['htmode'] = int(args.mode5)

    basic_cfg['configuration']['ssid'][0]['cfg']['ssid'] = args.ssid24
    basic_cfg['configuration']['ssid'][0]['cfg']['encryption'] = args.encryption24
    basic_cfg['configuration']['ssid'][0]['cfg']['key'] = args.key24
    basic_cfg['configuration']['ssid'][0]['cfg']['network'] = args.network24

    basic_cfg['configuration']['ssid'][1]['cfg']['ssid'] = args.ssid5
    basic_cfg['configuration']['ssid'][1]['cfg']['encryption'] = args.encryption5
    basic_cfg['configuration']['ssid'][1]['cfg']['key'] = args.key5
    basic_cfg['configuration']['ssid'][1]['cfg']['network'] = args.network5

    basic_cfg_str = json.dumps(basic_cfg)

    uri = build_uri("api/v1/device/" + args.serno + "/configure")
    resp = requests.post(uri, data=basic_cfg_str, headers=make_headers(), verify=False)
    check_response("POST", resp, make_headers(), basic_cfg_str, uri)
    pprint(resp)

def get_device_stats(serno):
    uri = build_uri("api/v1/device/" + serno + "/statistics")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    return resp.json()

def get_devices():
    uri = build_uri("api/v1/devices")
    resp = requests.get(uri, headers=make_headers(), verify=False)
    check_response("GET", resp, make_headers(), "", uri)
    data = resp.json()
    devices = data["devices"]
    return devices


def logout():
    global access_token
    uri = build_uri('api/v1/oauth2/%s' % access_token)
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





