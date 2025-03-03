#!/bin/bash
set -e

ROOT_REPO=$(pwd)

IMAGE_NAME="fivem-builder-linux-alpine"
IMAGE_TAG="latest"
REGISTRY="ghcr.io"
IMAGE_PATH="${REGISTRY}/${GITHUB_REPOSITORY}/${IMAGE_NAME}:${IMAGE_TAG}"

echo "Pulling Docker image from $IMAGE_PATH..."
docker pull $IMAGE_PATH

echo "Running build inside Docker container..."
docker run --rm \
  -v "$ROOT_REPO:/workspace" \
  -w /workspace \
  $IMAGE_PATH \
  sh code/tools/ci/build_server_docker_alpine_public.sh

echo "Build completed successfully!"
