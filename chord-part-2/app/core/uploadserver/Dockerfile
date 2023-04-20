FROM python:3.11.3-alpine3.17

ENV PORT 5058
EXPOSE ${PORT}
WORKDIR /files

RUN \
  set -eux && \
  addgroup -g 1000 user && \
  adduser -u 1000 -G user -s /bin/sh -h /home/user -D user && \
  chown -R user:user /files && \
  pip install uploadserver

USER user

CMD python -m uploadserver ${PORT}
