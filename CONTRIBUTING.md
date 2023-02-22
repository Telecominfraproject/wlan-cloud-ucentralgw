# How to Contribute

We'd love to accept your patches and contributions to this project. There are
just a few small guidelines you need to follow.

## Version of C++
This project is based on the C++17 standard and compiles as-is on most platforms 
using either clang or g++. Do not use C++21 or C++23 features for now. Some core 
libraries used in this project do not support C++21 or C++23 yet.

## Variable Naming
Naming of pretty much anything uses Pascal naming. Longer explicit names using casing. 
Member variable naming adds a `_` at the end of the vars. Try to
keep this standard going. Sometimes you must override a base class function and then of course
you need to follow the base class.

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

## Pull Requests
All submissions, including submissions by project members, require review. We
accept GitHub pull requests. Please create a branch with the Jira name for addressing the issue you are fixing or the 
feature you are implementing.
Create a pull-request from the branch into master. 
