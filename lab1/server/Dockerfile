# Reference: https://github.com/docker/awesome-compose/blob/master/nginx-flask-mongo/flask/Dockerfile

FROM python:3.11.2-alpine

WORKDIR /root
ENV FLASK_APP=server.py FLASK_RUN_HOST=0.0.0.0

RUN --mount=type=cache,target=/root/.cache/pip \
    pip3 install flask

EXPOSE 5000

COPY . .

CMD ["flask", "run"]

