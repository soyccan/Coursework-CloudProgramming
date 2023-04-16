#!/bin/sh

minikube start --nodes 5 --insecure-registry 192.168.49.0/24 --addons registry,dashboard
minikube dashboard --port 2222
kubectl apply -f configs/local-pv.yaml
