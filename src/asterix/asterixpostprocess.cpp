#include "asterixpostprocess.h"
#include "global.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

const float tod_24h = 24*60*60;

ASTERIXPostProcess::ASTERIXPostProcess()
{
}


void ASTERIXPostProcess::postProcess (unsigned int category, nlohmann::json& record)
{
    record["category"] = category;

    int sac {-1};
    int sic {-1};

    if (record.contains("010"))
    {
        sac = record.at("010").at("SAC");
        sic = record.at("010").at("SIC");
        record["ds_id"] =  sac*256 + sic;
    }

    if (category == 1) // CAT001 coversion hack
        postProcessCAT001 (sac, sic, record);
    else if (category == 2) // save last tods
        postProcessCAT002 (sac, sic, record);
    else if (category == 20)
        postProcessCAT020 (sac, sic, record);
    else if (category == 21)
        postProcessCAT021 (sac, sic, record);
    else if (category == 48)
        postProcessCAT048 (sac, sic, record);
    else if (category == 62)
        postProcessCAT062 (sac, sic, record);
}

void ASTERIXPostProcess::postProcessCAT001 (int sac, int sic, nlohmann::json& record)
{
    // antenna 0,1 to 1,2
    if (record.contains("020") && record.at("020").contains("ANT"))
    {
        nlohmann::json& item_020 = record.at("020");
        unsigned int antenna = item_020.at("ANT");
        item_020.at("ANT") = antenna + 1;
    }

    // civil emergency
    if (record.contains("070") && record.at("070").contains("Mode-3/A reply"))
    {
        nlohmann::json& item = record.at("070");
        unsigned int mode3a_code = item.at("Mode-3/A reply");

        if (mode3a_code == 7500)
            record["civil_emergency"] = 5;
        else if (mode3a_code == 7600)
            record["civil_emergency"] = 6;
        else if (mode3a_code == 7700)
            record["civil_emergency"] = 7;
    }

    // rdpc 0,1 to 1,2
    if (record.contains("170") && record.at("170").contains("RDPC"))
    {
        nlohmann::json& item = record.at("170");
        unsigned int value = item.at("RDPC");
        item.at("RDPC") = value + 1;
    }

    if (record.contains("141") && record.at("141").contains("Truncated Time of Day"))
    {
        if (sac > -1 && sic > -1 ) // bingo
        {
            std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});

            if (cat002_last_tod_period_.count(sac_sic) > 0)
            {
                double tod = record.at("141").at("Truncated Time of Day");
                //double tod = record.at("140").at("Time-of-Day");
                tod += cat002_last_tod_period_.at(sac_sic);

                //  loginf << "corrected " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
                //      << " to " << String::timeStringFromDouble(tod)
                //     << " last update " << cat002_last_tod_period_.at(sac_sic);

                record["140"]["Time-of-Day"] = tod;
            }
            else
            {
                loginf << "ASTERIXPostProcess: processRecord: removing truncated tod "
                       << String::timeStringFromDouble(record.at("141").at("Truncated Time of Day"))
                       << " since to CAT002 from sensor "<< sac << "/" << sic << " is not present";
                record["140"]["Time-of-Day"] = nullptr;
            }

            //     loginf << "UGA " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
            //       << " sac " << sac << " sic " << sic << " cnt " << cat002_last_tod_period_.count(sac_sic);

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
        if (sac > -1 && sic > -1 )
        {
            std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});

            if (cat002_last_tod_.count(sac_sic) > 0)
            {
                record["140"]["Time-of-Day"] = cat002_last_tod_.at(sac_sic); // set tod, better than nothing
            }
            else
                logdbg << "ASTERIXPostProcess: processRecord: skipping cat001 report without truncated time of day"
                       << " or last cat002 time";
        }
        else
            logdbg << "ASTERIXPostProcess: processRecord: skipping cat001 report without truncated time of day"
                   << " or sac/sic";
    }
}

