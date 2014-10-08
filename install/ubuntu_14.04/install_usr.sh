#!/bin/bash


echo "Please become root for installation of packages"

if ! dpkg -l | grep gdal-bin >/dev/null; then
echo "Installing GDAL library"
if ! sudo apt-get install -y gdal-bin ; then echo "Installing GDAL library failed"; exit 1; fi
fi
echo "GDAL library installed"

if ! dpkg -l | grep qt4-default >/dev/null; then
echo "Installing QT4 library"
if ! sudo apt-get install -y qt4-default ; then echo "Installing QT4 library failed"; exit 1; fi
fi
echo "QT4 library installed"

if ! dpkg -l | grep libboost-dev >/dev/null; then
echo "Installing Boost library"
if ! sudo apt-get install -y libboost-dev ; then echo "Installing Boost library failed"; exit 1; fi
fi
echo "Boost library installed"

if ! dpkg -l | grep libmysqlcppconn7 >/dev/null; then
echo "Installing MySQL Connector library"
if ! sudo apt-get install -y libmysqlcppconn7 ; then echo "Installing MySQL Connector library failed"; exit 1; fi
fi
echo "MySQL Connector library installed"


if ! dpkg -l | grep liblog4cpp5 >/dev/null; then
echo "Installing Log4Cpp library"
if ! sudo apt-get install -y liblog4cpp5 ; then echo "Installing Log4Cpp  library failed"; exit 1; fi
fi
echo "Log4Cpp  library installed"

if ! dpkg -l | grep libtinyxml2-0.0.0 >/dev/null; then
echo "Installing Tinyxml2 library"
if ! sudo apt-get install -y libtinyxml2-0.0.0 ; then echo "Installing Tinyxml2  library failed"; exit 1; fi
fi
echo "Tinyxml2  library installed"

echo "All libaries installed"
exit
