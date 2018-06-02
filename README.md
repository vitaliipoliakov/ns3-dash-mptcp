# ns-3.19 with MPTCP and MPEG-DASH. 

That's a ready-to-use distro with built-in support for MPTCP and MPEG-DASH. All the due credit goes to the work of guys from University of Sussex (for MPTCP implementation [1]) and Alpen-Adria-Universität Klagenfurt (for DASH [2]). I just slightly extended those two models such that they build and work together without issues. 

## How to build:

First go into **AMuSt-libdash-master/**, read its README and build the libdash for ns-3.

Next, make sure you have all ns-3 dependencies **and** ZLib dev packages.

Then you can go to **ns3/** and configure it:

    ./waf configure --with-dash=../AMuSt-libdash-master/libdash/ --enable-tests --disable-python

...and build it as usual.

## How to use

Feel free to check scripts in **ns3/scratch/** for examples on how to use.


## Refs

[1] https://github.com/mkheirkhah/mptcp

[2] https://github.com/ChristianKreuzberger/AMuSt-ns3
