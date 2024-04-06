1. Compile your libtorch and place into the folder
2. meta_parser and precompile were forked from [Piccolo engine](https://github.com/BoomingTech/Piccolo/tree/main)


You can use this command for a minimum libtorch:

``` bash
cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DUSE_CUDA=OFF -DUSE_MPS=OFF -DUSE_MKLDNN=OFF -DUSE_QNNPACK=OFF -DUSE_PYTORCH_QNNPACK=OFF -DBUILD_TEST=OFF -DUSE_NNPACK=OFF -DBUILD_CUSTOM_PROTOBUF=OFF -DBUILD_PYTHON=OFF -DCMAKE_BUILD_TYPE:STRING=Release -DPYTHON_EXECUTABLE:PATH=`which python3` -DCMAKE_INSTALL_PREFIX:PATH=../pytorch-install ../pytorch
cmake --build . --target install

cmake -DMCUT_BUILD_AS_SHARED_LIB:BOOL=OFF -DMCUT_BUILD_TESTS:BOOL=OFF -DMCUT_BUILD_TUTORIALS:BOOL=OFF -DMCUT_BUILD_DOCUMENTATION:BOOL=OFF ..
```