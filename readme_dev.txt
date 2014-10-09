Developers guide

Installation
------------
If you want to view the source code and build it for your system without contributing to it use the following console command to clone the repository contents:
'git clone git://git.code.sf.net/p/atsdb/code atsdb'. This will create a sub-folder 'atsdb' with the repository contents.

If you want to contribute to the project please create a sourcefore account and send the information and your intentions in a mail to me (helmut.puhr@tugraz.at), then I will add you to the project. After that you can use your own custom git command (http://sourceforge.net/p/atsdb/code/ci/master/tree/).

The following tools are required to build the project:
* gcc/g++ compiler (https://gcc.gnu.org/)
* git (http://git-scm.com/)
* cmake (for building, http://www.cmake.org/)
* doxygen (for gernerating documentation, http://www.stack.nl/~dimitri/doxygen/)
(for the user guide also latex is required, http://www.latex-project.org/)

The following libraries are required to build the project, in their developer versions:
* gdal (http://www.gdal.org/)
* qt4 (http://qt-project.org/)
* boost (http://www.boost.org/)
* mysql connector (http://dev.mysql.com/downloads/connector/)
* log4cpp (http://log4cpp.sourceforge.net/)
* tinyxml2 (http://www.grinninglizard.com/tinyxml2/)

For a number of system, installation scripts have been written that, if executed as root, install the libraries. They can be found in the 'installation/' folder with the appropriate usb-folder.

* centos_6.4/install_dev.sh
* centos_6.5/install_dev.sh
* ubuntu_14.04/install_dev.sh

If you use one of these system, please execute the appropriate shell-script as root. If you are on a different platform, please use the package-manager of your choice to install the listed libraries.

Building
--------

After installation, the project can be built. The first step is to create the build files using the cmake tool. Please familiarize yourself with CMake if required.

In the main installation folder 'atsdb', execute the console command 'cmake .'. This generates the actual make files four your platform. If no errors occured, execute 'make' and 'make install'. This generates the library and copies the header files. After these steps, the build library can be found under 'dist/lib' and the headers under 'dist/include'. Please use them locally or copy them to the system paths with wanted.


Author
------
Helmut Puhr
helmut.puhr@tugraz.at

