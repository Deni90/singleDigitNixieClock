#!/bin/bash
git submodule update --init --recursive

cd thirdParty/Arduino/tools
python3 get.py
cd -