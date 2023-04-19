#!/bin/sh

set -ux

# metrics-server
# https://github.com/kubernetes-sigs/metrics-server
# expose CPU & memory metrics on K8s metrics API
# kubectl apply -f https://github.com/kubernetes-sigs/metrics-server/releases/latest/download/components.yaml

# Prometheus
# https://github.com/prometheus-community/helm-charts/tree/main/charts/kube-prometheus-stack
# helm chart, recommended install method by Amazaon EKS doc
# include node_exporter (https://github.com/prometheus/node_exporter) that expose additional metrics
kubectl create namespace monitoring
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
helm upgrade -i prometheus prometheus-community/prometheus \
    --namespace monitoring \
    --set alertmanager.persistentVolume.storageClass=gp2,server.persistentVolume.storageClass=gp2
kubectl get pods -n monitoring -l "app=prometheus,component=server" -o jsonpath="{.items[0].metadata.name}"

# Prometheus Adapter
# https://github.com/kubernetes-sigs/prometheus-adapter/
# adapt Prometheus metrics to K8s custom metrics API
kubectl create namespace monitoring
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
helm upgrade -i prometheus-adapter prometheus-community/prometheus-adapter \
    --namespace monitoring \
    -f prometheus-adapter-config.yaml

# Horizontal Pod Autoscaler
kubectl apply -f chord-hpa.yaml

# AWS Cluster Autoscaler
helm repo add autoscaler https://kubernetes.github.io/autoscaler
helm install cluster-autoscaler autoscaler/cluster-autoscaler \
    --namespace kube-system \
    --set autoDiscovery.clusterName=chord
