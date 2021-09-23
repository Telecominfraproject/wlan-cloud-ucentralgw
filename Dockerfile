FROM alpine AS builder

RUN apk add --update --no-cache \
    openssl openssh \
    ncurses-libs \
    bash util-linux coreutils curl \
    make cmake gcc g++ libstdc++ libgcc git zlib-dev yaml-cpp-dev \
    openssl-dev boost-dev unixodbc-dev postgresql-dev mariadb-dev \
    apache2-utils yaml-dev apr-util-dev \
    lua-dev librdkafka-dev

RUN git clone https://github.com/stephb9959/poco /poco
RUN git clone https://github.com/stephb9959/cppkafka /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

ADD CMakeLists.txt build /owgw/
ADD cmake /owgw/cmake
ADD src /owgw/src

WORKDIR /owgw
RUN mkdir cmake-build
WORKDIR /owgw/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine

ENV OWGW_USER=owgw \
    OWGW_ROOT=/owgw-data \
    OWGW_CONFIG=/owgw-data

RUN addgroup -S "$OWGW_USER" && \
    adduser -S -G "$OWGW_USER" "$OWGW_USER"

RUN mkdir /openwifi
RUN mkdir -p "$OWGW_ROOT" "$OWGW_CONFIG" && \
    chown "$OWGW_USER": "$OWGW_ROOT" "$OWGW_CONFIG"
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc su-exec gettext ca-certificates

COPY --from=builder /owgw/cmake-build/owgw /openwifi/owgw
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

COPY owgw.properties.tmpl ${OWGW_CONFIG}/
COPY docker-entrypoint.sh /
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.pem 

EXPOSE 15002 16002 16003 17002 16102

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/openwifi/owgw"]
