#!/bin/sh
# TODO: Automate the process using helm & kustomize

set -ux

echo Set your registry url in env: REGISTRY_URL

if [ ! "${REGISTRY_URL:-}" ]; then
    if [ "${AWS:-}" ]; then
        registryId=$(aws ecr describe-registry | yq .registryId)
        region=$(aws configure get default.region)
        export REGISTRY_URL=${registryId}.dkr.ecr.${region}.amazonaws.com
    else
        export REGISTRY_URL=$(minikube ip):5000
    fi
fi

mkdir -pv .gen

# generate normal chord
export NAME=chord
export REPLICAS=2
export HOSTNAME_SUBDOMAIN=
export LEADER_ENV=
envsubst < chord.yaml > .gen/chord.yaml

# generate chord-leader
export NAME=chord-leader
export REPLICAS=1
export HOSTNAME_SUBDOMAIN='hostname: leader
      subdomain: chord'
export LEADER_ENV='- { name: LEADER, value: "true" }'
envsubst < chord.yaml > .gen/chord-leader.yaml

# AWS Load Balancer Controller
helm repo add eks https://aws.github.io/eks-charts
helm install aws-load-balancer-controller eks/aws-load-balancer-controller \
  -n kube-system \
  --set clusterName=chord \
  --set serviceAccount.create=false \
  --set serviceAccount.name=aws-load-balancer-controller

# apply chord app & service
kubectl apply -f .gen/chord.yaml
kubectl apply -f .gen/chord-leader.yaml
kubectl apply -f chord-svc.yaml
