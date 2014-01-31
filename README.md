`readgenesis`
===========

`readgenesis` is a Matlab reader for the [GENESIS simulator](http://genesis-sim.org/) binary data files. It is written in C and used through the Matlab MEX extension.

Note that running Genesis on a PowerPC architecture Macs will create big endian data files that need to be read with `readgenesis_BE` on Intel-based (little endian) PCs.

`readgenesis` is obsolete, but kept here for historical reasons. The preferred way of reading Genesis binary files is by using the endian-aware Matlab function `readgenbin.m`, which is included in the [Pandora toolbox](http://software.incf.org/software/pandora/).

History
-------

* Cengiz Gunay 2012
    Autoconf file

* Cengiz Gunay <cengique AT users.sf.net> 03.13.2004
    Fixed memory leak of not deallocating memory for the raw data buffer.

* Adapted for general use (a routine...)
    from Dieter Jaeger's xicanal lx_genread.c
    Alfonso Delgado-Reyes 07.03.2002

* Adapted for MATLAB 5.x and 6.x under:

    - Linux x86/PPC, September 2002
    - MS Windows, September 2002
    - Mac OS 7-9, September 2002
    - Mac OS X 10.x, September 2002
   
Instructions
------------

Type:

    make

which should create both `readgenesis` and `readgenesis_BE` (bigendian) MEX executables. Place them in the Matlab path. In Matlab, they can be accessed by typing:

    >> readgendbin([])
 
which will give you the usage help.

Pre-compiled binaries
---------------------

Under the `mex/` directory, you can find several pre-compiled binaries. Place them in the Matlab path and use like in the above example.

Manual Compile
--------------

Alternatively, you can compile manually. Any cc compiler should work ([ ] = optional). To compile:
* In Winblows (MS Visual C++ 6.x):

                mex [-v -O] -DWIN32 -output readgenesis readgenesis.c

* On new i64 Macs:

                /Applications/MATLAB_R2011a.app/bin/mex -v -O -I/usr/include -I/usr/lib/gcc/i686-apple-darwin10/4.2.1/include/ -I/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/install-tools/include -I/Applications/MATLAB_R2011a.app/extern/include/ -output readgenesis_mac readgenesis_mac.c 

* Anywhere else:

                mex [-v -O] -output readgenesis readgenesis.c

To get a endian-converting loader, enable big endian with: -D__BIG_ENDIAN__
