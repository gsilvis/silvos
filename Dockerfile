FROM cstanfill/silvos:latest

COPY . /root/silvos

WORKDIR /root/silvos

RUN make

CMD make test
