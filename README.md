# HKSAnalyzer
This is data analysis software for Hypernuclear experiment at Hall C.

## Building the code
Prerequisites: ROOT, hcana (https://github.com/JeffersonLab/hcana)


Compile with cmake:
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
make -jN
```
Setting up environment variables:
```
setenv LD_LIBRARY_PATH /path/to/installdir:$LD_LIBRARY_PATH (csh, tcsh)
export LD_LIBRARY_PATH=/path/to/installdir:$LD_LIBRARY_PATH (bash, zsh)
```
