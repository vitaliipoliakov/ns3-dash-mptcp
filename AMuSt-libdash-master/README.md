# AMuSt-libdash

This is an extension of [bitmovin/libdash](https://github.com/bitmovin/libdash) for the [Adaptive Multimedia Streaming Simulation (AMuSt) Framework](https://github.com/ChristianKreuzberger/AMuSt-Simulator). The extension for AMuSt can be found in the sub-directory **libdash/libdash/source/amust/**.

# libdash

libdash is the **official reference software of the ISO/IEC MPEG-DASH standard** and is an open-source library that provides an object orient (OO) interface to the MPEG-DASH standard, developed by [bitmovin](http://www.bitmovin.com).


## How to use

Please refer to the information provided at the [Adaptive Multimedia Streaming Simulation (AMuSt) Framework](https://github.com/ChristianKreuzberger/AMuSt-Simulator) github repository for more details. If you are just interested in using the libdash library, please consult the [bitmovin/libdash](https://github.com/bitmovin/libdash) github repository.

In addition we recommend looking at the [tutorial](tutorial.md) provided in here, which will tell the basics of adaptation logics and multimedia player configuration.

### Pre-Requesits (Todo: cleanup)

    sudo apt-get install build-essential gccxml
    sudo apt-get install git-core build-essential cmake libxml2-dev libcurl4-openssl-dev
    sudo apt-get install cmake libxml2-dev libcurl4-openssl-dev
    sudo apt-get install libxml2-dev libxslt-dev python-dev lib32z1-dev

### Build

    cd AMuSt-libdash/libdash
    mkdir build
    cd build
    cmake ../
    make

## libdash-License

libdash is open source available and licensed under LGPL:

“This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA“

As libdash is licensed under LGPL, changes to the library have to be published again to the open-source project, which we are doing with this github repository.



## Acknowledgements
This work (the extension of libdash found in **libdash/libdash/source/amust/**) was partially funded by the Austrian Science Fund (FWF) under the CHIST-ERA project [CONCERT](http://www.concert-project.org/) 
(A Context-Adaptive Content Ecosystem Under Uncertainty), project number I1402.

Please also consider the acknowledgements for *libdash* in the [bitmovin/libdash](https://github.com/bitmovin/libdash) repository.

