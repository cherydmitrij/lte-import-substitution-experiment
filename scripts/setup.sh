#!/usr/bin/env bash
set -e
sudo apt update && sudo apt install -y \
  g++ python3 python3-dev pkg-config cmake ninja-build git ccache \
  sqlite3 libsqlite3-dev libxml2-dev libboost-all-dev libgsl-dev \
  libgtk-3-dev wget bzip2

cd ~
mkdir -p workspace && cd workspace
wget https://www.nsnam.org/release/ns-allinone-3.41.tar.bz2
tar xjf ns-allinone-3.41.tar.bz2
cd ns-allinone-3.41/ns-3.41

cd src
git clone https://gitlab.com/cttc-lena/nr.git
cd nr && git checkout 5g-lena-v3.0.y && cd ../..

./ns3 configure --enable-examples --enable-tests
./ns3 build
