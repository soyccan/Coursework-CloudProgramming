#!/bin/sh

set -eux

echo Set your registry url in env: REGISTRY_URL

if [ ! "${REGISTRY_URL:-}" ]; then
    REGISTRY_URL=$(minikube ip):5000
fi

for name in chord init uploadserver; do
    # minikube image build -t "$name" "$name"
    docker build --tag "${REGISTRY_URL}/${name}:latest" "$name"
    docker push "${REGISTRY_URL}/${name}:latest"
done
