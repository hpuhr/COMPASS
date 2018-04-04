#!/bin/bash

add_path() {
   if [ ! -f $1 ]; then
      # config file doesn't exist, create it
      echo "export PATH=\${PATH:+\${PATH}:}$2" > $1
      return
   fi

   # the PATH statement can be very messy in the user's config file
   # don't make any fancy processing and just add it to the end of the file
   grep -q "export PATH=\${PATH:+\${PATH}:}$2" $1
   if [ $? -ne 0 ]; then
      echo "export PATH=\${PATH:+\${PATH}:}$2" >> $1
   fi
}

#
# install ATSDB util scripts
#

INSTALL_DIR=$HOME/bin
ATSDB_SCRIPTS="csv2atsdb ads-b.xchg/parseADSBxchg ads-b.xchg/prepADSBxchg"
pushd .

if [ ! -d $INSTALL_DIR ]; then
   mkdir $INSTALL_DIR
fi
mkdir -p $INSTALL_DIR

#
# obtain and install 'termsql' and 'mysql2sqlite'
#

which wget > /dev/null
if [ $? -ne 0 ]; then
   echo "'wget' cannot be found, please check that it is installed in your system,"
   echo "or install it and run this script again"
   exit 1
fi

cd /tmp

wget https://github.com/dumblob/mysql2sqlite/archive/master.zip
unzip master.zip
mv mysql2sqlite-master/mysql2sqlite $INSTALL_DIR
rm -rf master.zip mysql2sqlite-master

wget https://github.com/tobimensch/termsql/archive/master.zip
unzip master.zip
mv termsql-master/termsql $INSTALL_DIR
rm -rf master.zip termsql-master

#
# check if python dependencies are present and ok
#
echo "checking 'termsql' dependencies"
cat > /tmp/.check-python-mod << CHECK_PYMOD
#!/usr/bin/env python
def module_exists(module_name, module_required_version):
   try:
      mod = __import__(module_name)
   except ImportError:
      print ("module: '" + module_name + "' does not exist. Please consider installing it with a version greater than " + module_required_version)
   else:
      print ("module: '" + module_name + "' exists")
      if mod.__version__ <= module_required_version:
         print ("\tversion found is " + mod.__version__ + " please consider upgrading to version " + module_required_version)
      else:
         print ("\tand the version " + mod.__version__ + " is ok")

for module in [ 'sqlparse:0.1.14' ]:
   mod,ver = module.split(":")
   module_exists(mod, ver)
CHECK_PYMOD
chmod +x /tmp/.check-python-mod
/tmp/.check-python-mod
rm /tmp/.check-python-mod

#
# copy remaining scripts to the installation directory
#
popd
for file in $ATSDB_SCRIPTS
do
   chmod +x $file
   cp $file $INSTALL_DIR
done

#
# check and add 'INSTALL_DIR' to the user's PATH
#
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
  echo "$INSTALL_DIR is missing in your PATH, adding it."
  add_path $HOME/.bashrc $INSTALL_DIR
fi
