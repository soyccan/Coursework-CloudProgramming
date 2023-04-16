FROM python:3.11.3-alpine3.17

WORKDIR /root

RUN \
  set -eux && \
  pip install msgpack-rpc-python requests

COPY . /root

CMD ["/bin/sh"]
