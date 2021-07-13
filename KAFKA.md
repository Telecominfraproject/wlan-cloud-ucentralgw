# Kafka Integration
This is a fast moving target, so please visit often or set an alert in GitHub.

## Current plans
Currently, most telemetry and reports created in the gateway will be issues as topics in Kafka. You should configure 
your Kafka service with the following topics:

- `connection` : This is emitted whenever a device connects to the gateway. The report contains all ths information about the connection. 
- `state` : This is emitted for every `state` report coming from the AP. This state report contains all the information of state reports.  
- `healthcheck` : These are the `healthcheck` report sent from the AP.
- `wifiscan` : Whenever a `wifiscan` report is generated, it will be submitted here.
- `service_events` : Inter-service traffic.
- `security` : This will have application information (future use).
- `command` : Allow to send commands (future use).
- `alerts` : Alerts originating from devices (future use).

## Structure of `kafka` messages
Messages use 2 formats

### Device messages
Device from a device include a preamble that identifies the gateway responsible for the message. The payload 
includes the message itself.

### Inter-service messages
No preamble is used here. The entire payload is the message. The `key` represents the source of the message.

## Want more?
Let us know what else you would like to see in `kafka`. Or better, don't be shy and contribute something. We need more of you 
help make this into a success.