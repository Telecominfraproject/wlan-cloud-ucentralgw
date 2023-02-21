# Coding Style
We just want ot make sure we all follow the same rules when contributing 
code back into this tree.

## Version of C++
This project is based on the C++17 standard. If compiles as-is on most platforms 
using either clang or g++. Do not use C++21 or C++23 features for now. Some core 
libraries for this project do not yet support C++21 or C++23

## Variable Naming
Naming of pretty much anything uses Pascal naming. Longer explicit names using casing. 
Member variable naming adds a `_` at the end of the vars. Try to
keep this standard going. Sometimes you must override a base class function and then of course
you need to follow the base class.

## File Sizes
Do you best to keep your file sizes < 300 lines. It just makes the code more readable
and shortens compile times. 

## This is a cmake project
This is a cmake project, and you need to adhere to the cmake rules. If you need
to add a package to the CMakeList, you need to ensure that the package is available
on all required platforms and compiles. Remember that this project runs on Linux, OS X, 
and the Raspberry PI.

## Licensed packages
When adding a package, you must also state the licensing for the package. MIT, BSD, Apache licenses
are acceptable. No commercial licenses are allowed. 

## clang formatting
Please format your code using the included `.clang-format` file included in the project.

```bash
clang-format -i --style=<project root>/.clang-format myfile.cpp
```
