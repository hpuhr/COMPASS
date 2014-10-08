#!/bin/bash


echo "Please become root for installation of packages"

if ! rpm -qa | grep epel >/dev/null; then
echo "Installing EPEL (Extra Packages for Enterprise Linux) repository"
if ! rpm -Uvh http://download.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm ; then echo "Installing EPEL repository failed"; exit 1; fi
fi
echo "EPEL repository installed"

if ! rpm -qa | grep gdal >/dev/null; then
echo "Installing GDAL library"
if ! sudo yum install -y gdal ; then echo "Installing GDAL library failed"; exit 1; fi
fi
echo "GDAL library installed"

if ! rpm -qa | grep qt >/dev/null; then
echo "Installing QT4 library"
if ! sudo yum install -y qt4 ; then echo "Installing QT4 library failed"; exit 1; fi
fi
echo "QT4 Dev library installed"

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
echo "Log4Cpp library installed"

if ! rpm -qa | grep tinyxml2l >/dev/null; then
echo "Installing Tinyxml2 library"
if ! sudo yum install -y tinyxml2 ; then echo "Installing Tinyxml2 library failed"; exit 1; fi
fi
echo "Tinyxml2 library installed"

echo "All libaries installed"
exit
