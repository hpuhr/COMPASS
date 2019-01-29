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

    size_t bytesRead() const;
    size_t bytesToRead() const;

    float getStatusPercent ();

protected:
    std::string file_name_;
    bool archive_ {false};
    unsigned int num_objects_ {0};

    bool file_read_done_ {false};
    bool init_performed_ {false};

    std::ifstream file_stream_;
    std::stringstream tmp_stream_;

    unsigned int open_count_ {0};

    struct archive *a;
    struct archive_entry *entry;
    int64_t offset;
    bool entry_done_ {true}; // init to done to trigger read of next header

    size_t bytes_to_read_ {0};
    size_t bytes_read_ {0};
    size_t bytes_read_tmp_ {0};
    std::vector<std::string> objects_;

    void performInit ();
    void readFilePart ();

    void openArchive (bool raw);
    void closeArchive ();

    void cleanCommas ();
};

#endif // READJSONFILEPARTJOB_H
