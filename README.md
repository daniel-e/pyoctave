# pyoctave

pyoctave is an Octave extension to execute Python scripts.

[Octave](https://www.gnu.org/software/octave/) is a high-level interpreted language, primarily intended for numerical computations. It provides capabilities for the numerical solution of linear and nonlinear problems, and for performing other numerical experiments. It also provides extensive graphics capabilities for data visualization and manipulation.

When you want to use code written in other languages Octave provides an external code interface which can be used for extensions. For extensions the languages C++, C and Fortran are supported and Octave can call external C/C++ code through its native oct-file interface.

Currently there exists no bridge between Octave and Python and the goal of the pyoctave extension is to close this gap. With this extension it is possible to use the full power of Python for Octave like text processing, parsing and so on.

- [Steps](#steps)
  - [Compile the sources](#compile-the-sources)
  - [Octave](#octave)
  - [Run Python scripts](#run-python-scripts)
- [Current limitations](#current-limitation)


## Steps

Before you can install and use pyoctave make sure that all required dependencies are fulfilled to compile the extension. Typically, this should be the case for a standard Linux installation. If some of the following steps fails you should check the dependency.

For Ubuntu 14.10 the required packages can be installed with the following commands:

```
sudo apt-get update
sudo apt-get install git build-essentials python-all-dev liboctave-dev
```

### Compile the sources

First, get the sources and compile the extension.

```
git clone git://github.com/daniel-e/pyoctave.git
cd pyoctave
CXXFLAGS="-std=c++11 -g -O3" make
```

On success a file with the name `runpy.oct` should exist which is called by Octave when you want to execute Python code.

### Octave

Start octave as follows:

```
octave -p .
```

The option -p is used to add the current path to the function search paths of Octave. This is a list of directories in which Octave is looking for extensions. The extension uses this path to search these paths for Python modules.

Alternatively, you can use the Octave command `addpath()` to add a directory to the function search paths.

### Run Python scripts

Now, you can type `help runpy` in Octave to get the help for the `runpy` function.

To run the Python example which is part of the pyoctave distribution type:

```
octave:1> v = runpy("pyexample", "counter", "10")
v =

   0
   1
   2
   3
   4
   5
   6
   7
   8
   9
```

This command executes the function `counter` in the module `pyexample` with the argument 10 which returns a vector with the number from 0 to 10.

Now, you can work with this vector in Octave as usual. For example, you can create a 5 times 2 matrix with the command `reshape`.

```
octave:2> reshape(r, 5, 2)
ans =

   0   5
   1   6
   2   7
   3   8
   4   9
```

## Current limitations

TODO
