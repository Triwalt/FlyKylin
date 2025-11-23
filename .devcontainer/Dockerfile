FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

# Install CMake, Ninja, and other build tools
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    cmake \
    ninja-build \
    pkg-config \
    zip \
    unzip \
    tar \
    curl \
    git \
    build-essential \
    libgl1-mesa-dev \
    libxkbcommon-x11-0 \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# Install vcpkg
ENV VCPKG_ROOT=/usr/local/vcpkg
ENV PATH=$VCPKG_ROOT:$PATH
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
    && $VCPKG_ROOT/bootstrap-vcpkg.sh

# Pre-install dependencies (Optional: speeds up first container start)
# COPY vcpkg.json /tmp/vcpkg.json
# RUN vcpkg install --triplet x64-linux --x-install-root=$VCPKG_ROOT/installed

# Set up Qt environment variables (if using system Qt or vcpkg Qt)
# ENV CMAKE_PREFIX_PATH=...
