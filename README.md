<img src="https://dalihub.github.io/images/DaliLogo320x200.png">

# Table of Contents

   * [Build Instructions](#build-instructions)
      * [Building for Ubuntu desktop](#1-building-for-ubuntu-desktop)
         * [Minimum Requirements](#minimum-requirements)
         * [Building the Repository](#building-the-repository)
         * [DEBUG Builds](#debug-builds)
   * [Running the tests](#running-the-tests)
   * [Creating a visual test](#creating-a-visual-test)

# Build Instructions

## Building for Ubuntu desktop

### Requirements

 - Ubuntu 14.04 or later
 - Environment created using dali_env script in dali-core repository
 - GCC version 6

DALi requires a compiler supporting C++11 features.
Ubuntu 16.04 is the first version to offer this by default (GCC v5.4.0).

GCC version 6 is recommended since it has fixes for issues in version 5
e.g. it avoids spurious 'defined but not used' warnings in header files.

### Building the Repository

To build the repository:

         $ ./build.sh

### DEBUG Builds

Enter the 'build/tizen' folder

         $ cd build/tizen

Specify a debug build when building for desktop by passing the following parameter to cmake:

         $ cmake -DCMAKE_INSTALL_PREFIX=$DESKTOP_PREFIX -DCMAKE_BUILD_TYPE=Debug .

Before running make install as normal:

         $ make install -j8

# Running the tests

To run all the tests:

         $ ./execute.sh
         
# Creating a visual test

 - Make a directory in the "visual-tests" directory. Only one visual test will be created per directory.
 - The executable installed will have a ".test" appended to it, e.g. a "my-first-visual-test" directory produces "my-first-visual-test.test".
 - Add all source files for the required visual test in this directory.
 - No changes are required to the make system as long as the above is followed, your visual test will be automatically built & installed.
