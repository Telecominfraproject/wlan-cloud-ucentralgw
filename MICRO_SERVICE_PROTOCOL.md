# Micro-service backbone responsibilities

## Bus management
Each microservice must get onto kafka and consume/produce messages on the kafka bus. The topic to subscribe to is `service_events`. 

## System messages
System messages are what maintains the collection of micro-services working on the system. Each message has the format

```json
{
  "event": <event-type>,
  "id": 1234567890,
  "type": "owrrm",
  "publicEndPoint": "https://myhostname.com:16020",
  "privateEndPoint": "https://localhost:17020",
  "key" : "289479847948794870749",
  "version" : "1.0"
}
```

### `event-type`
When a service joins the bus, it should generate an event-type of `join`. When a service shutdown, it should generate a `leave` event-type. Every 30 seconds, a service 
should generate a `keep-alive` message.

### `id`
You should generate a random number from some unique factor for the system. This ID is used to identify different services. You should reuse that ID 
when you restart.

## Micro-service maintaining bus state
A micro-service should maintain its own lists of available micro-services by looking at the messages it receives and keep a list.

## The `type`
The `type` in the system message is oen of the following:
```c++
	static const std::string uSERVICE_SECURITY{"owsec"};
	static const std::string uSERVICE_GATEWAY{"owgw"};
	static const std::string uSERVICE_FIRMWARE{ "owfms"};
    static const std::string uSERVICE_TOPOLOGY{ "owtopo"};
    static const std::string uSERVICE_PROVISIONING{ "owprov"};
    static const std::string uSERVICE_OWLS{ "owls"};
    static const std::string uSERVICE_SUBCRIBER{ "owsub"};
    static const std::string uSERVICE_INSTALLER{ "owinst"};
    static const std::string uSERVICE_ANALYTICS{ "owanalytics"};
	static const std::string uSERVICE_OWRRM{ "owrrm"};
```

The `type` is what you should use to find the `privateEndPoint` you are looking to communicate with.

### Example
Assume you want to communicate with the gateway t pconfigure a device. 

```text
1. Look into my list of current Micro-services for the type=owgw.
2. Use the priovateEndPoint associated with that entry
```

## REST API calls on the private interface
For inter-service REST calls, you should never use the `Authorization: Bearer token` method. Instead, the following headers should be included in all API calls
```json
{
  "X-API-KEY" : "289479847948794870749",
  "X-INTERNAL-NAME" : "https://myhostname.com:16020"
}
```

### `X-API-KEY`
This is the `key` you included in your `system-messages`. 

### `X-INTERNAL-NAME`
This is the `publicEndPoint` you included in your `system-messages`.

This method can _only_ be used to any another `privateEndPoint` in the system. You can use the exact same EndPoints provided in the OpenAPI files for any of the services.
