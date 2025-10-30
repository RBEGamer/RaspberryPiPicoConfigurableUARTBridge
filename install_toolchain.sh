#!/bin/bash
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi
apt update
apt upgrade
apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi python3 python3-pip python3-venv



su vscode
git submodule sync --recursive
git submodule update --init --recursive

cmake -B build-pico2 -DPICO_BOARD=pico2
make -C build-pico2