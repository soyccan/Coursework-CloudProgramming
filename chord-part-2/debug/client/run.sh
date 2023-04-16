#!/bin/sh

kubectl run client --image $(minikube ip):5000/client --rm -it
