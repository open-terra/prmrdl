#!/bin/sh
git clone https://github.com/open-terra/terra.git
mkdir terra/build
cd terra/build
cmake -DBUILD_TESTING=OFF ..
make && sudo make install
