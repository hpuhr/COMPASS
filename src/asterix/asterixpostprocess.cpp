/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "asterixpostprocess.h"

#include "global.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

const float tod_24h = 24 * 60 * 60;

ASTERIXPostProcess::ASTERIXPostProcess() {}

void ASTERIXPostProcess::postProcess(unsigned int category, nlohmann::json& record)
{
    record["category"] = category;

    int sac{-1};
    int sic{-1};

    if (record.contains("010"))
    {
        assert (record.at("010").contains("SAC"));
        sac = record.at("010").at("SAC");

        assert (record.at("010").contains("SIC"));
        sic = record.at("010").at("SIC");

        record["ds_id"] = sac * 256 + sic;
    }

    if (category == 1)  // CAT001 coversion hack
        postProcessCAT001(sac, sic, record);
    else if (category == 2)  // save last tods
        postProcessCAT002(sac, sic, record);
    else if (category == 20)
        postProcessCAT020(sac, sic, record);
    else if (category == 21)
        postProcessCAT021(sac, sic, record);
    else if (category == 48)
        postProcessCAT048(sac, sic, record);
    else if (category == 62)
        postProcessCAT062(sac, sic, record);
}

void ASTERIXPostProcess::postProcessCAT001(int sac, int sic, nlohmann::json& record)
{
    // antenna 0,1 to 1,2
//    if (record.contains("020") && record.at("020").contains("ANT"))
//    {
//        nlohmann::json& item_020 = record.at("020");
//        unsigned int antenna = item_020.at("ANT");
//        item_020.at("ANT") = antenna + 1;
//    }

    // civil emergency
//    if (record.contains("070") && record.at("070").contains("Mode-3/A reply"))
//    {
//        nlohmann::json& item = record.at("070");
//        unsigned int mode3a_code = item.at("Mode-3/A reply");

//        if (mode3a_code == 7500)
//            record["civil_emergency"] = 5;
//        else if (mode3a_code == 7600)
//            record["civil_emergency"] = 6;
//        else if (mode3a_code == 7700)
//            record["civil_emergency"] = 7;
//    }

    // rdpc 0,1 to 1,2
//    if (record.contains("170") && record.at("170").contains("RDPC"))
//    {
//        nlohmann::json& item = record.at("170");
//        unsigned int value = item.at("RDPC");
//        item.at("RDPC") = value + 1;
//    }

    if (record.contains("141") && record.at("141").contains("Truncated Time of Day"))
    {
        if (sac > -1 && sic > -1)  // bingo
        {
            std::pair<unsigned int, unsigned int> sac_sic({sac, sic});

            if (cat002_last_tod_period_.count(sac_sic) > 0)
            {
                double tod = record.at("141").at("Truncated Time of Day");
                // double tod = record.at("140").at("Time-of-Day");
                tod += cat002_last_tod_period_.at(sac_sic);

                //  loginf << "corrected " <<
                //  String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
                //      << " to " << String::timeStringFromDouble(tod)
                //     << " last update " << cat002_last_tod_period_.at(sac_sic);

                record["140"]["Time-of-Day"] = tod;
            }
            else
            {
                loginf << "ASTERIXPostProcess: processRecord: removing truncated tod "
                       << String::timeStringFromDouble(record.at("141").at("Truncated Time of Day"))
                       << " since to CAT002 from sensor " << sac << "/" << sic << " is not present";
                record["140"]["Time-of-Day"] = nullptr;
            }

            //     loginf << "UGA " <<
            //     String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
            //       << " sac " << sac << " sic " << sic << " cnt " <<
            //       cat002_last_tod_period_.count(sac_sic);

            //    assert (record.at("140").at("Time-of-Day") > 3600.0);
        }
        else
        {
            logdbg << "ASTERIXPostProcess: processRecord: skipping cat001 report without sac/sic";
            record["140"]["Time-of-Day"] = nullptr;
        }
    }
    else
    {
        if (sac > -1 && sic > -1)
        {
            std::pair<unsigned int, unsigned int> sac_sic({sac, sic});

            if (cat002_last_tod_.count(sac_sic) > 0)
            {
                record["140"]["Time-of-Day"] =
                    cat002_last_tod_.at(sac_sic);  // set tod, better than nothing
            }
            else
                logdbg << "ASTERIXPostProcess: processRecord: skipping cat001 report without "
                          "truncated time of day"
                       << " or last cat002 time";
        }
        else
            logdbg << "ASTERIXPostProcess: processRecord: skipping cat001 report without truncated "
                      "time of day"
                   << " or sac/sic";
    }
}

