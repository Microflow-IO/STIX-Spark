FROM registry.jxit.net.cn:5000/alpine:3.19.1 AS builder
# FROM registry.jxit.net.cn:5000/alpine:3.19.1-arm

RUN sed -i 's@dl-cdn.alpinelinux.org@mirrors.tencent.com@g' /etc/apk/repositories && \
    apk update && \
    apk add --no-cache tcpdump vim curl build-base gcc zlib-dev tzdata pcre-dev && \
    cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone

WORKDIR /unistix

COPY . /unistix

RUN make install && make -f make-mkfile install

FROM registry.jxit.net.cn:5000/alpine:3.19.1

RUN sed -i 's@dl-cdn.alpinelinux.org@mirrors.tencent.com@g' /etc/apk/repositories && \
    apk update && \
    apk add --no-cache tcpdump vim curl pcre zlib tzdata && \
    cp /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone

ADD stix-all.tar.gz /unistix

WORKDIR /unistix/stix-all

COPY --from=builder /usr/local/bin/unistix /usr/local/bin/unistix
COPY --from=builder /usr/local/bin/mkfile /usr/local/bin/mkfile
