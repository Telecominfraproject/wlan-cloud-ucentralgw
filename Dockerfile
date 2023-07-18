ARG DEBIAN_VERSION=11.5-slim
ARG POCO_VERSION=poco-tip-v2
ARG CPPKAFKA_VERSION=tip-v1
ARG VALIJASON_VERSION=tip-v1
ARG APP_NAME=owgw
ARG APP_HOME_DIR="/openwifi"

FROM debian:$DEBIAN_VERSION AS build-base

RUN apt-get update && apt-get install --no-install-recommends -y \
    make cmake g++ git \
    libpq-dev libmariadb-dev libmariadbclient-dev-compat \
    librdkafka-dev libboost-all-dev libssl-dev \
    zlib1g-dev nlohmann-json3-dev ca-certificates libfmt-dev

FROM build-base AS poco-build

ARG POCO_VERSION

ADD https://api.github.com/repos/AriliaWireless/poco/git/refs/tags/${POCO_VERSION} version.json
RUN git clone https://github.com/AriliaWireless/poco --branch ${POCO_VERSION} /poco

WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS cppkafka-build

ARG CPPKAFKA_VERSION

ADD https://api.github.com/repos/AriliaWireless/cppkafka/git/refs/tags/${CPPKAFKA_VERSION} version.json
RUN git clone https://github.com/AriliaWireless/cppkafka --branch ${CPPKAFKA_VERSION} /cppkafka

WORKDIR /cppkafka
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS valijson-build

ARG VALIJASON_VERSION

ADD https://api.github.com/repos/AriliaWireless/valijson/git/refs/tags/${VALIJASON_VERSION} version.json
RUN git clone https://github.com/AriliaWireless/valijson --branch ${VALIJASON_VERSION} /valijson

WORKDIR /valijson
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install

FROM build-base AS app-build

ADD CMakeLists.txt build /$APP_NAME/
ADD cmake /$APP_NAME/cmake
ADD src /$APP_NAME/src
ADD .git /$APP_NAME/.git

COPY --from=poco-build /usr/local/include /usr/local/include
COPY --from=poco-build /usr/local/lib /usr/local/lib
COPY --from=cppkafka-build /usr/local/include /usr/local/include
COPY --from=cppkafka-build /usr/local/lib /usr/local/lib
COPY --from=valijson-build /usr/local/include /usr/local/include

WORKDIR /$APP_NAME
RUN mkdir cmake-build
WORKDIR /$APP_NAME/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

FROM debian:$DEBIAN_VERSION

ENV APP_USER=$APP_NAME \
    APP_ROOT=/$APP_NAME-data \
    APP_CONFIG=/$APP_NAME-data \
    APP_NAME=$APP_NAME \
    APP_HOME_DIR=$APP_HOME_DIR

RUN echo "Vars: $APP_USER $APP_NAME"
RUN useradd "$APP_USER"

RUN mkdir $APP_HOME_DIR
RUN mkdir -p $APP_ROOT $APP_CONFIG" && \
    chown $APP_USER: $APP_ROOT $APP_CONFIG

RUN apt-get update && apt-get install --no-install-recommends -y \
    librdkafka++1 gosu gettext ca-certificates bash jq curl wget \
    libmariadb-dev-compat libpq5 unixodbc postgresql-client libfmt7 sqlite3

COPY readiness_check /readiness_check
COPY test_scripts/curl/cli /cli

COPY $APP_NAME.properties.tmpl /
COPY docker-entrypoint.sh /
COPY wait-for-postgres.sh /
COPY rtty_ui /dist/rtty_ui
RUN wget https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-ucentral-deploy/main/docker-compose/certs/restapi-ca.pem \
    -O /usr/local/share/ca-certificates/restapi-ca-selfsigned.crt

COPY --from=app-build /$APP_NAME/cmake-build/$APP_NAME $APP_HOME_DIR/$APP_NAME
COPY --from=cppkafka-build /cppkafka/cmake-build/src/lib /usr/local/lib/
COPY --from=poco-build /poco/cmake-build/lib /usr/local/lib/

RUN ldconfig

EXPOSE 15002 16002 16003 17002 16102

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["$APP_HOME_DIR/$APP_NAME"]
