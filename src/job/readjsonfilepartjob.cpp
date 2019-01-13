#include "readjsonfilepartjob.h"
#include "stringconv.h"
#include "logger.h"

#include <archive.h>
#include <archive_entry.h>

#include <regex>

using namespace Utils;

ReadJSONFilePartJob::ReadJSONFilePartJob(const std::string& file_name, bool archive, unsigned int num_objects)
    : Job("ReadJSONFilePartJob"), file_name_(file_name), archive_(archive), num_objects_(num_objects)
{

}
ReadJSONFilePartJob::~ReadJSONFilePartJob()
{
    if (archive_)
        closeArchive();
    else
        file_stream_.close();
}

void ReadJSONFilePartJob::run ()
{
    logdbg << "ReadJSONFilePartJob: run: start";
    started_ = true;

    assert (!done_);
    assert (!file_read_done_);
    assert (!objects_.size());
    assert (!bytes_read_tmp_);

    if (!init_performed_)
    {
        performInit();
        assert (init_performed_);
    }

    //while (!file_read_done_ && objects_.size() < num_objects_)
    readFilePart();

    //cleanCommas ();

    done_=true;

    logdbg << "ReadJSONFilePartJob: run: done";
    return;
}

void ReadJSONFilePartJob::performInit ()
{
    assert (!init_performed_);

    if (archive_)
    {
        // if gz but not tar.gz or tgz
        bool raw = String::hasEnding (file_name_, ".gz") && !String::hasEnding (file_name_, ".tar.gz");

        loginf  << "ReadJSONFilePartJob: performInit: importing " << file_name_ << " raw " << raw;

        openArchive(raw);

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
            loginf << "ReadJSONFilePartJob: performInit: got "
                   << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);
            bytes_to_read_ += archive_entry_size(entry);
        }

        closeArchive();
        openArchive(raw); // slightly dirty

        loginf << "ReadJSONFilePartJob: performInit: archive size " << bytes_to_read_;
    }
    else
    {
        file_stream_.open(file_name_, std::ios::ate);
        bytes_to_read_ = file_stream_.tellg();
        loginf << "ReadJSONFilePartJob: performInit: non-archive size " << bytes_to_read_;
        file_stream_.seekg(0);
    }

    init_performed_ = true;
}

void ReadJSONFilePartJob::readFilePart ()
{
    loginf << "ReadJSONFilePartJob: readFilePart: begin";

    if (archive_)
    {
        const void *buff;
        size_t size;

        int r;
        bool closed_bracked = false;

        while (entry_not_done_ || archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
            loginf << "ReadJSONFilePartJob: readFilePart: parsing archive file: "
                   << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);

            for (;;)
            {
                r = archive_read_data_block(a, &buff, &size, &offset);

                if (r == ARCHIVE_EOF)
                {
                    entry_not_done_ = false;
                    break;
                }
                if (r != ARCHIVE_OK)
                    throw std::runtime_error("ReadJSONFilePartJob: readFilePart: archive error: "
                                             +std::string(archive_error_string(a)));

                std::string str (reinterpret_cast<char const*>(buff), size);

                for (char c : str)
                {
                    closed_bracked = false;

                    if (c == '{')
                        ++open_count_;
                    else if (c == '}')
                    {
                        --open_count_;
                        closed_bracked = true;
                    }

                    if (open_count_ || closed_bracked) // only add if enclosed by brackets
                        tmp_stream_ << c;

                    ++bytes_read_;
                    ++bytes_read_tmp_;

                    if (c == '\n') // next lines after objects
                        continue;

                    if (closed_bracked && open_count_ == 0)
                    {
                        objects_.push_back(tmp_stream_.str());
                        tmp_stream_.str("");
                    }

                }

                if (objects_.size() > num_objects_ || (objects_.size() && bytes_read_tmp_ > 1e7))
                    // parsed buffer, reached obj limit
                {
                    //|| (objects_.size() && bytes_read_tmp_ > 1e7)
                    //loginf << "UGA returning " << bytes_read_tmp_;
                    entry_not_done_ = true;
                    return;
                }
                else // parsed buffer, continue
                {
                    entry_not_done_ = false;
                }
            }
            if (!entry_not_done_) // will read next entry
            {
                assert (open_count_ == 0); // nothing left open
                assert (tmp_stream_.str().size() == 0 || tmp_stream_.str() == "\n");
            }
        }

        assert (open_count_ == 0); // nothing left open
        assert (tmp_stream_.str().size() == 0 || tmp_stream_.str() == "\n");

        file_read_done_ = true;
    }
    else
    {
        char c;
        bool closed_bracked = false;

        while (file_stream_.get(c) && objects_.size() < num_objects_) // loop getting single characters
        {
            closed_bracked = false;

            if (c == '{')
                ++open_count_;
            else if (c == '}')
            {
                --open_count_;
                closed_bracked = true;
            }

            if (open_count_ || closed_bracked) // only add if enclosed by brackets
                tmp_stream_ << c;

            ++bytes_read_;

            if (c == '\n') // next lines after objects
                continue;

            if (closed_bracked && open_count_ == 0)
            {
                //loginf << "ReadJSONFilePartJob: readFilePart: obj '" << tmp_stream_.str() << "'";
                objects_.push_back(tmp_stream_.str());
                tmp_stream_.str("");
            }
        }

        if (objects_.size() != num_objects_)
            file_read_done_ = true;

        loginf << "ReadJSONFilePartJob: readFilePart: parsed " << objects_.size() << " done " << file_read_done_;

        assert (open_count_ == 0); // nothing left open
        assert (tmp_stream_.str().size() == 0 || tmp_stream_.str() == "\n");
    }

    loginf << "ReadJSONFilePartJob: readFilePart: done";
}

