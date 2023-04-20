#!/bin/sh

kubectl run dnsutil --image registry.k8s.io/e2e-test-images/jessie-dnsutils:1.3 --rm -it -- nslookup "$@"
