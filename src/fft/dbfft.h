#pragma once

#include "fft/fftbase.h"
#include "property.h"

class DBFFT : public FFTBase
{
public:

    static const std::string table_name_;

    const static Property name_column_;
    const static Property info_column_;

    DBFFT();
    virtual ~DBFFT();

protected:
};

