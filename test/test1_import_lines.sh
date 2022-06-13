# exit when any command fails
set -e

echo "App name: $1";

#$1 --reset --quit
mkdir -p ~/data/test/tmp/
$1 --import_data_sources_file ~/data/test/test1/data_sources.json --quit
$1 --create_db ~/data/test/tmp/test1_lines.db --import_asterix_file ~/data/test/test1/test.ff --import_asterix_file_line L1  --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test1_lines.db --import_asterix_file ~/data/test/test1/test.ff --import_asterix_file_line L2  --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test1_lines.db --import_asterix_file ~/data/test/test1/test.ff --import_asterix_file_line L3  --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test1_lines.db --import_asterix_file ~/data/test/test1/test.ff --import_asterix_file_line L4  --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test1_lines.db --associate_data --load --quit

