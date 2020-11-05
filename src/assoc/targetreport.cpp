#include "assoc/targetreport.h"
#include "assoc/target.h"

namespace Association
{

TargetReport::TargetReport(string dbo_name, unsigned int rec_num, unsigned int ds_id, float tod)
    : dbo_name_(dbo_name), rec_num_(rec_num), ds_id_(ds_id), tod_(tod)
{
}

}
