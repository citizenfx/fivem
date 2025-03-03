#!/bin/bash
set -e

ROOT_REPO=$(pwd)

IMAGE_NAME="fivem-builder-linux-alpine"

echo "Building Docker image..."
docker build -t $IMAGE_NAME -f "$ROOT_REPO/code/tools/ci/docker-builder/Dockerfile" "$ROOT_REPO"

echo "Running build inside Docker container..."
docker run --rm \
  -v "$ROOT_REPO:/workspace" \
  -w /workspace \
  $IMAGE_NAME \
  sh code/tools/ci/build_server_docker_alpine_public.sh

echo "Build completed successfully!"
