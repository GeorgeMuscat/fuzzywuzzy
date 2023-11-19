#!/usr/bin/env bash

cd "$(dirname "$0")"

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y python3 python3-dev python3-venv libc6:i386 gcc-multilib g++-multilib git libssl-dev libffi-dev build-essential curl

curl -sSL https://bootstrap.pypa.io/get-pip.py | python3 -
python3 -m pip install -r requirements.txt
python3 -m pip install -e .

# Why are you looking here?