#/bin/bash

# Install Xapian core if not already present
if [[ ! -d xapian-core-1.2.13 ]]
then
    tar -xf xapian-core-1.2.13.tar.gz
fi

cd xapian-core-1.2.13
mkdir install
./configure --prefix=$PWD/install
make -j16
make install
cd -

# Build search engine
make
