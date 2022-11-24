# Restricted devices

## What is a restricted device?
A restricted device is one that because of regulations or a desire for utmost security, requires signatures to access restricted or blocked 
features. The restriction process is burnt in the device at manufacturing or later by running a specific command on the device. Once a device
is restricted, it cannot be unlocked. 

## Current restrictions
Restrictions are stored on the AP in a protected partition. They are contained in a file called `restrictions.json`. Here is a sample:
```json
{
        "country": [
                "US", "CA"
        ],
        "dfs": true,
        "rtty": true,
        "tty": true,
        "developer": true,
        "sysupgrade": true,
        "commands": true,
        "key_info": {
                "vendor": "dummy",
                "algo": "static"
        }
}
```
- country
  - List of countries where this device may be used
- dfs
  - Disallow DFS Override during wifi-scan. If set to `true`, device will not allow to override DFS channels
- rtty
  - Disallow the use of the RTTY command for this device
- tty
  - Do not allow the AP to accept `tty` connection
- developer
  - Internal use only.
- sysupgrade
  - If set to `true`, only signed firmware upgrade command will be allowed.
- commands
  - If set to `true`, do not allow commands.
- key_info
  - This structure defines how signatures should be generated and verified in a secure system
    - vendor
      - An identified that must match the vendor name provided in the controller
    - algo
      - The signature algorithm. Here are the supported algorithms
        - `static`
          - A test algorithm that always returns and uses a value of `aaaaaaaaaa`. This should never be used in the field.
        - `dgst-sha256`
          - The default OpenSSL RSA signature generation and verification. The controller will use the following command to generate the signature
          ```sh
          openssl dgst -sha256 -sign private-key.pem -out signature.txt myfile
          ```
          - The AP will be using the following to verify the signature
          ```sh
          openssl dgst -sha256 -verify public-key.pem -signature signature.txt myfile
          ```

## Creating signatures on the controller
When a device is restricted and a signature is required, the controller can generate the signature 
for the specified `vendor`. However, on the controlelr side, you must configure the vendors. In
order to do so we suggest the following.
- Create a directory called `signatures` under your `certs` directory
- Copy the public and private keys for each `vendor` name. We suggest naming them accordingly
  - <vendor>-private-key.pem
  - <vendor>-public-key.pem
- In the `owgw.properties` file, you need to declare these signatures the following way
```properties
    signature.manager.0.key.public = $OWGW_ROOT/certs/signatures/test1-public-key.pem
    signature.manager.0.key.private = $OWGW_ROOT/certs/signatures/test1-private-key.pem
    signature.manager.0.vendor = test1
    
    signature.manager.1.key.public = $OWGW_ROOT/certs/signatures/test2-public-key.pem
    signature.manager.1.key.private = $OWGW_ROOT/certs/signatures/test2-private-key.pem
    signature.manager.1.vendor = test2
```

## How do you use the signatures?
There is nothing to do really. Now the controller will use the proper key to create the signatures 
when it sends commands to the AP. It will use the algorithm that the device understands too. This is transparent 
to the user. The `vendor` name used in the controller configuration must match the `vendor` name provided in the 
`restrictions.json` file.
