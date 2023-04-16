FROM alpine:3.17.3

ENV CHORD_IP 127.0.0.1
ENV CHORD_PORT 5057
EXPOSE ${CHORD_PORT}
WORKDIR /workdir

RUN \
  set -eux && \
  addgroup -g 1000 user && \
  adduser -u 1000 -G user -s /bin/sh -h /home/user -D user && \
  chown -R user:user /workdir

COPY chord /usr/local/bin
USER user

CMD /usr/local/bin/chord ${CHORD_IP} ${CHORD_PORT}
