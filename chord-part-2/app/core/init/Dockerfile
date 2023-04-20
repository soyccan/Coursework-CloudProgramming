FROM python:3.11.3-alpine3.17

WORKDIR /workdir

RUN \
  set -eux && \
  addgroup -g 1000 user && \
  adduser -u 1000 -G user -s /bin/sh -h /home/user -D user && \
  chown -R user:user /workdir && \
  pip install msgpack-rpc-python

# TODO: debug utils should not exist in production
COPY --chown=user:user _dbg.py /workdir

COPY --chown=user:user init.py /home/user
USER user

CMD ["python3", "/home/user/init.py"]
