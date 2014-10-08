#!/bin/bash

echo "Please become root for installation of packages"

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

if ! rpm -qa | grep qt >/dev/null; then
echo "Installing QT4 library"
if ! sudo yum install -y qt4 ; then echo "Installing QT4 library failed"; exit 1; fi
fi
echo "QT4 library installed"

if ! rpm -qa | grep boost >/dev/null; then
echo "Installing Boost library"
if ! sudo yum install -y boost ; then echo "Installing Boost library failed"; exit 1; fi
fi
echo "Boost library installed"

if ! rpm -qa | grep mysql-connector >/dev/null; then
echo "Installing MySQL Connector library"
if ! rpm -Uvh http://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-1.1.4-linux-glibc2.5-x86-64bit.rpm ; then echo "Installing MySQL Connector library failed"; exit 1; fi
fi
echo "MySQL Connector library installed"


if ! rpm -qa | grep log4cpp >/dev/null; then
echo "Installing Log4Cpp library"
if ! sudo yum install -y log4cpp ; then echo "Installing Log4Cpp library failed"; exit 1; fi
fi
echo "Log4Cpp Dev library installed"

if ! rpm -qa | grep tinyxml2 >/dev/null; then
echo "Installing Tinyxml2 library"
if ! sudo yum install -y tinyxml2 ; then echo "Installing Tinyxml2 library failed"; exit 1; fi
fi
echo "Tinyxml2 library installed"

if ! rpm -qa | grep armadillo-3 >/dev/null; then
echo "Installing Armadillo3 library"
if ! rpm -Uvh ftp://ftp.is.co.za/mirror/fedora.redhat.com/epel/6/x86_64/armadillo-3.800.2-1.el6.x86_64.rpm ; then echo "Installing Armadillo3 library failed"; exit 1; fi
fi
echo "Armadillo3 library installed"

if ! rpm -qa | grep gdal >/dev/null; then
echo "Installing GDAL library"
if ! sudo yum install -y gdal --skip-broken; then echo "Installing GDAL library failed"; exit 1; fi
fi
echo "GDAL library installed"

echo "All libaries installed"
exit
