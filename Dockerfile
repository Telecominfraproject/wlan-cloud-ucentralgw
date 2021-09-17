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

ADD CMakeLists.txt build /ucentralgw/
ADD cmake /ucentralgw/cmake
ADD src /ucentralgw/src

WORKDIR /ucentralgw
RUN mkdir cmake-build
WORKDIR /ucentralgw/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM alpine

ENV UCENTRALGW_USER=ucentralgw \
    UCENTRALGW_ROOT=/ucentralgw-data \
    UCENTRALGW_CONFIG=/ucentralgw-data

RUN addgroup -S "$UCENTRALGW_USER" && \
    adduser -S -G "$UCENTRALGW_USER" "$UCENTRALGW_USER"

RUN mkdir /ucentral
RUN mkdir -p "$UCENTRALGW_ROOT" "$UCENTRALGW_CONFIG" && \
    chown "$UCENTRALGW_USER": "$UCENTRALGW_ROOT" "$UCENTRALGW_CONFIG"
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc su-exec gettext ca-certificates

COPY --from=builder /ucentralgw/cmake-build/ucentralgw /ucentral/ucentralgw
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

COPY ucentralgw.properties.tmpl ${UCENTRALGW_CONFIG}/
COPY docker-entrypoint.sh /

EXPOSE 15002 16002 16003 17002 16102

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/ucentral/ucentralgw"]
