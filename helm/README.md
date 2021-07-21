# ucentralgw

This Helm chart helps to deploy uCentralGW to the Kubernetes clusters. It is mainly used in [assembly chart](https://github.com/Telecominfraproject/wlan-cloud-ucentral-deploy/tree/main/chart) as uCentralGW requires other services as dependencies that are considered in that Helm chart. This chart is purposed to define deployment logic close to the application code itself and define default values that could be overriden during deployment.


## TL;DR;

```bash
$ helm install .
```

## Introduction

This chart bootstraps an ucentralgw on a [Kubernetes](http://kubernetes.io) cluster using the [Helm](https://helm.sh) package manager.

## Installing the Chart

Currently this chart is not assembled in charts archives, so [helm-git](https://github.com/aslafy-z/helm-git) is required for remote the installation

To install the chart with the release name `my-release`:

```bash
$ helm install --name my-release git+https://github.com/Telecominfraproject/wlan-cloud-ucentralgw@helm?ref=master
```

The command deploys ucentralgw on the Kubernetes cluster in the default configuration. The [configuration](#configuration) section lists the parameters that can be configured during installation.

> **Tip**: List all releases using `helm list`

## Uninstalling the Chart

To uninstall/delete the `my-release` deployment:

```bash
$ helm delete my-release
```

The command removes all the Kubernetes components associated with the chart and deletes the release.

## Configuration

The following table lists the configurable parameters of the chart and their default values. If Default value is not listed in the table, please refer to the [Values](values.yaml) files for details.

| Parameter | Type | Description | Default |
|-----------|------|-------------|---------|
| replicaCount | number | Amount of replicas to be deployed | `1` |
| strategyType | string | Application deployment strategy | `'Recreate'` |
| nameOverride | string | Override to be used for application deployment |  |
| fullnameOverride | string | Override to be used for application deployment (has priority over nameOverride) |  |
| images.ucentralgw.repository | string | Docker image repository |  |
| images.ucentralgw.tag | string | Docker image tag | `'master'` |
| images.ucentralgw.pullPolicy | string | Docker image pull policy | `'Always'` |
| services.ucentralgw.type | string | uCentralGW service type | `'LoadBalancer'` |
| services.ucentralgw.ports.websocket.servicePort | number | Websocket endpoint port to be exposed on service | `15002` |
| services.ucentralgw.ports.websocket.targetPort | number | Websocket endpoint port to be targeted by service | `15002` |
| services.ucentralgw.ports.websocket.protocol | string | Websocket endpoint protocol | `'TCP'` |
| services.ucentralgw.ports.restapi.servicePort | number | REST API endpoint port to be exposed on service | `16002` |
| services.ucentralgw.ports.restapi.targetPort | number | REST API endpoint port to be targeted by service | `16002` |
| services.ucentralgw.ports.restapi.protocol | string | REST API endpoint protocol | `'TCP'` |
| services.ucentralgw.ports.restapiinternal.servicePort | string | Internal REST API endpoint port to be exposed on service | `17002` |
| services.ucentralgw.ports.restapiinternal.targetPort | number | Internal REST API endpoint port to be targeted by service | `17002` |
| services.ucentralgw.ports.restapiinternal.protocol | string | Internal REST API endpoint protocol | `'TCP'` |
| services.ucentralgw.ports.fileuploader.servicePort | string | Fileuploader endpoint port to be exposed on service | `16003` |
| services.ucentralgw.ports.fileuploader.targetPort | number | Fileuploader endpoint port to be targeted by service | `16003` |
| services.ucentralgw.ports.fileuploader.protocol | string | Fileuploader endpoint protocol | `'TCP'` |
| checks.ucentralgw.liveness.httpGet.path | string | Liveness check path to be used | `'/'` |
| checks.ucentralgw.liveness.httpGet.port | number | Liveness check port to be used (should be pointint to ALB endpoint) | `16102` |
| checks.ucentralgw.readiness.httpGet.path | string | Readiness check path to be used | `'/'` |
| checks.ucentralgw.readiness.httpGet.port | number | Readiness check port to be used (should be pointint to ALB endpoint) | `16102` |
| ingresses.restapi.enabled | boolean | Defines if REST API endpoint should be exposed via Ingress controller | `False` |
| ingresses.restapi.hosts | array | List of hosts for exposed REST API |  |
| ingresses.restapi.paths | array | List of paths to be exposed for REST API |  |
| ingresses.fileuploader.enabled | boolean | Defines if Fileuploader endpoint should be exposed via Ingress controller | `False` |
| ingresses.fileuploader.hosts | array | List of hosts for exposed Fileuploader |  |
| ingresses.fileuploader.paths | array | List of paths for exposed Fileuploader |  |
| volumes.ucentralgw | array | Defines list of volumes to be attached to uCentralGW |  |
| persistence.enabled | boolean | Defines if uCentralGW requires Persistent Volume (required for permanent files storage and SQLite DB if enabled) | `True` |
| persistence.accessModes | array | Defines PV access modes |  |
| persistence.size | string | Defines PV size | `'10Gi'` |
| public_env_variables | hash | Defines list of environment variables to be passed to uCentralGW | |
| configProperties | hash | Configuration properties that should be passed to the application in `ucentralgw.properties`. May be passed by key in set (i.e. `configProperties."rtty\.token"`) | |
| certs | hash | Defines files (keys and certificates) that should be passed to uCentralGW (PEM format is adviced to be used) (see `volumes.ucentralgw` on where it is mounted) |  |
| certsCAs | hash | Defines files with CAs that should be passed to uCentralGW (see `volumes.ucentralgw` on where it is mounted) |  |


Specify each parameter using the `--set key=value[,key=value]` argument to `helm install`. For example,

```bash
$ helm install --name my-release \
  --set replicaCount=1 \
    .
```

The above command sets that only 1 instance of your app should be running

Alternatively, a YAML file that specifies the values for the parameters can be provided while installing the chart. For example,

```bash
$ helm install --name my-release -f values.yaml .
```

> **Tip**: You can use the default [values.yaml](values.yaml) as a base for customization.


