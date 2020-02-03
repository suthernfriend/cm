#!/usr/bin/env bash

REPOSITORY="https://github.com/suthernfriend/cm.git"
BUILD_DIR="/tmp/cm"
source /etc/os-release

if [ "$ID" == "debian" ]; then
  apt-get install -y build-essentail libyaml-cpp-dev libboost-all-dev cmake git || exit 1
  git clone "$REPOSITORY" "$BUILD_DIR" || exit 1
  cd "$BUILD_DIR" || exit 1
  cmake -S . -B "$BUILD_DIR/build" || exit 1
  cd "$BUILD_DIR/build" || exit 1
  make -j 2 || exit 1
  install -Dm700 ./cm /bin/cm
fi

