#!/usr/bin/env bash

cd "$(dirname "$0")"

sudo apt-get update
sudo apt-get install -y software-properties-common
sudo add-apt-repository -y ppa:deadsnakes/ppa

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y python3.12 python3.12-dev libc6:i386 gcc-multilib g++-multilib git libssl-dev libffi-dev build-essential curl

if [ "$(python3.12 -V | grep -vE 3\.12)" ]; then
    echo "Failed to install Python 3.12! Pls fix, thx melon."
    exit 1
fi

curl -sSL https://bootstrap.pypa.io/get-pip.py | python3.12 -
python3.12 -m pip install -r requirements.txt
python3.12 -m pip install -e .
