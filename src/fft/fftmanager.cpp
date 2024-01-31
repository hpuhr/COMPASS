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

using namespace std;

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

void FFTManager::deleteAllConfigFFTs()
{
    config_ffts_.clear();

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

void FFTManager::addNewFFT (const std::string& name)
{
    loginf << "FFTManager: addNewFFT: name " << name;

    assert (!hasDBFFT(name));

    if (hasConfigFFT(name))
    {
        loginf << "FFTManager: addNewFFT: name " << name << " from config";

        ConfigurationFFT& cfg_ds = configFFT(name);

        db_ffts_.emplace_back(std::move(cfg_ds.getAsNewDBDS()));
        sortDBFFTs();
    }
    else
    {
        loginf << "FFTManager: addNewFFT: name " << name << " create new";

        createConfigFFT(name);

        DBFFT* new_ds = new DBFFT();
        new_ds->name(name);

        db_ffts_.emplace_back(std::move(new_ds));

        sortDBFFTs();
    }

    assert (hasDBFFT(name));
    updateFFTNamesAll();

    emit fftsChangedSignal();

    loginf << "FFTManager: addNewFFT: name " << name << " done";
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
        if (canAddNewFFTFromConfig(cfg_fft_it->name()))
        {
            addNewFFT(cfg_fft_it->name()); // creates from config if possible
            new_created = true;
        }
    }

    if (new_created)
        saveDBFFTs(); // save if new ones were created

    updateFFTNamesAll();
    sortDBFFTs();
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
