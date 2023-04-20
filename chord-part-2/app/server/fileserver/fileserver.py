# from flask import Flask, jsonify, json, Response, request
import flask
import sys
import requests
import msgpackrpc
import hashlib

CHORD_HOSTNAME = "chord.chord.svc"

app = flask.Flask(__name__)


# The service basepath has a short response just to ensure that healthchecks
# sent to the service root will receive a healthy response.
@app.route("/")
def healthCheck():
    return flask.jsonify({"message": "Nothing here, used for health check."})


@app.route("/<filename>")
def getFile(filename):
    client = new_client(CHORD_HOSTNAME, 5057)
    h = hash(filename)
    print("Hash of {} is {}".format(filename, h))

    node = client.call("find_successor", h)
    node_ip = node[0].decode()

    print("Downloading file from http://{}".format(node_ip))
    response = requests.get("http://{}:5058/{}".format(node_ip, filename))

    return response


@app.route("/", methods=["POST"])
def uploadFile():
    filename = flask.request.files.get("files").filename
    client = new_client(CHORD_HOSTNAME, 5057)
    h = hash(filename)
    print("Hash of {} is {}".format(filename, h))

    node = client.call("find_successor", h)
    node_ip = node[0].decode()

    print("Uploading file to http://{}".format(node_ip))
    response = requests.post(
        "http://{}:5058/upload".format(node_ip), files=flask.request.files
    )

    return response


def new_client(ip, port):
    return msgpackrpc.Client(msgpackrpc.Address(ip, port), timeout=1)


def hash(str):
    return int(hashlib.md5(str.encode()).hexdigest(), 16) & ((1 << 32) - 1)


# Run the service on the local server it has been deployed to,
# listening on port 8080.
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)
