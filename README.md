# Flavortown build system
> build system made in c++ for c++ configured in c++

The repository is an example of how it can be used, the main file is 'build.hh', a header library with building blocks allowing you to make a quick and efficient build recipes, which allow you to NOT learn another language just to compile your projects, since you already have a Turing complete language at your fingertips why would you need to learn another one just to run your project?

To checkout the project run:
```bash
clang++ -o build build.cpp
```
You can choose any c++ of your choice, the goal of this library is to make it as simple as possible, so you don't have to remember any flags, except the output file.

This library gives you few helpful tools in order to craft your build recipes and run them, the main thing being 'CompilerRecipe'. Checkout build.cpp for examples and my project: [curve_editor](https://github.com/Cathyprime/curve_editor) that uses this build system for compilation
