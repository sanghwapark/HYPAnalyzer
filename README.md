# HYPAnalyzer
Analysis software for Hypernuclear experiment at Hall C

# Building (with cmake)
Dependencies: ROOT version 6 or higher, hcana (https://github.com/JeffersonLab/hcana)
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$path/to/install ..
make -jN install
```

Set your local environment:
```
export PATH=$path/to/install/bin:$PATH
export LD_LIBRARY_PATH=$path/to/install/lib:$LD_LIBRARY_PATH
```

# Replay
An example replay script is in test_bench direcotry. One will need to update the map and parameter files as needed. 
```
hcana
.x replay.C(runNumber, nEvent)
```
