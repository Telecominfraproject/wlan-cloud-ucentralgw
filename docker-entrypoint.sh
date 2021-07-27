#!/bin/sh
set -e

if [ "$1" = '/ucentral/ucentralgw' -a "$(id -u)" = '0' ]; then
    chown -R "$UCENTRALGW_USER": "$UCENTRALGW_ROOT" "$UCENTRALGW_CONFIG"
    exec su-exec "$UCENTRALGW_USER" "$@"
fi

exec "$@"
