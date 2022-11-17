# exit when any command fails
set -e

echo "App name: $1";

#$1 --reset --quit
mkdir -p ~/data/test/tmp/
rm -rf ~/data/test/tmp/*

./test1_eval.sh $1
./test1_import_lines.sh $1
./test1_import.sh $1
./test2_eval.sh $1
./test3_eval.sh $1
./test4_eval.sh $1
./test5_cat010.sh $1
./test6_vp.sh $1
