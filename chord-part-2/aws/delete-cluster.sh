#!/bin/sh

set -ux

export AWS_ACCOUNT=$(aws sts get-caller-identity --query Account --output text)
export AWS_REGION=$(aws configure get default.region)

aws ec2 delete-key-pair --key-name chord

aws ecr delete-repository --repository-name chord/core/init
aws ecr delete-repository --repository-name chord/core/chord
aws ecr delete-repository --repository-name chord/core/uploadserver
aws ecr delete-repository --repository-name chord/debug/client
aws ecr delete-repository --repository-name chord/server/fileserver

eksctl delete cluster -f chord-eks.yaml --disable-nodegroup-eviction
