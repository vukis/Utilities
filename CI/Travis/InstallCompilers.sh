#!/bin/sh

# Install gcc-7
# sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
echo Install GCC-7...
sudo apt-get update -qq
sudo apt-get install -qq g++-7
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 90
if [ $CXX == "g++" ]; then 
  export CXX="g++-7"; 
fi
  
# Install clang
# sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main"
# sudo apt-get install --allow-unauthenticated -qq clang++-4.0
# export CXX="clang++-4.0"