void ASTERIXPostProcess::postProcessCAT002 (int sac, int sic, nlohmann::json& record)
{
    //"030": "Time of Day": 33501.4140625

    if (record.contains("030"))
    {
        if (sac > -1 && sic > -1) // bingo
        {
            //std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});
            double cat002_last_tod = record.at("030").at("Time of Day");
            double cat002_last_tod_period = 512.0 * ((int)(cat002_last_tod / 512));
            cat002_last_tod_period_ [std::make_pair(sac, sic)] = cat002_last_tod_period;
            cat002_last_tod_ [std::make_pair(sac, sic)] = cat002_last_tod;
        }
    }
}

void ASTERIXPostProcess::postProcessCAT020 (int sac, int sic, nlohmann::json& record)
{
    // rdp chain 0,1 to 1,2
    if (record.contains("020") && record.at("020").contains("CHN"))
    {
        nlohmann::json& item_020 = record.at("020");
        unsigned int chain = item_020.at("CHN");
        item_020.at("CHN") = chain + 1;
    }

    // altitude capability
    if (record.contains("230") && record.at("230").contains("ARC"))
    {
        nlohmann::json& item_230 = record.at("230");
        unsigned int arc = item_230.at("ARC");
        if (arc == 0)
            item_230["ARC_ft"] = 100.0;
        else if (arc == 1)
            item_230["ARC_ft"] = 25.0;
    }
}

void ASTERIXPostProcess::postProcessCAT021 (int sac, int sic, nlohmann::json& record)
{
    if (record.contains("150")) // true airspeed
    {
        json& air_speed_item = record.at("150");
        assert (air_speed_item.contains("IM"));
        assert (air_speed_item.contains("Air Speed"));

        bool mach = air_speed_item.at("IM") == 1;
        double airspeed = air_speed_item.at("Air Speed");

        if (mach)
        {
            air_speed_item["Air Speed [knots]"] = airspeed*666.739;
            air_speed_item["Air Speed [mach]"] = airspeed;
        }
        else
        {
            air_speed_item["Air Speed [knots]"] = airspeed;
            air_speed_item["Air Speed [mach]"] = airspeed/666.739;
        }
    }
    //        else if (record.contains("160"))
    //        {
    //            json& ground_speed_item = record.at("160");
    //            //assert (ground_speed_item.contains("IM"));
    //            assert (ground_speed_item.contains("Air Speed));

    //            double ground_speed = ground_speed_item.at("Ground Speed");
    //            ground_speed_item.at("Ground Speed") = ground_speed * 3600;
    //        }
}

