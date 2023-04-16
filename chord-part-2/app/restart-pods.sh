#!/bin/sh

set -ux

echo Set your registry url in env: REGISTRY_URL

if [ ! "${REGISTRY_URL:-}" ]; then
    export REGISTRY_URL=$(minikube ip):5000
fi

kubectl delete pod chord-leader --force --grace-period 0
kubectl scale deployment chord --replicas 0 --timeout 0
"$(dirname "$0")"/deploy.sh
