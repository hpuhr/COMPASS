#include "fft/dbfft.h"
#include "logger.h"

using namespace std;


const std::string DBFFT::table_name_{"ffts"};

const Property DBFFT::name_column_{"name", PropertyDataType::STRING};
const Property DBFFT::info_column_{"info", PropertyDataType::STRING};

DBFFT::DBFFT()
{
}

DBFFT::~DBFFT()
{

}