void ASTERIXPostProcess::postProcessCAT002(int sac, int sic, nlohmann::json& record)
{
    //"030": "Time of Day": 33501.4140625

    if (record.contains("030"))
    {
        if (sac > -1 && sic > -1)  // bingo
        {
            // std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});
            double cat002_last_tod = record.at("030").at("Time of Day");
            double cat002_last_tod_period = 512.0 * ((int)(cat002_last_tod / 512));
            cat002_last_tod_period_[std::make_pair(sac, sic)] = cat002_last_tod_period;
            cat002_last_tod_[std::make_pair(sac, sic)] = cat002_last_tod;
        }
    }
}

void ASTERIXPostProcess::postProcessCAT020(int sac, int sic, nlohmann::json& record)
{
    // rdp chain 0,1 to 1,2
//    if (record.contains("020") && record.at("020").contains("CHN"))
//    {
//        nlohmann::json& item_020 = record.at("020");
//        unsigned int chain = item_020.at("CHN");
//        item_020.at("CHN") = chain + 1;
//    }

    // altitude capability
//    if (record.contains("230") && record.at("230").contains("ARC"))
//    {
//        nlohmann::json& item_230 = record.at("230");
//        unsigned int arc = item_230.at("ARC");
//        if (arc == 0)
//            item_230["ARC_ft"] = 100.0;
//        else if (arc == 1)
//            item_230["ARC_ft"] = 25.0;
//    }
}

