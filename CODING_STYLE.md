# Coding Style
I just want ot make sure we all follow the same rules when contributing 
code back into this tree.

## Version of C++
This project is based on the C++17 standard. If compiles as-is on most platforms 
using either clang or g++. Do not use C++21 features for now. I would love to do some
of the new stuff but let's wait for these features to be available on
all compilers first. `coroutine` has to wait.

## Naming
Naming of pretty much anything uses Pascal naming. I know... You might not be a big fan or have 
fallen to the JS gods and use camelNaming. Well, let's all make an effort to keep
this coherent. Member variable naming adds a `_` at the end of the vars. Try to
keep this standard going. Sometimes you must override a base class function and then of course
you need to follow the base class. Let's be real...

## File Sizes
Do you best to keep your file sizes < 300 lines. It just makes the code more readable
and shortens compile times. 

## This is a cmake project
This is a cmake project and you need to adhere to the cmake rules. If you need
to add a package to the CMakeList, you need to ensure that the package is available
on all required platforms and compiles. Remember that this project runs on Linux, OS X, 
and the Raspberry PI.

## Licensed packages
When adding a package, you must also state the licensing for the package. MIT, BSD, Apache licenses
are acceptable. No commercial licenses are allowed. 


