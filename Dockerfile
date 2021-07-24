FROM alpine AS builder

RUN apk update && \
    apk add --no-cache openssl openssh && \
    apk add --no-cache ncurses-libs && \
    apk add --no-cache bash util-linux coreutils curl && \
    apk add --no-cache make cmake gcc g++ libstdc++ libgcc git zlib-dev yaml-cpp-dev && \
    apk add --no-cache openssl-dev boost-dev unixodbc-dev postgresql-dev mariadb-dev && \
    apk add --no-cache apache2-utils yaml-dev apr-util-dev && \
    apk add --no-cache lua-dev librdkafka-dev

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

RUN addgroup -S ucentralgw && adduser -S -G ucentralgw ucentralgw

RUN mkdir /ucentral
RUN mkdir /ucentralgw-data
RUN apk add --update --no-cache librdkafka mariadb-connector-c libpq unixodbc

COPY --from=builder /ucentralgw/cmake-build/ucentralgw /ucentral/ucentralgw
COPY --from=builder /cppkafka/cmake-build/src/lib/* /lib/
COPY --from=builder /poco/cmake-build/lib/* /lib/

EXPOSE 15002
EXPOSE 16002
EXPOSE 16003
EXPOSE 17002
EXPOSE 16102

USER ucentralgw
ENTRYPOINT /ucentral/ucentralgw
