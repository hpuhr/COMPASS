# exit when any command fails
set -e

echo "App name: $1";

#$1 --reset --quit
mkdir -p ~/data/test/tmp/
$1 --import_data_sources_file ~/data/test/test2/data_sources.json --quit
$1 --create_db ~/data/test/tmp/test2.db --import_asterix_file ~/data/test/test2/test.ff --import_asterix_file_line L1  --asterix_framing ioss --quit
$1 --open_db ~/data/test/tmp/test2.db --import_sectors_json ~/data/test/test2/sectors.json --quit
$1 --open_db ~/data/test/tmp/test2.db --associate_data --load --quit
# test adsb full report
#$1 --open_db ~/data/test/tmp/test2.db --evaluate --evaluation_parameters '{"active_sources_ref": {"CAT062": {"30650": true}}, "active_sources_tst": {"CAT021": {"547": true }}, "current_standard": "test", "dbcontent_name_ref": "CAT062", "dbcontent_name_tst": "CAT021",         "report_include_target_details": true, "report_include_target_tr_details": false, "report_skip_targets_wo_issues": false, "use_grp_in_sector": {"test": { "MATS_DOI": { "Mandatory": false }, "MATS_TMA": {"Mandatory": false }, "doi_adsb": { "Mandatory": true }, "tma2": {"Mandatory": false }}}}' --export_eval_report ~/data/test/tmp/test2_test_adsb_full_eval/report.tex --no_cfg_save --quit
# dub adsb full report
#$1 --open_db ~/data/test/tmp/test2.db --evaluate --evaluation_parameters '{"active_sources_ref": {"CAT062": {"30650": true}}, "active_sources_tst": {"CAT021": {"547": true }}, "current_standard": "Dubious Targets", "dbcontent_name_ref": "CAT062", "dbcontent_name_tst": "CAT021",         "report_include_target_details": true, "report_include_target_tr_details": false, "report_skip_targets_wo_issues": false, "use_grp_in_sector": {"Dubious Targets": { "MATS_DOI": { "Optional": false }, "MATS_TMA": {"Optional": false }, "doi_adsb": { "Optional": true }, "tma2": {"Optional": false }}}}' --export_eval_report ~/data/test/tmp/test2_dub_adsb_full_eval/report.tex --no_cfg_save --quit
# dub adsb targets with issues report
$1 --open_db ~/data/test/tmp/test2.db --evaluate --evaluation_parameters '{"active_sources_ref": {"CAT062": {"30650": true}}, "active_sources_tst": {"CAT021": {"547": true }}, "current_standard": "Dubious Targets", "dbcontent_name_ref": "CAT062", "dbcontent_name_tst": "CAT021", "line_id_ref": 0, "line_id_tst": 0, "report_include_target_details": true, "report_include_target_tr_details": false, "report_skip_targets_wo_issues": true, "use_grp_in_sector": {"Dubious Targets": { "MATS_DOI": { "Optional": false }, "MATS_TMA": {"Optional": false }, "doi_adsb": { "Optional": true }, "tma2": {"Optional": false }}}}' --export_eval_report ~/data/test/tmp/test2_dub_adsb_issues_eval/report.tex --no_cfg_save --quit


