#pragma once

#include "fft/fftbase.h"
#include "configurable.h"

class FFTManager;

class DBFFT;

class ConfigurationFFT : public Configurable, public FFTBase
{
public:
    ConfigurationFFT(const std::string& class_id, const std::string& instance_id,
                            FFTManager& ds_manager);
    virtual ~ConfigurationFFT();

    DBFFT* getAsNewDBDS();

protected:
    virtual void checkSubConfigurables() {}
};

