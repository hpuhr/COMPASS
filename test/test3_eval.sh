# exit when any command fails
set -e

echo "App name: $1";

#$1 --reset --quit
mkdir -p ~/data/test/tmp/
$1 --import_data_sources_file ~/data/test/test3/ASMGCS_Sensor.json --quit
$1 --create_db ~/data/test/tmp/test3.db --import_asterix_file ~/data/test/test3/REC_ASMGCS_ADSB_20220307.atx --import_asterix_file_line L1 --asterix_framing none --quit
$1 --open_db ~/data/test/tmp/test3.db --import_asterix_file ~/data/test/test3/REC_ASMGCS_MLAT_20220307.atx --import_asterix_file_line L1 --asterix_framing none --quit
$1 --open_db ~/data/test/tmp/test3.db --import_asterix_file ~/data/test/test3/REC_ASMGCS_TRACKER_20220307.atx --import_asterix_file_line L1 --asterix_framing none --quit
$1 --open_db ~/data/test/tmp/test3.db --import_sectors_json ~/data/test/test3/ASMGCS_Polygon.json --quit
# first trail
$1 --open_db ~/data/test/tmp/test3.db --import_gps_trail ~/data/test/test3/MobaXterm_COM8StandardSerialoverBluetoothlinkCOM8_20220307_110934.txt --import_gps_parameters '{"callsign": "TEH01", "ds_name": "GPS Trail", "ds_sac": 0, "ds_sic": 0, "set_callsign": true,  "set_mode_3a_code": false,  "set_target_address": true, "target_address": 5250912, "tod_offset": 0.0}'  --quit
# second trail
$1 --open_db ~/data/test/tmp/test3.db --import_gps_trail ~/data/test/test3/MobaXterm_COM8StandardSerialoverBluetoothlinkCOM8_20220307_110934.txt --import_gps_parameters '{"callsign": "TEH08", "ds_name": "GPS Trail", "ds_sac": 0, "ds_sic": 0, "mode_3a_code": 512, "set_callsign": true,  "set_mode_3a_code": true,  "set_target_address": true, "target_address": 5250919, "tod_offset": 0.0}'  --quit

$1 --open_db ~/data/test/tmp/test3.db --associate_data --load --quit
# ed-117a
$1 --open_db ~/data/test/tmp/test3.db --evaluate --evaluation_parameters '{"active_sources_ref": {"RefTraj": {"0": true }}, "active_sources_tst": {"CAT020": {"6427": true }}, "current_standard": "EUROCAE ED-117A", "dbcontent_name_ref": "RefTraj", "dbcontent_name_tst": "CAT020",         "report_include_target_details": true, "report_include_target_tr_details": false, "report_skip_targets_wo_issues": false, "use_grp_in_sector": {"EUROCAE ED-117A": { "All": {"Active Stands": false, "Apron Taxiways": false, "Common": true, "Manoeuvring Area": true }}}}' --export_eval_report ~/data/test/tmp/test3_ed117a_full_eval/report.tex --no_cfg_save --quit

