FROM python:3.11.2-alpine

WORKDIR /root

RUN --mount=type=cache,target=/root/.cache/pip \
    pip3 install requests

COPY . .

CMD ["python3", "client.py"]
