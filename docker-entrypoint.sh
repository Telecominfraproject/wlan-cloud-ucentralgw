#!/bin/sh
set -e

if [ "$1" = '/ucentral/ucentralgw' -a "$(id -u)" = '0' ]; then
    if [ "$RUN_CHOWN" = 'true' ]; then
      chown -R "$UCENTRALGW_USER": "$UCENTRALGW_ROOT" "$UCENTRALGW_CONFIG"
    fi
    exec su-exec "$UCENTRALGW_USER" "$@"
fi

exec "$@"
