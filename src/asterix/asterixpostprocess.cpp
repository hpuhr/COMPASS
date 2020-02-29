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
    else if (category == 21)
        postProcessCAT021 (sac, sic, record);
    else if (category == 62)
        postProcessCAT062 (sac, sic, record);
}

void ASTERIXPostProcess::postProcessCAT001 (int sac, int sic, nlohmann::json& record)
{
    //        if (record.find("090") != record.end())
    //            if (record.at("090").find("Flight Level") != record.at("090").end())
    //            {
    //                double flight_level = record.at("090").at("Flight Level"); // is mapped in ft
    //                record.at("090").at("Flight Level") = flight_level* 1e-2;  // ft to fl
    //            }

    // "141":  "Truncated Time of Day": 221.4296875 mapped to "140.Time-of-Day"

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

                //                    loginf << "corrected " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
                //                           << " to " << String::timeStringFromDouble(tod)
                //                           << " last update " << cat002_last_tod_period_.at(sac_sic);

                record["140"]["Time-of-Day"] = tod;
            }
            else
            {
                loginf << "ASTERIXDecodeJob: processRecord: removing truncated tod "
                       << String::timeStringFromDouble(record.at("141").at("Truncated Time of Day"))
                       << " since to CAT002 from sensor "<< sac << "/" << sic << " is not present";
                record["140"]["Time-of-Day"] = nullptr;
            }

            //                loginf << "UGA " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
            //                       << " sac " << sac << " sic " << sic << " cnt " << cat002_last_tod_period_.count(sac_sic);

            //                assert (record.at("140").at("Time-of-Day") > 3600.0);
        }
        else
        {
            loginf << "ASTERIXDecodeJob: processRecord: skipping cat001 report without sac/sic";
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
                logdbg << "ASTERIXDecodeJob: processRecord: skipping cat001 report without truncated time of day"
                       << " or last cat002 time";
        }
        else
            logdbg << "ASTERIXDecodeJob: processRecord: skipping cat001 report without truncated time of day"
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

//void ASTERIXPostProcess::postProcessCAT020 (int sac, int sic, nlohmann::json& record)
//{

//}

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

//void ASTERIXPostProcess::postProcessCAT048 (int sac, int sic, nlohmann::json& record)
//{

//}

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
