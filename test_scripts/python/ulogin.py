#!/usr/bin/env python3

import json
from urllib.parse import urlparse
from pathlib import Path
import ssl
import requests


uri = "https://localhost:16001/api/v1/oauth2"
username = "support@example.com"
password = "support"
host = urlparse(uri)
access_token = ""


if Path("cert.pem").is_file():
    print("Using local self-signed cert.pem")
    sslcontext = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    sslcontext.verify_mode = ssl.CERT_REQUIRED
    sslcontext.check_hostname = False
    sslcontext.load_verify_locations("cert.pem")
else:
    context = True


def build_uri(path):
    global host
    new_uri = 'https://%s:%d/%s' % (host.hostname, host.port, path)
    return new_uri


def make_headers():
    global access_token
    headers = {'Authorization': 'Bearer %s' % access_token}
    return headers


def login():
    global access_token
    payload = json.dumps({"userId": username, "password": password})
    data = requests.post(build_uri("api/v1/oauth2"), data=payload, verify=False)
    token = data.json()
    access_token = token["access_token"]


def list_devices():
    data = requests.get(build_uri("api/v1/devices"), headers=make_headers(), verify=False).json()
    devices = data["devices"]
    print('Devices:', devices)


def logout():
    global access_token
    data = requests.delete(build_uri('api/v1/oauth2/%s' % access_token), headers=make_headers(), verify=False)
    print('Logged out:', data.status_code)


login()
list_devices()
logout()