void ASTERIXPostProcess::postProcessCAT021(int sac, int sic, nlohmann::json& record)
{
//    if (record.contains("150"))  // true airspeed
//    {
//        json& air_speed_item = record.at("150");
//        assert(air_speed_item.contains("IM"));
//        assert(air_speed_item.contains("Air Speed"));

//        bool mach = air_speed_item.at("IM") == 1;
//        double airspeed = air_speed_item.at("Air Speed");

//        if (mach)
//        {
//            air_speed_item["Air Speed [knots]"] = airspeed * 0.001 * 666.739;  // lsb, mach 2 kn
//            air_speed_item["Air Speed [mach]"] = airspeed * 0.001;             // lsb
//        }
//        else
//        {
//            air_speed_item["Air Speed [knots]"] = airspeed * 3600.0;   // nm/s 2 kn
//            air_speed_item["Air Speed [mach]"] = airspeed * 0.185205;  // lsb, nm/s 2 mach
//        }
//    }

    // altitude capability
//    if (record.contains("040") && record.at("040").contains("ARC"))
//    {
//        nlohmann::json& item = record.at("040");
//        unsigned int arc = item.at("ARC");
//        if (arc == 0)
//            item["ARC_ft"] = 25.0;
//        else if (arc == 1)
//            item["ARC_ft"] = 100.0;
//    }

    // link technology
//    if (record.contains("210") && record.at("210").contains("LTT"))
//    {
//        nlohmann::json& item = record.at("210");
//        unsigned int ltt = item.at("LTT");
//        // = 0 Other
//        if (ltt == 0)
//        {
//            item["LTT_OTHER"] = "Y";
//            item["LTT_UAT"] = "N";
//            item["LTT_MDS"] = "N";
//            item["LTT_VDL"] = "N";
//        }
//        // = 1 UAT
//        if (ltt == 0)
//        {
//            item["LTT_OTHER"] = "N";
//            item["LTT_UAT"] = "Y";
//            item["LTT_MDS"] = "N";
//            item["LTT_VDL"] = "N";
//        }
//        // = 2 1090 ES
//        else if (ltt == 2)
//        {
//            item["LTT_OTHER"] = "N";
//            item["LTT_UAT"] = "N";
//            item["LTT_MDS"] = "Y";
//            item["LTT_VDL"] = "N";
//        }
//        // = 3 VDL 4
//        else if (ltt == 3)
//        {
//            item["LTT_OTHER"] = "N";
//            item["LTT_UAT"] = "N";
//            item["LTT_MDS"] = "N";
//            item["LTT_VDL"] = "Y";
//        }
//    }

    // ecat str
//    if (record.contains("020") && record.at("020").contains("ECAT"))
//    {
//        nlohmann::json& item = record.at("020");
//        unsigned int ecat = item.at("ECAT");

//        //# 0 = No ADS-B Emitter Category Information
//        //# value record '0' db 'NO_INFO': 20672
//        if (ecat == 0)
//            item["ECAT_str"] = "NO_INFO";

//        //# 1 = light aircraft <= 15500 lbs
//        //# value record '1' db 'LIGHT_AIRCRAFT': 2479
//        else if (ecat == 1)
//            item["ECAT_str"] = "LIGHT_AIRCRAFT";

//        //# 2 = 15500 lbs < small aircraft <75000 lbs
//        //# value record '2' db 'SMALL_AIRCRAFT': 1941
//        else if (ecat == 2)
//            item["ECAT_str"] = "SMALL_AIRCRAFT";

//        //# 3 = 75000 lbs < medium a/c < 300000 lbs
//        //# value record '3' db 'MEDIUM_AIRCRAFT': 44098
//        else if (ecat == 3)
//            item["ECAT_str"] = "MEDIUM_AIRCRAFT";

//        //# 4 = High Vortex Large
//        //# value record '4' db 'HIGH_VORTEX_LARGE': 7232
//        else if (ecat == 4)
//            item["ECAT_str"] = "HIGH_VORTEX_LARGE";

//        //# 5 = 300000 lbs <= heavy aircraft
//        //# value record '5' db 'HEAVY_AIRCRAFT': 56392
//        else if (ecat == 5)
//            item["ECAT_str"] = "HEAVY_AIRCRAFT";

//        //# 6 = highly manoeuvrable (5g acceleration capability) and high speed (>400 knots cruise)
//        else if (ecat == 6)
//            item["ECAT_str"] = "HIGHLY_MANOEUVRABLE";

//        //# 7 to 9 = reserved
//        // elif ecat in (7, 8, 9):
//        //    return None  # ?

//        //# 10 = rotocraft
//        // elif ecat == 10:
//        //    return 'ROTOCRAFT'  # ?
//        else if (ecat == 0)
//            item["ECAT_str"] = "NO_INFO";

//        //# 11 = glider / sailplane
//        else if (ecat == 11)
//            item["ECAT_str"] = "GLIDER";

//        //# 12 = lighter-than-air
//        // elif ecat == 12:
//        //    return 'LIGHTER_THAN_AIR'  # ?
//        else if (ecat == 12)
//            item["ECAT_str"] = "LIGHTER_THAN_AIR";

//        //# 13 = unmanned aerial vehicle
//        else if (ecat == 13)
//            item["ECAT_str"] = "UNMANNED";

//        //# 14 = space / transatmospheric vehicle
//        else if (ecat == 14)
//            item["ECAT_str"] = "SPACE_VEHICLE";

//        //# 15 = ultralight / handglider / paraglider
//        else if (ecat == 15)
//            item["ECAT_str"] = "ULTRALIGHT";

//        //# 16 = parachutist / skydiver
//        else if (ecat == 16)
//            item["ECAT_str"] = "SKYDIVER";

//        //# 17 to 19 = reserved
//        // elif ecat in (17, 18, 19):
//        //    return None  # ?

//        //# 20 = surface emergency vehicle
//        //# value record '20' db 'SURF_EMERGENCY': 286
//        else if (ecat == 20)
//            item["ECAT_str"] = "SURF_EMERGENCY";

//        //# 21 = surface service vehicle
//        //# value record '21' db 'SURF_SERVICE': 12773
//        else if (ecat == 21)
//            item["ECAT_str"] = "SURF_SERVICE";

//        //# 22 = fixed ground or tethered obstruction
//        else if (ecat == 22)
//            item["ECAT_str"] = "GROUND_OBSTRUCTION";

//        //# 23 = cluster obstacle
//        // elif ecat == 23:
//        //    return 'CLUSTER_OBSTACLE'  # ?
//        else if (ecat == 0)
//            item["ECAT_str"] = "NO_INFO";

//        //# 24 = line obstacle
//        else if (ecat == 24)
//            item["ECAT_str"] = "LINE_OBSTACLE";
//    }

    // surveillance status str
//    if (record.contains("020") && record.at("020").contains("SS"))
//    {
//        nlohmann::json& item = record.at("020");
//        unsigned int ss = item.at("SS");

//        //        # 0 No condition reported
//        if (ss == 0)
//            item["SS_str"] = "NO_CONDITION";
//        //        # = 1 Permanent Alert (Emergency condition)
//        else if (ss == 1)
//            item["SS_str"] = "PERMANENT_ALERT";
//        //        # = 2 Temporary Alert (change in Mode 3/A Code other than emergency)
//        else if (ss == 2)
//            item["SS_str"] = "TEMPORARY_ALERT";
//        //        # = 3 SPI set
//        else if (ss == 3)
//            item["SS_str"] = "SPI_SET";
//    }
}

