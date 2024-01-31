#ifndef FFTMANAGER_H
#define FFTMANAGER_H

#include "configurable.h"

#include "json.hpp"

#include <QObject>

#include <set>
#include <vector>
#include <memory>

class COMPASS;
class ConfigurationFFT;
class FFTsConfigurationDialog;
class DBFFT;

class FFTManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void fftsChangedSignal();

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

    void configurationDialogDoneSlot();

public:
    FFTManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~FFTManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasConfigFFT(const std::string& name);
    void createConfigFFT(const std::string& name);
    ConfigurationFFT& configFFT(const std::string& name);
    const std::vector<std::unique_ptr<ConfigurationFFT>>& configFFTs() const;
    void deleteAllConfigFFTs();

    bool hasDBFFT(const std::string& name);
    DBFFT& dbFFT(const std::string& name);
    bool canAddNewFFTFromConfig (const std::string& name);
    void addNewFFT (const std::string& name); // be sure not to call from different thread
    const std::vector<std::unique_ptr<DBFFT>>& dbFFTs() const;

    void deleteFFT(const std::string& name);

    const std::vector<std::string>& getAllFFTNames(); // both config and db

    void saveDBFFTs();

    FFTsConfigurationDialog* configurationDialog();

protected:
    COMPASS& compass_;

    std::vector<std::unique_ptr<ConfigurationFFT>> config_ffts_;
    std::vector<std::unique_ptr<DBFFT>> db_ffts_;

    std::vector<std::string> fft_names_all_; // both from config and db, vector to have order

    std::unique_ptr<FFTsConfigurationDialog> config_dialog_;

    virtual void checkSubConfigurables();

    void loadDBFFTs();
    void sortDBFFTs();

    void updateFFTNamesAll();
};

#endif // FFTMANAGER_H
