#!/bin/sh

set -ux

kubectl rollout restart deployment chord-leader
kubectl rollout restart deployment chord
