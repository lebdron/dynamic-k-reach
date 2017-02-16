# Dynamic K-Reachability
This is an extension of k-hop reachability index originally described in referenced publication. The extension includes edge and vertex insertion and removal, without reconstructing the index from scratch.

Basic dynamic k-reach index is currently implemented in C++. Scalable k-reach is work in progress, now with prototype implemented in Python (which currently supports edge addition only).

## How to run
### Basic dynamic k-reach
Boost headers are currently required to run. Ensure that `BOOST_ROOT` is properly set in CMakeLists.txt.

Also ensure that `filename` string will be pointing to a valid edge list after compilation.

```
mkdir build
cd build
cmake ..
dynamic_k_reach
```

### Scalable dynamic k-reach
All information now can be found in Scalable K-Reach.ipynb.

## Reference
James Cheng, Zechao Shang, Hong Cheng, Haixun Wang, and Jeffrey Xu Yu. [**Efficient processing of k-hop reachability queries**](http://link.springer.com/article/10.1007/s00778-013-0346-6) *The VLDB Journal 2014*
