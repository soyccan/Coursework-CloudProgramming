#!/bin/sh

set -eux

echo Set your registry url in env: REGISTRY_URL

if [ ! "${REGISTRY_URL:-}" ]; then
    if [ "${AWS:-}" ]; then
        registryId=$(aws ecr describe-registry | yq .registryId)
        region=$(aws configure get default.region)
        REGISTRY_URL=${registryId}.dkr.ecr.${region}.amazonaws.com
    else
        REGISTRY_URL=$(minikube ip):5000
    fi
fi

for name in chord init uploadserver; do
    # minikube image build -t "$name" "$name"
    docker build --tag "${REGISTRY_URL}/${name}:latest" "$name"
    docker push "${REGISTRY_URL}/${name}:latest"
done
