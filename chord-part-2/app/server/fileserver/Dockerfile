FROM python:3.11.3-alpine3.17

EXPOSE 80
WORKDIR /workdir

RUN \
  set -eux && \
  addgroup -g 1000 user && \
  adduser -u 1000 -G user -s /bin/sh -h /home/user -D user && \
  chown -R user:user /workdir && \
  pip install flask

USER user

CMD ["python", "fileserver.py"]
