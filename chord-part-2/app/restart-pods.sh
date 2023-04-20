#!/bin/sh

set -ux

kubectl rollout restart deployment -n chord chord-leader
kubectl rollout restart deployment -n chord chord
kubectl rollout restart deployment -n chord chord-server
