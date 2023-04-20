#!/bin/sh

set -ux

export AWS_ACCOUNT=$(aws sts get-caller-identity --query Account --output text)
export AWS_REGION=$(aws configure get default.region)

aws ec2 create-key-pair --key-name chord --key-type ed25519 > .chord-key.yaml

aws ecr create-repository --repository-name chord/core/init
aws ecr create-repository --repository-name chord/core/chord
aws ecr create-repository --repository-name chord/core/uploadserver
aws ecr create-repository --repository-name chord/debug/client
aws ecr create-repository --repository-name chord/server/fileserver
aws ecr get-login-password | docker login --username AWS --password-stdin \
    ${AWS_ACCOUNT}.dkr.${AWS_REGION}.amazonaws.com

eksctl create cluster -f chord-eks.yaml
aws eks update-kubeconfig --name chord
