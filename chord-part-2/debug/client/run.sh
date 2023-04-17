#!/bin/sh

set -ux

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

kubectl run client --image ${REGISTRY_URL}/client --rm -it
