#!/usr/bin/python3

import sys
import requests

filename = sys.argv[1]
ip = sys.argv[2]

print("Downloading file from http://{}".format(ip))
response = requests.get("http://{}:5058/{}".format(ip, filename))

with open(filename, "wb") as f:
	f.write(response.content)
