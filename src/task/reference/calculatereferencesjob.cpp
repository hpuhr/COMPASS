#include "calculatereferencesjob.h"
#include "calculatereferencestask.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "stringconv.h"

#include "util/tbbhack.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace boost::posix_time;

CalculateReferencesJob::CalculateReferencesJob(CalculateReferencesTask& task, std::shared_ptr<dbContent::Cache> cache)
    : Job("CalculateReferencesJob"), task_(task), cache_(cache)
{

}

CalculateReferencesJob::~CalculateReferencesJob()
{

}

void CalculateReferencesJob::run()
{
    logdbg << "CalculateReferencesJob: run: start";

    started_ = true;

    ptime start_time;
    ptime stop_time;

    start_time = microsec_clock::local_time();

    emit statusSignal("Doing stuff");

    stop_time = microsec_clock::local_time();

    double load_time;
    time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CalculateReferencesJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";

    done_ = true;
}
