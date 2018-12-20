# Secure-svm

基于安全多方计算的SVM算法


## enviroment

1. m4

``` shell
sudo apt-get install m4
```

2. gmp

``` shell
sudo apt-get install lzip
lzip -d gmp-6.1.2.tar.lz
tar -xvf gmp-6.1.2.tar
cd gmp-6.1.2
./configure
make
make check
sudo make install
```

3. ntl(10.3)

``` shell
gunzip ntl-xxx.tar.gz
tar xf ntl-xxx.tar
cd ntl-xxx/src
./configure
make
make check
sudo make install
```

**Before compile**
Copy ZZ.cpp into NTL_PACKAGE_DIR/src/

Copy ZZ.h into NTL_PACKAGE_DIR/include/NTL/

setting `NTL_THREAD_BOOST=on` during the configuration of NTL to enable thread-boosting.
(in `DoConfig`)