void ASTERIXPostProcess::postProcessCAT048 (int sac, int sic, nlohmann::json& record)
{
    // altitude capability
    if (record.contains("230") && record.at("230").contains("ARC"))
    {
        nlohmann::json& item_230 = record.at("230");
        unsigned int arc = item_230.at("ARC");
        if (arc == 0)
            item_230["ARC_ft"] = 100.0;
        else if (arc == 1)
            item_230["ARC_ft"] = 25.0;
    }

    // civil emergency
    if (record.contains("070") && record.at("070").contains("Mode-3/A reply"))
    {
        nlohmann::json& item = record.at("070");
        unsigned int mode3a_code = item.at("Mode-3/A reply");

        if (mode3a_code == 7500)
            record["civil_emergency"] = 5;
        else if (mode3a_code == 7600)
            record["civil_emergency"] = 6;
        else if (mode3a_code == 7700)
            record["civil_emergency"] = 7;
    }

    // ground bit
    if (record.contains("230") && record.at("230").contains("STAT"))
    {
        nlohmann::json& item = record.at("230");
        unsigned int stat = item.at("STAT");

        //        # = 0 No alert, no SPI, aircraft airborne
        //        if stat == 0:
        //            return 'N'
        if (stat == 0)
            record["ground_bit"] = "N";
        //        # = 1 No alert, no SPI, aircraft on ground
        //        if stat == 1:
        //            return 'Y'
        else if (stat == 1)
            record["ground_bit"] = "Y";
        //        # = 2 Alert, no SPI, aircraft airborne
        //        if stat == 2:
        //            return 'N'
        else if (stat == 2)
            record["ground_bit"] = "N";
        //        # = 3 Alert, no SPI, aircraft on ground
        //        if stat == 3:
        //            return 'Y'
        else if (stat == 3)
            record["ground_bit"] = "Y";
    }

    // mode4 friendly
    if (record.contains("020") && record.at("020").contains("FOE/FRI"))
    {
        nlohmann::json& item = record.at("020");
        unsigned int foefrie = item.at("FOE/FRI");

        //#Mode-4 interrorgation type:
        //# - = no interrogation, 0 No Mode 4 interrogation
        //if frifoe == 0:
        //    return '-'
        if (foefrie == 0)
            record["mode4_friendly"] = "N";

        //# F = Friendly target, 1 Friendly target
        //if frifoe == 1:
        //    return 'F'
        else if (foefrie == 1)
            record["mode4_friendly"] = "F";

        //# U = Unknown Target, 2 Unknown target
        //if frifoe == 2:
        //    return 'U'
        else if (foefrie == 2)
            record["mode4_friendly"] = "U";

        //# N = No Reply, 3 No reply
        //if frifoe == 3:
        //    return 'N'
        else if (foefrie == 3)
            record["mode4_friendly"] = "N";
    }

    // rdp chain 0,1 to 1,2
    if (record.contains("020") && record.at("020").contains("RDP"))
    {
        nlohmann::json& item = record.at("020");
        unsigned int value = item.at("RDP");
        item.at("RDP") = value + 1;
    }

    // track climb desc mode
    if (record.contains("170") && record.at("170").contains("CDM"))
    {
        nlohmann::json& item = record.at("170");
        unsigned int cdm = item.at("CDM");

        //# value record '0' db 'M': 4117 M = Maintaining
        //if cdm == 0:
        //    return 'M'
        if (cdm == 0)
            record["track_climb_desc_mode"] = "M";
        //# value record '1' db 'C': 1133 C = Climbing
        //if cdm == 1:
        //    return 'C'
        else if (cdm == 1)
            record["track_climb_desc_mode"] = "C";
        //# value record '2' db 'D': 330 D = Descending
        //if cdm == 2:
        //    return 'D'
        else if (cdm == 2)
            record["track_climb_desc_mode"] = "D";
        //# value record '3' db 'I': 569 I = Invalid
        //if cdm == 3:
        //    return 'I'
        else if (cdm == 3)
            record["track_climb_desc_mode"] = "I";
    }
}

