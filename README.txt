HOW TO BUILD:

newproject is the project directory

For x86_64
1. cd newproject/src
2. make
3. make install

For bbb:
1. cd newproject/src
2. ARCH=bbb make
3. make install

All the running fils and environments are located at newproject/install directory.

HOW TO RUN

Copy the newproject/install directory to the target.
There is one run.sh in the directory, run it:
./run.sh



