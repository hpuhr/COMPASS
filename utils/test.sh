#!/bin/bash

#
# creates a SQLite3 file container loadable into ATSDB with the current
# ADS-B exchange database between 08Z and 09Z
#

ADSBdb=`ls | grep -E '[[:digit:]]{4}-[[:digit:]]{2}-[[:digit:]]{2}.zip'`
if [ $? -ne 0 ]; then
   echo "ADS-B exchange database has not been found in $PWD"
   echo " * expecting a file with name 'yyyy-mm-dd.zip'"
   exit 1
fi

ADSBxchg=`echo $ADSBdb | sed 's/\.zip$//g'`

prepADSBxchg -f $ADSBxchg.zip -t 8 -a
csv2atsdb -f $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.csv -n ADSBx -t A -b 1M

#
# remove all receivers and create a unique receiver with ds_id of '1'
#
echo "delete from ds_ads where _rowid_ > '1';" | sqlite3 $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db
echo "delete from le_ds where _rowid_ > '1';" | sqlite3 $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db
echo "update ds_ads set ds_id = '1';" | sqlite3 $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db
echo "update le_ds set ds_id = '1';" | sqlite3 $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db
echo "update sd_ads set ds_id = '1';" | sqlite3 $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db
mv $ADSBxchg/08/ADS-Bxchg.$ADSBxchg.db .
