#include "buffercsvexportjob.h"

BufferCSVExportJob::BufferCSVExportJob(std::shared_ptr<Buffer> buffer, const std::string& file_name, bool overwrite)
    : buffer_(buffer), file_name_(file_name), overwrite_(overwrite)
{

}

BufferCSVExportJob::~BufferCSVExportJob()
{

}

void BufferCSVExportJob::run ()
{
    logdbg << "BufferCSVExportJob: execute: start";
    started_ = true;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    // do stuff
    unsigned int row_count = 1000;

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    if (diff.total_seconds() > 0)
        loginf  << "BufferCSVExportJob: run: done after " << diff << ", " << 1000.0*row_count/diff.total_milliseconds() << " el/s";

    done_=true;

    logdbg << "BufferCSVExportJob: execute: done";
    return;
}
