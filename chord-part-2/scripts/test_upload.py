#!/usr/bin/python3

import sys
import requests

filename = sys.argv[1]
ip = sys.argv[2]

files = {
	'files': open(filename, 'rb'),
}

print("Uploading file to http://{}".format(ip))
response = requests.post('http://{}:5058/upload'.format(ip), files=files)
