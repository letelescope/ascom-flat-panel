# Build and install driver

## Pre-requisites

A fully functionnal indi driver dev environment c.f. [official documentation](https://docs.indilib.org/drivers/basics/project-setup.html)

## Build and install

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ../
make
sudo make install
```

or 

```
./make-install.sh
```
