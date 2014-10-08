#!/bin/bash

echo "Please become root for installation of packages"

if ! rpm -qa | grep "gcc-c++" >/dev/null; then
echo "Installing G++"
if ! sudo yum install -y gcc-c++ ; then echo "Installing G++"; exit 1; fi
fi
echo "G++ installed"

if ! rpm -qa | grep git >/dev/null; then
echo "Installing GIT"
if ! sudo yum install -y git ; then echo "Installing GIT"; exit 1; fi
fi
echo "GIT installed"

if ! rpm -qa | grep cmake >/dev/null; then
echo "Installing CMake"
if ! sudo yum install -y cmake ; then echo "Installing CMake"; exit 1; fi
fi
echo "CMake installed"

if ! rpm -qa | grep doxygen >/dev/null; then
echo "Installing Doxygen"
if ! sudo yum install -y doxygen ; then echo "Installing Doxygen"; exit 1; fi
fi
echo "Doxygen installed"

if ! rpm -qa | grep epel >/dev/null; then
echo "Installing EPEL (Extra Packages for Enterprise Linux) repository"
if ! sudo yum install -y epel-release ; then echo "Installing EPEL repository failed"; exit 1; fi
fi
echo "EPEL repository installed"

if ! rpm -qa | grep elgis >/dev/null; then
echo "Installing ELGIS"
if ! sudo rpm -Uvh http://elgis.argeo.org/repos/6/elgis-release-6-6_0.noarch.rpm ; then echo "Installing ELGIS repository failed"; exit 1; fi
fi
echo "ELGIS repository installed"

if ! rpm -qa | grep qt-devel >/dev/null; then
echo "Installing QT4 Dev library"
if ! sudo yum install -y qt4-devel ; then echo "Installing QT4 Dev library failed"; exit 1; fi
fi
echo "QT4 Dev library installed"

if ! rpm -qa | grep boost-devel >/dev/null; then
echo "Installing Boost library"
if ! sudo yum install -y boost-devel ; then echo "Installing Boost library failed"; exit 1; fi
fi
echo "Boost library installed"

if ! rpm -qa | grep mysql-devel >/dev/null; then
echo "Installing MySQL Dev library"
if ! sudo yum install -y mysql-devel ; then echo "Installing MySQL Dev library failed"; exit 1; fi
fi
echo "MySQl Dev library installed"

if ! rpm -qa | grep mysql-connector >/dev/null; then
echo "Installing MySQL Connector library"
if ! rpm -Uvh http://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-1.1.4-linux-glibc2.5-x86-64bit.rpm ; then echo "Installing MySQL Connector library failed"; exit 1; fi
fi
echo "MySQL Connector library installed"


if ! rpm -qa | grep log4cpp-devel >/dev/null; then
echo "Installing Log4Cpp Dev library"
if ! sudo yum install -y log4cpp-devel ; then echo "Installing Log4Cpp Dev library failed"; exit 1; fi
fi
echo "Log4Cpp Dev library installed"

if ! rpm -qa | grep tinyxml2-devel >/dev/null; then
echo "Installing Tinyxml2 Dev library"
if ! sudo yum install -y tinyxml2-devel ; then echo "Installing Tinyxml2 Dev library failed"; exit 1; fi
fi
echo "Tinyxml2 Dev library installed"

if ! rpm -qa | grep armadillo-3 >/dev/null; then
echo "Installing Armadillo3 library"
if ! rpm -Uvh ftp://ftp.is.co.za/mirror/fedora.redhat.com/epel/6/x86_64/armadillo-3.800.2-1.el6.x86_64.rpm ; then echo "Installing Armadillo3 library failed"; exit 1; fi
fi
echo "Armadillo3 library installed"

if ! rpm -qa | grep armadillo-devel-3 >/dev/null; then
echo "Installing Armadillo3 Dev library"
if ! rpm -Uvh ftp://ftp.is.co.za/mirror/fedora.redhat.com/epel/6/x86_64/armadillo-devel-3.800.2-1.el6.x86_64.rpm ; then echo "Installing Armadillo3 Dev library failed"; exit 1; fi
fi
echo "Armadillo3 Dev library installed"

if ! rpm -qa | grep gdal-devel >/dev/null; then
echo "Installing GDAL Dev library"
if ! sudo yum install -y gdal-devel --skip-broken; then echo "Installing GDAL Dev library failed"; exit 1; fi
fi
echo "GDAL Dev library installed"

echo "All libaries installed"
exit
