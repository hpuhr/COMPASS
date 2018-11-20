#include "readjsonfilepartjob.h"

#include "logger.h"

ReadJSONFilePartJob::ReadJSONFilePartJob(const std::string& file_name, bool archive, unsigned int num_objects)
    : Job("ReadJSONFilePartJob"), file_name_(file_name), archive_(archive), num_objects_(num_objects)
{

}
ReadJSONFilePartJob::~ReadJSONFilePartJob()
{

}

void ReadJSONFilePartJob::run ()
{
    loginf << "ReadJSONFilePartJob: run: start";
    started_ = true;

    assert (!done_);
    assert (!file_read_done_);
    assert (!objects_.size());

    if (!init_performed_)
    {
        performInit();
        assert (init_performed_);
    }

    while (!file_read_done_ && objects_.size() < num_objects_)
        readFilePart();

    done_=true;

    loginf << "ReadJSONFilePartJob: run: done";
    return;
}

void ReadJSONFilePartJob::performInit ()
{
    assert (!init_performed_);

    if (archive_)
    {
        assert (false); // TODO
    }
    else
    {
        file_stream_.open(file_name_);
    }

    init_performed_ = true;
}

void ReadJSONFilePartJob::readFilePart ()
{
    if (archive_)
    {
        assert (false); // TODO
    }
    else
    {
        char c;
        unsigned int open_count {0};

        unsigned int parsed_objects = 0;

        while (file_stream_.get(c) && parsed_objects < num_objects_) // loop getting single characters
        {
            if (c == '{')
                ++open_count;
            else if (c == '}')
                --open_count;

            tmp_stream_ << c;

            if (c == '\n') // next lines after objects
                continue;

            if (open_count == 0)
            {
                //loginf << "ReadJSONFilePartJob: readFilePart: obj '" << tmp_stream_.str() << "'";
                objects_.push_back(tmp_stream_.str());
                tmp_stream_.str("");
                ++parsed_objects;
            }
        }

        if (parsed_objects != num_objects_)
            file_read_done_ = true;

        loginf << "ReadJSONFilePartJob: readFilePart: parsed " << parsed_objects << " done " << file_read_done_;

        assert (open_count == 0); // nothing left open
        assert (tmp_stream_.str().size() == 0 || tmp_stream_.str() == "\n");
    }
}

void ReadJSONFilePartJob::resetDone ()
{
    assert (!file_read_done_);
    assert (!objects_.size());
    done_ = false; // yet another part
}

bool ReadJSONFilePartJob::fileReadDone() const
{
    return file_read_done_;
}

std::vector<std::string>& ReadJSONFilePartJob::objects()
{
    return objects_;
}



