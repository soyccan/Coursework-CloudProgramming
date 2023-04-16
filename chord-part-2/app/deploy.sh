#!/bin/sh

set -eux

echo Set your registry url in env: REGISTRY_URL

if [ ! "${REGISTRY_URL:-}" ]; then
    export REGISTRY_URL=$(minikube ip):5000
fi

envsubst < chord.yaml | kubectl apply -f-
