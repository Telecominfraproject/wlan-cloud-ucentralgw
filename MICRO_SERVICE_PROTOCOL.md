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

### Responsibilities
Each micro service is responsible to generate its own messages and keep track of messages coming from other 
micro services. This is necessary so that any micro service may reach our any other micro service. This provides 
discovery for any micro service. All current micro services provided in OpenWiFi perform these functions. If you leverage 
the C++ framework, this functionality if performed automatically.

### `event-type`
Each micro service is responsible to generate and consume these events

#### `join` event
When a service start and joins the bus, it should generate an event-type of `join`. 

### `leave` event
When a service shuts down, it should generate a `leave` event-type. 

### `keep-alive` event
Every 30 seconds, a service should generate a `keep-alive` message.


### `id`
You should generate a random number from some unique factor for the system. This ID is used to identify different services. You should reuse that ID 
when you restart.

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
Assume you want to communicate with the gateway to configure a device. 

```text
1. Look into my list of current Micro-services for the type=owgw.
2. Use the privateEndPoint associated with that entry
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

## OpenAPI Integration
To appear in the UI consoles, a microservice should be able to handle a get to the `/api/v1/system` endpoint on its `publicEndPoint` interface.

Here is a brief description of what the microservice should answer:
```yaml
  /system:
    get:
      tags:
        - System Commands
      summary: Retrieve different values from the running service.
      operationId: getSystemCommand
      parameters:
        - in: query
          description: Get a value
          name: command
          schema:
            type: string
            enum:
              - info
          required: true

      responses:
        200:
          description: Successful command execution
          content:
            application/json:
              schema:
                oneOf:
                  - $ref: '#/components/schemas/SystemInfoResults'
        403:
          $ref: '#/components/responses/Unauthorized'
        404:
          $ref: '#/components/responses/NotFound'
```
The relevant data structures are:
```yaml
    SystemInfoResults:
      type: object
      properties:
        version:
          type: string
        uptime:
          type: integer
          format: integer64
        start:
          type: integer
          format: integer64
        os:
          type: string
        processors:
          type: integer
        hostname:
          type: string
        certificates:
          type: array
          items:
            type: object
            properties:
              filename:
                type: string
              expires:
                type: integer
                format: int64

```
and 
```yaml
  responses:
    NotFound:
      description: The specified resource was not found.
      content:
        application/json:
          schema:
            properties:
              ErrorCode:
                type: integer
              ErrorDetails:
                type: string
              ErrorDescription:
                type: string

    Unauthorized:
      description: The requested does not have sufficient rights to perform the operation.
      content:
        application/json:
          schema:
            properties:
              ErrorCode:
                type: integer
                enum:
                  - 0     # Success
                  - 1     # PASSWORD_CHANGE_REQUIRED,
                  - 2     # INVALID_CREDENTIALS,
                  - 3     # PASSWORD_ALREADY_USED,
                  - 4     # USERNAME_PENDING_VERIFICATION,
                  - 5     # PASSWORD_INVALID,
                  - 6     # INTERNAL_ERROR,
                  - 7     # ACCESS_DENIED,
                  - 8     # INVALID_TOKEN
                  - 9     # EXPIRED_TOKEN
                  - 10    # RATE_LIMIT_EXCEEDED
                  - 11    # BAD_MFA_TRANSACTION
                  - 12    # MFA_FAILURE
                  - 13    # SECURITY_SERVICE_UNREACHABLE
              ErrorDetails:
                type: string
              ErrorDescription:
                type: string
```