void ASTERIXPostProcess::postProcessCAT062 (int sac, int sic, nlohmann::json& record)
{
    if (record.contains("185"))
    {
        //185.Vx Vy
        json& speed_item = record.at("185");
        assert (speed_item.contains("Vx"));
        assert (speed_item.contains("Vy"));

        double v_x = speed_item.at("Vx");
        double v_y = speed_item.at("Vy");

        double speed = sqrt(pow(v_x, 2) + pow(v_y, 2)) * 1.94384; // ms2kn
        double track_angle = atan2(v_x, v_y) * RAD2DEG;

        speed_item["Ground Speed"] = speed;
        speed_item["Track Angle"] = track_angle;
    }

    if (record.contains("080")
            && record.at("080").contains("PSR")
            && record.at("080").contains("SSR")
            && record.at("080").contains("MDS")) // && record.at("080").contains("ADS") not used
    {
        //            if find_value("080.CST", record) == 1:
        //                return 0  # no detection
        if (record.at("080").contains("CST") && record.at("080").at("CST") == 1)
            record["detection_type"] = 0;  // no detection
        else
        {
            //            psr_updated = find_value("080.PSR", record) == 0
            //            ssr_updated = find_value("080.SSR", record) == 0
            //            mds_updated = find_value("080.MDS", record) == 0
            //            ads_updated = find_value("080.ADS", record) == 0
            bool psr_updated = record.at("080").at("PSR") == 0;
            bool ssr_updated = record.at("080").at("SSR") == 0;
            bool mds_updated = record.at("080").at("MDS") == 0;
            //bool ads_updated = record.at("080").at("ADS");

            //            if not mds_updated:
            if (!mds_updated)
            {

                //                if psr_updated and not ssr_updated:
                if (psr_updated && !ssr_updated)
                {
                    //                    if find_value("290.MLT.Age", record) is not None:
                    //                        # age not 63.75
                    //                        mlat_age = find_value("290.MLT.Age", record)
                    if (record.contains("290")
                            && record.at("290").contains("MLT")
                            && record.at("290").at("MLT").contains("Age")
                            && record.at("290").at("MLT").at("Age") <= 12.0)
                        //                        if mlat_age <= 12.0:
                        //                            return 3
                        record["detection_type"] = 3; // combined psr & mlat ssr
                    else
                        //                    return 1  # single psr, no mode-s
                        record["detection_type"] = 1; //single psr, no mode-s
                }
                //                if not psr_updated and ssr_updated:
                //                    return 2  # single ssr, no mode-s
                else if (!psr_updated && ssr_updated)
                    record["detection_type"] = 2; // single ssr, no mode-s
                //                if psr_updated and ssr_updated:
                //                    return 3  # cmb, no mode-s
                else if (psr_updated && ssr_updated)
                    record["detection_type"] = 2; // single ssr, no mode-s

                // not psr_updated and not ssr_updated:

                //            if find_value("380.ADR.Target Address", record) is not None:
                //                return 5

                else if (record.contains("380")
                         && record.at("380").contains("ADR")
                         && record.at("380").at("ADR").contains("Target Address"))
                    record["detection_type"] = 5;  // ssr, mode-s

                //            if find_value("060.Mode-3/A reply", record) is not None \
                //                    or find_value("136.Measured Flight Level", record) is not None:
                //                return 2
                else if ((record.contains("060") && record.at("060").contains("Mode-3/A reply"))
                         || (record.contains("136") && record.at("136").contains("Measured Flight Level")))
                    record["detection_type"] = 5;  // ssr, mode-s

                //            return 0  # unknown
                else
                    record["detection_type"] = 0;  // unkown
            }
            //            else:
            else
            {
                //                if not psr_updated:
                //                    return 5  # ssr, mode-s
                //                else:
                //                    return 7  # cmb, mode-s
                if (!psr_updated)
                    record["detection_type"] = 5; // ssr, mode-s
                else
                    record["detection_type"] = 7; // cmb, mode-s
            }
        }
    }

    // overrides
    if (override_active_)
    {
        if (record.contains("010")
                && record.at("010").contains("SAC")
                && record.at("010").contains("SIC")
                && record.at("010").at("SAC") == this->override_sac_org_
                && record.at("010").at("SIC") == this->override_sic_org_)
        {
            record.at("010").at("SAC") = override_sac_new_;
            record.at("010").at("SIC") = override_sic_new_;
            record["ds_id"] =  override_sac_new_*256 + override_sic_new_;
        }

        if (record.contains("070")
                && record.at("070").contains("Time Of Track Information"))
        {
            float tod = record.at("070").at("Time Of Track Information"); // in seconds

            tod += override_tod_offset_;

            // check for out-of-bounds because of midnight-jump
            while (tod < 0.0f)
                tod += tod_24h;
            while (tod > tod_24h)
                tod -= tod_24h;

            assert (tod >= 0.0f);
            assert (tod <= tod_24h);

            record.at("070").at("Time Of Track Information") = tod;
        }
    }
}