void ASTERIXPostProcess::postProcessCAT048(int sac, int sic, nlohmann::json& record)
{
    // altitude capability
//    if (record.contains("230") && record.at("230").contains("ARC"))
//    {
//        nlohmann::json& item_230 = record.at("230");
//        unsigned int arc = item_230.at("ARC");
//        if (arc == 0)
//            item_230["ARC_ft"] = 100.0;
//        else if (arc == 1)
//            item_230["ARC_ft"] = 25.0;
//    }

    // civil emergency
//    if (record.contains("070") && record.at("070").contains("Mode-3/A reply"))
//    {
//        nlohmann::json& item = record.at("070");
//        unsigned int mode3a_code = item.at("Mode-3/A reply");

//        if (mode3a_code == 7500)
//            record["civil_emergency"] = 5;
//        else if (mode3a_code == 7600)
//            record["civil_emergency"] = 6;
//        else if (mode3a_code == 7700)
//            record["civil_emergency"] = 7;
//    }

    // ground bit
//    if (record.contains("230") && record.at("230").contains("STAT"))
//    {
//        nlohmann::json& item = record.at("230");
//        unsigned int stat = item.at("STAT");

//        //        # = 0 No alert, no SPI, aircraft airborne
//        //        if stat == 0:
//        //            return "N"
//        if (stat == 0)
//            record["ground_bit"] = "N";
//        //        # = 1 No alert, no SPI, aircraft on ground
//        //        if stat == 1:
//        //            return "Y"
//        else if (stat == 1)
//            record["ground_bit"] = "Y";
//        //        # = 2 Alert, no SPI, aircraft airborne
//        //        if stat == 2:
//        //            return "N"
//        else if (stat == 2)
//            record["ground_bit"] = "N";
//        //        # = 3 Alert, no SPI, aircraft on ground
//        //        if stat == 3:
//        //            return "Y"
//        else if (stat == 3)
//            record["ground_bit"] = "Y";
//    }

    // mode4 friendly
//    if (record.contains("020") && record.at("020").contains("FOE/FRI"))
//    {
//        nlohmann::json& item = record.at("020");
//        unsigned int foefrie = item.at("FOE/FRI");

//        //#Mode-4 interrorgation type:
//        //# - = no interrogation, 0 No Mode 4 interrogation
//        // if frifoe == 0:
//        //    return '-'
//        if (foefrie == 0)
//            record["mode4_friendly"] = "N";

//        //# F = Friendly target, 1 Friendly target
//        // if frifoe == 1:
//        //    return 'F'
//        else if (foefrie == 1)
//            record["mode4_friendly"] = "F";

//        //# U = Unknown Target, 2 Unknown target
//        // if frifoe == 2:
//        //    return 'U'
//        else if (foefrie == 2)
//            record["mode4_friendly"] = "U";

//        //# N = No Reply, 3 No reply
//        // if frifoe == 3:
//        //    return "N"
//        else if (foefrie == 3)
//            record["mode4_friendly"] = "N";
//    }

    // rdp chain 0,1 to 1,2
//    if (record.contains("020") && record.at("020").contains("RDP"))
//    {
//        nlohmann::json& item = record.at("020");
//        unsigned int value = item.at("RDP");
//        item.at("RDP") = value + 1;
//    }

    // track climb desc mode
//    if (record.contains("170") && record.at("170").contains("CDM"))
//    {
//        nlohmann::json& item = record.at("170");
//        unsigned int cdm = item.at("CDM");

//        //# value record '0' db 'M': 4117 M = Maintaining
//        // if cdm == 0:
//        //    return 'M'
//        if (cdm == 0)
//            record["track_climb_desc_mode"] = "M";
//        //# value record '1' db 'C': 1133 C = Climbing
//        // if cdm == 1:
//        //    return 'C'
//        else if (cdm == 1)
//            record["track_climb_desc_mode"] = "C";
//        //# value record '2' db 'D': 330 D = Descending
//        // if cdm == 2:
//        //    return 'D'
//        else if (cdm == 2)
//            record["track_climb_desc_mode"] = "D";
//        //# value record '3' db 'I': 569 I = Invalid
//        // if cdm == 3:
//        //    return 'I'
//        else if (cdm == 3)
//            record["track_climb_desc_mode"] = "I";
//    }

    // report type
//    if (record.contains("161") && record.at("161").contains("TRACK NUMBER"))
//        record["report_type"] = 1;
//    else
//        record["report_type"] = 0;
}

