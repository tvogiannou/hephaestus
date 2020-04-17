
# Demos


## Build instructions
The demos come with a CMake script that should work simply with the default options. 

```bash
cd demos
mkdir build
cd build

# build with default options
cmake ..
cmake --build .
```

## Previewer app

The previewer is a simple window application based on [GLFW](https://www.glfw.org/). Please follow [the instructions](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps) in the official GLFW site on how to install any dependencies.
To built the app set `HEPHAESTUS_PREVIEWER_APP` in the cmake command (it is ON by default)
```bash
cmake -DHEPHAESTUS_PREVIEWER_APP=1 ..
```

## Headless renderer standalone demo
The standalone demo is a single source file console executable that demonstrates how to use the headless renderer to render a single frame and store as an image file.
To built the executable set `HEPHAESTUS_HEADLESS_EXAMPLE` in the cmake command (it is ON by default)
```bash
cmake -HEPHAESTUS_HEADLESS_EXAMPLE=1 ..
```

## Python bindings
The python bindings is a simple python module using [pybind11](https://github.com/pybind/pybind11) exposing some basic functionality of the headless renderer in python.
```bash
# pybind is integrated as a git submodule in the repo so it needs to be updated manually
git submodule update --init

# HEPHAESTUS_PYTHON_BINDINGS is OFF by default
cmake -DPYBIND11_PYTHON_VERSION=3.5 -DHEPHAESTUS_PYTHON_BINDINGS=1 .. 
```

The PYBIND11_PYTHON_VERSION is optional and can be used to explicitly specify the python version to use. Otherwise pybind11 will pick the python version setup in the system.
