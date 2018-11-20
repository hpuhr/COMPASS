#ifndef READJSONFILEPARTJOB_H
#define READJSONFILEPARTJOB_H

#include "job.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

class ReadJSONFilePartJob : public Job
{
public:
    ReadJSONFilePartJob(const std::string& file_name, bool archive, unsigned int num_objects);
    virtual ~ReadJSONFilePartJob();

    virtual void run ();

    void resetDone ();

    bool fileReadDone() const;

    std::vector<std::string>& objects(); // for moving out

protected:
    std::string file_name_;
    bool archive_ {false};
    unsigned int num_objects_ {0};

    bool file_read_done_ {false};
    bool init_performed_ {false};

    std::ifstream file_stream_;
    std::stringstream tmp_stream_;

    std::vector<std::string> objects_;

    void performInit ();
    void readFilePart ();

    //boost::posix_time::ptime start_time_;
    //boost::posix_time::ptime stop_time_;
};

#endif // READJSONFILEPARTJOB_H
