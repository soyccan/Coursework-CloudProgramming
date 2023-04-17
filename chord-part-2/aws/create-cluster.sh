#!/bin/sh

set -ux

export AWS_ACCOUNT=$(aws sts get-caller-identity --query Account --output text)

aws ec2 create-key-pair --key-name chord --key-type ed25519

aws ecr create-repository --repository-name init
aws ecr create-repository --repository-name chord
aws ecr create-repository --repository-name uploadserver
aws ecr create-repository --repository-name client
aws ecr get-login-password | docker login --username AWS --password-stdin \
    ${AWS_ACCOUNT}.dkr.ecr.us-east-1.amazonaws.com

envsubst < chord-eks.yaml | eksctl create cluster -f-
aws eks update-kubeconfig --name chord
