# exit when any command fails
set -e

echo "App name: $1";

#$1 --reset --quit
mkdir -p ~/data/test/tmp/
$1 --import_data_sources_file ~/data/test/test1/data_sources.json --quit
$1 --create_db ~/data/test/tmp/test1_eval.db --import_asterix_file ~/data/test/test1/test.ff --import_asterix_file_line L1 --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test1_eval.db --import_sectors_json ~/data/test/test1/sectors.json --quit
$1 --open_db ~/data/test/tmp/test1_eval.db --associate_data --load --quit
$1 --open_db ~/data/test/tmp/test1_eval.db --evaluate --evaluation_parameters '{"active_sources_ref": {"CAT062": {"12990": true}}, "active_sources_tst": {"CAT021": {"12830": true }}, "current_standard": "Dubious Targets", "dbcontent_name_ref": "CAT062", "dbcontent_name_tst": "CAT021", "use_grp_in_sector": {"Dubious Targets": {"fir_cut_sim": {"Optional": true }}}}' --export_eval_report ~/data/test/test1/tmp/dub_adsb_eval/report.tex --no_cfg_save --quit
$1 --open_db ~/data/test/tmp/test1_eval.db --evaluate --evaluation_parameters '{"active_sources_ref": { "CAT062": {"12990": true}}, "active_sources_tst": { "CAT020": { "12820": true, "12821": true, "12822": true, "12823": true, "12824": true, "12825": true, "12826": true, "12827": true, "12828": true, "12831": true },  "CAT021": { "12830": true }}, "current_standard": "test", "dbcontent_name_ref": "CAT062", "dbcontent_name_tst": "CAT020", "use_grp_in_sector": {"test": {"fir_cut_sim": {"Mandatory": true }}}}' --export_eval_report ~/data/test/tmp/test1_mlat_eval/report.tex --no_cfg_save --quit