void ReadJSONFilePartJob::resetDone ()
{
    assert (!file_read_done_);
    assert (!objects_.size());
    done_ = false; // yet another part
    bytes_read_tmp_ = 0;
}

bool ReadJSONFilePartJob::fileReadDone() const
{
    return file_read_done_;
}

std::vector<std::string>& ReadJSONFilePartJob::objects()
{
    return objects_;
}

size_t ReadJSONFilePartJob::bytesRead() const
{
    return bytes_read_;
}

size_t ReadJSONFilePartJob::bytesToRead() const
{
    return bytes_to_read_;
}

void ReadJSONFilePartJob::openArchive (bool raw)
{
    int r;

    a = archive_read_new();

    if (raw)
    {
        archive_read_support_filter_gzip(a);
        archive_read_support_filter_bzip2(a);
        archive_read_support_format_raw(a);
    }
    else
    {
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);

    }
    r = archive_read_open_filename(a, file_name_.c_str(), 10240); // Note 1

    if (r != ARCHIVE_OK)
        logerr << "JSONImporterTask: openArchive: archive open error: "
               << std::string(archive_error_string(a));
}
void ReadJSONFilePartJob::closeArchive ()
{
    int r = archive_read_close(a);
    if (r != ARCHIVE_OK)
        logerr << "JSONImporterTask: closeArchive: archive read close error: "
               << std::string(archive_error_string(a));
    else
    {
        r = archive_read_free(a);

        if (r != ARCHIVE_OK)
            logerr << "JSONImporterTask: closeArchive: archive read free error: "
                   << std::string(archive_error_string(a));
    }

    if (r != ARCHIVE_OK)
        throw std::runtime_error("ReadJSONFilePartJob: closeArchive: archive error: "
                                 +std::string(archive_error_string(a)));
}

float ReadJSONFilePartJob::getStatusPercent ()
{
    if (bytes_to_read_ == 0)
        return 0.0;
    else
        return 100.0*static_cast<double>(bytes_read_)/static_cast<double>(bytes_to_read_);
}

void ReadJSONFilePartJob::cleanCommas ()
{
    loginf << "ReadJSONFilePartJob: cleanCommas: " << objects_.size() << " objects";

    std::regex commas_between_brackets("\\[(,|\n)+\\]");
    std::regex multiple_commas(",\\n*,+");
    std::regex stupid_commas_at_bracket_begin("\\[\\n*,+");
    std::regex stupid_commas_at_bracket_end(",+\\n*\\]");

    for (auto& str_it : objects_)
    {
        str_it = std::regex_replace(str_it, commas_between_brackets, {"[]"});
        str_it = std::regex_replace(str_it, multiple_commas, {","});
        str_it = std::regex_replace(str_it, stupid_commas_at_bracket_begin, {"["});
        str_it = std::regex_replace(str_it, stupid_commas_at_bracket_end, {"]"});

        // \[(,|\n)+\]   commas between brackets
        // /,(?!["{}[\]])/g  multiple commas
    }
}
