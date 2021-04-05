#Build stage 0

FROM alpine

RUN apk update && \
    apk add --no-cache openssl openssh && \
    apk add --no-cache ncurses-libs && \
    apk add --no-cache bash util-linux coreutils curl && \
    apk add --no-cache make cmake gcc g++ libstdc++ libgcc git zlib-dev yaml-cpp-dev && \
    apk add --no-cache openssl-dev boost-dev unixodbc-dev postgresql-dev mariadb-dev && \
    apk add --no-cache apache2-utils yaml-dev apr-util-dev

RUN mkdir /root/.ssh
ADD git_rsa /root/.ssh/git_rsa
RUN touch /root/.ssh/known_hosts
RUN chown -R root:root /root/.ssh
RUN chmod 600 /root/.ssh/git_rsa && \
    echo "IdentityFile /root/.ssh/git_rsa" >> /etc/ssh/ssh_config && \
    echo -e "StrictHostKeyChecking no" >> /etc/ssh/ssh_config
RUN ssh-keyscan github.com >> /root/.ssh/known_hosts
RUN git clone git@github.com:stephb9959/ucentralgw.git /ucentralgw

RUN git clone https://github.com/stephb9959/poco /poco
WORKDIR /poco
RUN mkdir cmake-build
WORKDIR cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8
RUN cmake --build . --target install
WORKDIR /ucentralgw
RUN mkdir cmake-build
WORKDIR /ucentralgw/cmake-build
RUN cmake ..
RUN cmake --build . --config Release -j8

RUN mkdir /ucentral
RUN cp /ucentralgw/cmake-build/ucentral /ucentral/ucentral
RUN chmod +x /ucentral/ucentral
RUN mkdir /ucentral-data

RUN rm -rf /poco
RUN rm -rf /ucentralgw

EXPOSE 15002
EXPOSE 16001
EXPOSE 16003

ENTRYPOINT /ucentral/ucentral


