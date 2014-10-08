#!/bin/bash


echo "Please become root for installation of packages"

if ! dpkg -l | grep g++ >/dev/null; then
echo "Installing G++"
if ! sudo apt-get install -y g++ ; then echo "Installing G++"; exit 1; fi
fi
echo "G++ installed"

if ! dpkg -l | grep git >/dev/null; then
echo "Installing GIT"
if ! sudo apt-get install -y git ; then echo "Installing GIT"; exit 1; fi
fi
echo "GIT installed"

if ! dpkg -l | grep cmake >/dev/null; then
echo "Installing CMake"
if ! sudo apt-get install -y cmake ; then echo "Installing CMake"; exit 1; fi
fi
echo "CMake installed"

if ! dpkg -l | grep doxygen >/dev/null; then
echo "Installing Doxygen"
if ! sudo apt-get install -y doxygen ; then echo "Installing Doxygen"; exit 1; fi
fi
echo "Doxygen installed"

if ! dpkg -l | grep libgdal-dev >/dev/null; then
echo "Installing GDAL Dev library"
if ! sudo apt-get install -y libgdal-dev ; then echo "Installing GDAL Dev library failed"; exit 1; fi
fi
echo "GDAL Dev library installed"

if ! dpkg -l | grep libqt4-dev >/dev/null; then
echo "Installing QT4 Dev library"
if ! sudo apt-get install -y libqt4-dev ; then echo "Installing QT4 Dev library failed"; exit 1; fi
fi
echo "QT4 Dev library installed"

if ! dpkg -l | grep libboost-dev >/dev/null; then
echo "Installing Boost library"
if ! sudo apt-get install -y libboost-dev ; then echo "Installing Boost library failed"; exit 1; fi
fi
echo "Boost library installed"

if ! dpkg -l | grep libmysqlcppconn-dev >/dev/null; then
echo "Installing MySQL Connector library"
if ! sudo apt-get install -y libmysqlcppconn-dev ; then echo "Installing MySQL Connector library failed"; exit 1; fi
fi
echo "MySQL Connector library installed"


if ! dpkg -l | grep liblog4cpp5-dev >/dev/null; then
echo "Installing Log4Cpp Dev library"
if ! sudo apt-get install -y liblog4cpp5-dev ; then echo "Installing Log4Cpp Dev library failed"; exit 1; fi
fi
echo "Log4Cpp Dev library installed"

if ! dpkg -l | grep libtinyxml2-dev >/dev/null; then
echo "Installing Tinyxml2 Dev library"
if ! sudo apt-get install -y libtinyxml2-dev ; then echo "Installing Tinyxml2 Dev library failed"; exit 1; fi
fi
echo "Tinyxml2 Dev library installed"

echo "All libaries installed"
exit