void ASTERIXPostProcess::postProcessCAT062(int sac, int sic, nlohmann::json& record)
{
//    if (record.contains("185"))
//    {
//        // 185.Vx Vy
//        json& speed_item = record.at("185");
//        assert(speed_item.contains("Vx"));
//        assert(speed_item.contains("Vy"));

//        double v_x = speed_item.at("Vx");
//        double v_y = speed_item.at("Vy");

//        double speed = sqrt(pow(v_x, 2) + pow(v_y, 2)) * 1.94384;  // ms2kn
//        double track_angle = atan2(v_x, v_y) * RAD2DEG;

//        speed_item["Ground Speed"] = speed;
//        speed_item["Track Angle"] = track_angle;
//    }

//    if (record.contains("080") && record.at("080").contains("PSR") &&
//        record.at("080").contains("SSR") &&
//        record.at("080").contains("MDS"))  // && record.at("080").contains("ADS") not used
//    {
//        //            if find_value("080.CST", record) == 1:
//        //                return 0  # no detection
//        if (record.at("080").contains("CST") && record.at("080").at("CST") == 1)
//            record["detection_type"] = 0;  // no detection
//        else
//        {
//            //            psr_updated = find_value("080.PSR", record) == 0
//            //            ssr_updated = find_value("080.SSR", record) == 0
//            //            mds_updated = find_value("080.MDS", record) == 0
//            //            ads_updated = find_value("080.ADS", record) == 0
//            bool psr_updated = record.at("080").at("PSR") == 0;
//            bool ssr_updated = record.at("080").at("SSR") == 0;
//            bool mds_updated = record.at("080").at("MDS") == 0;
//            // bool ads_updated = record.at("080").at("ADS");

//            //            if not mds_updated:
//            if (!mds_updated)
//            {
//                //                if psr_updated and not ssr_updated:
//                if (psr_updated && !ssr_updated)
//                {
//                    //                    if find_value("290.MLT.Age", record) is not None:
//                    //                        # age not 63.75
//                    //                        mlat_age = find_value("290.MLT.Age", record)
//                    if (record.contains("290") && record.at("290").contains("MLT") &&
//                        record.at("290").at("MLT").contains("Age") &&
//                        record.at("290").at("MLT").at("Age") <= 12.0)
//                        //                        if mlat_age <= 12.0:
//                        //                            return 3
//                        record["detection_type"] = 3;  // combined psr & mlat ssr
//                    else
//                        //                    return 1  # single psr, no mode-s
//                        record["detection_type"] = 1;  // single psr, no mode-s
//                }
//                //                if not psr_updated and ssr_updated:
//                //                    return 2  # single ssr, no mode-s
//                else if (!psr_updated && ssr_updated)
//                    record["detection_type"] = 2;  // single ssr, no mode-s
//                //                if psr_updated and ssr_updated:
//                //                    return 3  # cmb, no mode-s
//                else if (psr_updated && ssr_updated)
//                    record["detection_type"] = 2;  // single ssr, no mode-s

//                // not psr_updated and not ssr_updated:

//                //            if find_value("380.ADR.Target Address", record) is not None:
//                //                return 5

//                else if (record.contains("380") && record.at("380").contains("ADR") &&
//                         record.at("380").at("ADR").contains("Target Address"))
//                    record["detection_type"] = 5;  // ssr, mode-s

//                //            if find_value("060.Mode-3/A reply", record) is not None \
//                //                    or find_value("136.Measured Flight Level", record) is not None:
//                //                return 2
//                else if ((record.contains("060") && record.at("060").contains("Mode-3/A reply")) ||
//                         (record.contains("136") &&
//                          record.at("136").contains("Measured Flight Level")))
//                    record["detection_type"] = 5;  // ssr, mode-s

//                //            return 0  # unknown
//                else
//                    record["detection_type"] = 0;  // unkown
//            }
//            //            else:
//            else
//            {
//                //                if not psr_updated:
//                //                    return 5  # ssr, mode-s
//                //                else:
//                //                    return 7  # cmb, mode-s
//                if (!psr_updated)
//                    record["detection_type"] = 5;  // ssr, mode-s
//                else
//                    record["detection_type"] = 7;  // cmb, mode-s
//            }
//        }
//    }

    //340.SID.SAC

//    if (record.contains("340") && record.at("340").contains("SID")
//            && record.at("340").at("SID").contains("SAC")
//            && record.at("340").at("SID").contains("SIC"))
//    {
//        unsigned int lu_sac = record.at("340").at("SID").at("SAC");
//        unsigned int lu_sic = record.at("340").at("SID").at("SIC");
//        record["track_lu_ds_id"] = lu_sac * 256 + lu_sic;
//    }

    // 380.COM.STAT
    //    Flight Status
    //    = 0 No alert, no SPI, aircraft airborne
    //    = 1 No alert, no SPI, aircraft on ground
    //    = 2 Alert, no SPI, aircraft airborne
    //    = 3 Alert, no SPI, aircraft on ground
    //    = 4 Alert, SPI, aircraft airborne or on ground
    //    = 5 No alert, SPI, aircraft airborne or on ground
    //    = 6 Not defined
    //    = 7 Unknown or not yet extracted

//    if (record.contains("380") && record.at("380").contains("COM")
//            && record.at("380").at("COM").contains("STAT"))
//    {
//        unsigned int stat = record.at("380").at("COM").at("STAT");
//        record["fs_alert"] = (stat == 2) || (stat == 3) || (stat == 4);
//        record["fs_spi"] = (stat == 4) || (stat == 5);
//        record["fs_gbs"] = (stat == 1) || (stat == 3);

//        //select ground_bit,count(*) from sd_track group by ground_bit;
//    }

    // overrides
    if (override_active_)
    {
        if (record.contains("010") && record.at("010").contains("SAC") &&
            record.at("010").contains("SIC") &&
            record.at("010").at("SAC") == this->override_sac_org_ &&
            record.at("010").at("SIC") == this->override_sic_org_)
        {
            record.at("010").at("SAC") = override_sac_new_;
            record.at("010").at("SIC") = override_sic_new_;
            record["ds_id"] = override_sac_new_ * 256 + override_sic_new_;
        }

        if (record.contains("070") && record.at("070").contains("Time Of Track Information"))
        {
            float tod = record.at("070").at("Time Of Track Information");  // in seconds

            tod += override_tod_offset_;

            // check for out-of-bounds because of midnight-jump
            while (tod < 0.0f)
                tod += tod_24h;
            while (tod > tod_24h)
                tod -= tod_24h;

            assert(tod >= 0.0f);
            assert(tod <= tod_24h);

            record.at("070").at("Time Of Track Information") = tod;
        }
    }
}
