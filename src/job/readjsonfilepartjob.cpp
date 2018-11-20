#include "readjsonfilepartjob.h"
#include "stringconv.h"
#include "logger.h"

#include <archive.h>
#include <archive_entry.h>

using namespace Utils;

ReadJSONFilePartJob::ReadJSONFilePartJob(const std::string& file_name, bool archive, unsigned int num_objects)
    : Job("ReadJSONFilePartJob"), file_name_(file_name), archive_(archive), num_objects_(num_objects)
{

}
ReadJSONFilePartJob::~ReadJSONFilePartJob()
{
    if (archive_)
    {
        int r = archive_read_close(a);
        if (r != ARCHIVE_OK)
            logerr << "JSONImporterTask: importFileArchive: archive read close error: " << std::string(archive_error_string(a));
        else
        {

        r = archive_read_free(a);

        if (r != ARCHIVE_OK)
            logerr << "JSONImporterTask: importFileArchive: archive read free error: " << std::string(archive_error_string(a));
        }
    }
    else
        file_stream_.close();
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
        // if gz but not tar.gz or tgz
        bool raw = String::hasEnding (file_name_, ".gz") && !String::hasEnding (file_name_, ".tar.gz");

        loginf  << "JSONImporterTask: importFileArchive: importing " << file_name_ << " raw " << raw;

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
            throw std::runtime_error("JSONImporterTask: importFileArchive: archive error: "
                                     +std::string(archive_error_string(a)));
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
        const void *buff;
        size_t size;

    //    std::stringstream ss;

    //    QMessageBox msg_box;
    //    std::string msg = "Importing archive '"+filename+"'.";
    //    msg_box.setText(msg.c_str());
    //    msg_box.setStandardButtons(QMessageBox::NoButton);
    //    msg_box.show();


        //unsigned int entry_cnt = 0;
        int r;

        while (entry_not_done_ || archive_read_next_header(a, &entry) == ARCHIVE_OK)
        {
            //        for (unsigned int cnt=0; cnt < 10; cnt++)
            //            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            loginf << "JSONImporterTask: importFileArchive: parsing archive file: "
                   << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);

//            msg = "Reading archive entry " + std::to_string(entry_cnt) + ": "
//                    + std::string(archive_entry_pathname(entry)) + ".\n";
//            if (all_cnt_)
//                msg +=  + "# of updates: " + std::to_string(all_cnt_)
//                        + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
//                        + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
//                        + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
//                        + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";

//            msg_box.setInformativeText(msg.c_str());
//            msg_box.show();

            //        for (unsigned int cnt=0; cnt < 50; ++cnt)
            //            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);


            unsigned int parsed_objects {0};
//            boost::posix_time::ptime entry_start_time = boost::posix_time::microsec_clock::local_time();

//            boost::posix_time::ptime tmp_time = boost::posix_time::microsec_clock::local_time();

//            double read_time = 0;
//            double nloh_parse_time = 0;
//            double my_parse_time = 0;
//            double insert_time = 0;

            for (;;)
            {
                r = archive_read_data_block(a, &buff, &size, &offset);

                if (r == ARCHIVE_EOF)
                    break;
                if (r != ARCHIVE_OK)
                    throw std::runtime_error("JSONImporterTask: importFileArchive: archive error: "
                                             +std::string(archive_error_string(a)));

                std::string str (reinterpret_cast<char const*>(buff), size);

                for (char c : str)
                {
                    if (c == '{')
                        ++open_count_;
                    else if (c == '}')
                        --open_count_;

                    tmp_stream_ << c;

                    if (c == '\n') // next lines after objects
                        continue;

                    if (open_count_ == 0)
                    {
                        objects_.push_back(tmp_stream_.str());
                        tmp_stream_.str("");
                        ++parsed_objects;
                    }

                }

                if (parsed_objects > num_objects_) // parsed buffer, reached obj limit
                {
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

        unsigned int parsed_objects = 0;

        while (file_stream_.get(c) && parsed_objects < num_objects_) // loop getting single characters
        {
            if (c == '{')
                ++open_count_;
            else if (c == '}')
                --open_count_;

            tmp_stream_ << c;

            if (c == '\n') // next lines after objects
                continue;

            if (open_count_ == 0)
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

        assert (open_count_ == 0); // nothing left open
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



