#include "fftmanager.h"
#include "configurationfft.h"
#include "fftsconfigurationdialog.h"
#include "compass.h"
#include "dbinterface.h"
#include "fft/dbfft.h"
#include "stringconv.h"
#include "number.h"
#include "files.h"
#include "json.hpp"
#include "logger.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "global.h"

#include <osgEarth/GeoMath>

#include <QMessageBox>

#include <fstream>

using namespace std;
using namespace Utils;
using namespace nlohmann;

FFTManager::FFTManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "ffts.json"), compass_(*compass)
{
    createSubConfigurables();

    updateFFTNamesAll();
}

FFTManager::~FFTManager()
{
    loginf << "FFTManager: dtor";
}

void FFTManager::generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id)
{
    if (class_id == "ConfigurationFFT")
    {
        unique_ptr<ConfigurationFFT> fft {new ConfigurationFFT(class_id, instance_id, *this)};
        loginf << "FFTManager: generateSubConfigurable: adding config fft "
               << fft->name();

        assert (!hasConfigFFT(fft->name()));
        config_ffts_.emplace_back(std::move(fft));
    }
    else
        throw std::runtime_error("FFTManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool FFTManager::hasConfigFFT (const std::string& name)
{
    return find_if(config_ffts_.begin(), config_ffts_.end(),
                   [name] (const std::unique_ptr<ConfigurationFFT>& s)
    { return s->name() == name; } ) != config_ffts_.end();
}

void FFTManager::createConfigFFT(const std::string& name)
{
    assert (!hasConfigFFT(name));

    auto new_cfg = Configuration::create("ConfigurationFFT");

    new_cfg->addParameter<std::string>("name", name);

    generateSubConfigurableFromConfig(std::move(new_cfg));

    updateFFTNamesAll();
}

void FFTManager::deleteFFT(const std::string& name)
{
    loginf << "FFTManager: deleteFFT: name " << name;

    if (hasConfigFFT(name))
    {

        auto ds_it = find_if(config_ffts_.begin(), config_ffts_.end(),
                             [name] (const std::unique_ptr<ConfigurationFFT>& s)
        { return s->name() == name; } );

        config_ffts_.erase(ds_it);
    }

    if (hasDBFFT(name))
    {

        auto ds_it = find_if(db_ffts_.begin(), db_ffts_.end(),
                             [name] (const std::unique_ptr<DBFFT>& s)
        { return s->name() == name; } );

        db_ffts_.erase(ds_it);
    }

    updateFFTNamesAll();

    emit fftsChangedSignal();
}

ConfigurationFFT& FFTManager::configFFT (const std::string& name)
{
    assert (hasConfigFFT(name));

    return *find_if(config_ffts_.begin(), config_ffts_.end(),
                    [name] (const std::unique_ptr<ConfigurationFFT>& s)
    { return s->name() == name; } )->get();
}

const std::vector<std::unique_ptr<ConfigurationFFT>>& FFTManager::configFFTs() const
{
    return config_ffts_;
}

void FFTManager::deleteAllFFTs()
{
    config_ffts_.clear();
    db_ffts_.clear();

    saveDBFFTs();

    updateFFTNamesAll();

    emit fftsChangedSignal();
}

bool FFTManager::hasDBFFT(const string& name)
{
    return find_if(db_ffts_.begin(), db_ffts_.end(),
                   [name] (const std::unique_ptr<DBFFT>& s)
    { return s->name() == name; } ) != db_ffts_.end();
}



DBFFT& FFTManager::dbFFT(const std::string& name)
{
    assert (hasDBFFT(name));

    return *find_if(db_ffts_.begin(), db_ffts_.end(),
                    [name] (const std::unique_ptr<DBFFT>& s)
    { return s->name() == name; } )->get();
}

bool FFTManager::canAddNewFFTFromConfig (const std::string& name)
{
    if (hasDBFFT(name))
        return false;

    return hasConfigFFT(name);
}

void FFTManager::addNewFFT (const std::string& name, bool emit_signal)
{
    logdbg << "FFTManager: addNewFFT: name " << name;

    assert (!hasDBFFT(name));

    if (hasConfigFFT(name))
    {
        logdbg << "FFTManager: addNewFFT: name " << name << " from config";

        ConfigurationFFT& cfg_fft = configFFT(name);

        db_ffts_.emplace_back(std::move(cfg_fft.getAsNewDBFFT()));

    }
    else
    {
        logdbg << "FFTManager: addNewFFT: name " << name << " create new";

        createConfigFFT(name);

        DBFFT* new_fft = new DBFFT();
        new_fft->name(name);

        db_ffts_.emplace_back(std::move(new_fft));
    }

    sortDBFFTs();
    assert (hasDBFFT(name));
    updateFFTNamesAll();

    if (emit_signal)
        emit fftsChangedSignal();

    loginf << "FFTManager: addNewFFT: name " << name << " done";
}

void FFTManager::addNewFFT (const std::string& name, nlohmann::json info, bool emit_signal)
{
    loginf << "FFTManager: addNewFFT: name " << name << " with info";

    assert (!hasConfigFFT(name));
    assert (!hasDBFFT(name));

    createConfigFFT(name);
    ConfigurationFFT& cfg_fft = configFFT(name);
    cfg_fft.info(info);

    assert (hasConfigFFT(name));
    assert (canAddNewFFTFromConfig(name));

    db_ffts_.emplace_back(std::move(cfg_fft.getAsNewDBFFT()));

    sortDBFFTs();

    if (emit_signal)
        emit fftsChangedSignal();
}

const std::vector<std::unique_ptr<DBFFT>>& FFTManager::dbFFTs() const
{
    return db_ffts_;
}


const std::vector<string>& FFTManager::getAllFFTNames() // both config and db
{
    return fft_names_all_;
}

void FFTManager::checkSubConfigurables() {}

void FFTManager::databaseOpenedSlot()
{
    loginf << "FFTManager: databaseOpenedSlot";

    loadDBFFTs();

    emit fftsChangedSignal();

    //    if (load_widget_)
    //        load_widget_->updateContent();
}

void FFTManager::databaseClosedSlot()
{
    db_ffts_.clear();

    updateFFTNamesAll();

    emit fftsChangedSignal();

    //    if (load_widget_)
    //        load_widget_->updateContent();
}


void FFTManager::configurationDialogDoneSlot()
{
    loginf << "FFTManager: configurationDialogDoneSlot";

    config_dialog_->hide();
    config_dialog_ = nullptr;

    emit fftsChangedSignal();
}

void FFTManager::saveDBFFTs()
{
    loginf << "FFTManager: saveDBFFTs";

    DBInterface& db_interface = COMPASS::instance().interface();

    assert(db_interface.dbOpen());
    db_interface.saveFFTs(db_ffts_);
}

void FFTManager::exportFFTs(const std::string& filename)
{
    loginf << "FFTManager: exportFFTs: filename '" << filename << "'";

    json data;

    data["content_type"] = "ffts";
    data["content_version"] = "0.1";

    data["ffts"] = json::array();
    json& ffts = data.at("ffts");

    unsigned int cnt = 0;

    for (auto& ds_it : db_ffts_)
    {
        ffts[cnt] = ds_it->getAsJSON();
        ++cnt;
    }

    std::ofstream file(filename);
    file << data.dump(4);

    QMessageBox m_info(QMessageBox::Information, "Export FFTs",
                       "File export: '"+QString(filename.c_str())+"' done.\n"
                       +QString::number(fft_names_all_.size())+" FFTs saved.", QMessageBox::Ok);
    m_info.exec();
}

void FFTManager::importFFTs(const std::string& filename)
{
    loginf << "FFTManager: importFFTs: filename '" << filename << "'";

    try
    {
        if (!Files::fileExists(filename))
            throw std::runtime_error ("File '"+filename+"' not found.");

        std::ifstream input_file(filename, std::ifstream::in);

        json j = json::parse(input_file);

        if (!j.contains("content_type")
                || !j.at("content_type").is_string()
                || j.at("content_type") != "ffts")
            throw std::runtime_error("current data is not fft content");

        if (!j.contains("content_version")
                || !j.at("content_version").is_string()
                || j.at("content_version") != "0.1")
            throw std::runtime_error("current data content version is not supported");

        if (!j.contains("ffts")
                || !j.at("ffts").is_array())
            throw std::runtime_error("current data contains no ffts");

        for (auto& j_fft_it : j.at("ffts").get<json::array_t>())
        {
            assert(j_fft_it.contains("name"));
            assert(j_fft_it.contains("info"));

            string name = j_fft_it.at("name");

            addNewFFT(name, j_fft_it.at("info"), false);
        }
    }
    catch (json::exception& e)
    {
        logerr << "FFTManager: importFFTs: could not load file '"
               << filename << "', exception '" << e.what() << "'";
        throw e;
    }

    saveDBFFTs();

    updateFFTNamesAll();

    emit fftsChangedSignal();
}

FFTsConfigurationDialog* FFTManager::configurationDialog()
{
    if (!config_dialog_)
    {
        config_dialog_.reset(new FFTsConfigurationDialog(*this));

        connect (config_dialog_.get(), &FFTsConfigurationDialog::doneSignal,
                 this, &FFTManager::configurationDialogDoneSlot);
    }

    return config_dialog_.get();
}

std::pair<bool, float> FFTManager::isFromFFT(
        double prelim_latitute_deg, double prelim_longitude_deg,
        boost::optional<unsigned int> mode_s_address, bool ignore_mode_s,
        boost::optional<unsigned int> mode_a_code,
        boost::optional<float> mode_c_code)
{

    bool check_passed;

    for (const auto& fft_it : db_ffts_)
    {
        check_passed = true;

        if (!ignore_mode_s && fft_it->hasModeSAddress())
        {
            if (!mode_s_address.has_value() || mode_s_address.value() != fft_it->modeSAddress())
                check_passed = false;
        }

        if (check_passed && fft_it->hasMode3ACode())
        {
            if (!mode_a_code.has_value() || mode_a_code.value() != fft_it->mode3ACode())
                check_passed = false;
        }

        if (check_passed && fft_it->hasModeCCode())
        {
            if (!mode_c_code.has_value() || mode_c_code.value() != fft_it->modeCCode())
                check_passed = false;
        }

        if (check_passed && fft_it->hasPosition())
        {
            double distance_m = osgEarth::GeoMath::distance(
                prelim_latitute_deg * DEG2RAD, prelim_longitude_deg * DEG2RAD,
                    fft_it->latitude() * DEG2RAD, fft_it->longitude() * DEG2RAD);

            check_passed = distance_m <= max_fft_plot_distance_m_;
        }

        if (check_passed)
        {
            if (fft_it->hasAltitude())
                return {true, fft_it->altitude() / FT2M}; // meters to ft
            else
                return {true, 0}; // no altitude info
        }
    }

    // unknown fft

    if (mode_a_code.has_value() && mode_a_code.value() == 4095) // 7777
        return {true, 0}; // is fft, but unknown altitude, better to correct with 0 alt

    return {false, 0}; // no matches, not altitude info
}

void FFTManager::loadDBFFTs()
{
    assert (!db_ffts_.size());

    DBInterface& db_interface = COMPASS::instance().interface();

    // load from db
    if (db_interface.existsFFTsTable())
        db_ffts_ = db_interface.getFFTs();
    else
        db_interface.createFFTsTable();

    bool new_created = false;

    // create from config into db ones
    for (const auto& cfg_fft_it : config_ffts_)
    {
        if (!hasDBFFT(cfg_fft_it->name()) && canAddNewFFTFromConfig(cfg_fft_it->name()))
        {
            loginf << "FFTManager: loadDBFFTs: creating db fft '" << cfg_fft_it->name() << "'";

            addNewFFT(cfg_fft_it->name(), false); // creates from config if possible
            new_created = true;
        }
    }

    if (new_created)
        saveDBFFTs(); // save if new ones were created

    // create db into config ones if missing
    for (const auto& db_fft_it : db_ffts_)
    {
        string fft_name = db_fft_it->name();
        if (!hasConfigFFT(fft_name)) // create
        {
            loginf << "FFTManager: loadDBFFTs: creating cfg fft '" << fft_name << "'";

            createConfigFFT(fft_name);
            assert (hasConfigFFT(fft_name));
            configFFT(fft_name).info(db_fft_it->info());
        }
    }

    updateFFTNamesAll();
    sortDBFFTs();

    emit fftsChangedSignal();
}

void FFTManager::sortDBFFTs()
{
    sort(db_ffts_.begin(), db_ffts_.end(),
         [](const std::unique_ptr<DBFFT>& a, const std::unique_ptr<DBFFT>& b) -> bool
    {
        return a->name() < b->name();
    });
}

void FFTManager::updateFFTNamesAll()
{
    fft_names_all_.clear();

    set<string> fft_names_set;

    for (auto& fft_it : config_ffts_) // add from cfg
    {
        if (!fft_names_set.count(fft_it->name()))
            fft_names_set.insert(fft_it->name());
    }

    for (auto& fft_it : db_ffts_) // add from db
    {
        if (!fft_names_set.count(fft_it->name()))
            fft_names_set.insert(fft_it->name());
    }

    std::copy(fft_names_set.begin(), fft_names_set.end(), std::back_inserter(fft_names_all_)); // copy to vec
}
