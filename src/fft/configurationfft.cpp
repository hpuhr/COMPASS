#include "fft/configurationfft.h"
#include "fft/dbfft.h"
#include "fftmanager.h"
#include "logger.h"

using namespace std;
using namespace nlohmann;


ConfigurationFFT::ConfigurationFFT(const std::string& class_id, const std::string& instance_id,
                                                 FFTManager& ds_manager)
    : Configurable(class_id, instance_id, &ds_manager)
{
    registerParameter("name", &name_, std::string());
    registerParameter("info", &info_, {});
}

ConfigurationFFT::~ConfigurationFFT()
{
}

DBFFT* ConfigurationFFT::getAsNewDBDS()
{
    DBFFT* new_fft = new DBFFT();
    new_fft->name(name_);

    if (!info_.is_null())
        new_fft->info(info_.dump());

    loginf << "ConfigurationFFT: getAsNewDBDS: name " << new_fft->name()
            << " info '" << new_fft->info().dump() << "'";

    return new_fft;
}

