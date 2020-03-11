
## Previewer app

## Headless renderer standalone demo

## Python bindings
As part of the demos, there is a simple python module using [pybind11](https://github.com/pybind/pybind11) exposing some basic functionality of the headless renderer.
```bash
git submodule update --init

cd demos
mkdir build && cd build
cmake -DPYBIND11_PYTHON_VERSION=3.5 -DHEPHAESTUS_PYTHON_BINDINGS=1 .. 
```

The PYBIND11_PYTHON_VERSION is optional and can be used to explicitly specify the python version to use. Otherwise pybind11 will pick the python version setup in the system.
