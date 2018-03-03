/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <sstream>

#include "buffercsvexportjob.h"


BufferCSVExportJob::BufferCSVExportJob(std::shared_ptr<Buffer> buffer, const std::string& file_name, bool overwrite, bool use_presentation)
    : Job("BufferCSVExportJob"), buffer_(buffer), file_name_(file_name), overwrite_(overwrite),
      use_presentation_(use_presentation)
{
    assert (file_name_.size());
}

BufferCSVExportJob::~BufferCSVExportJob()
{

}

void BufferCSVExportJob::run ()
{
    logdbg << "BufferCSVExportJob: execute: start";
    started_ = true;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    std::ofstream output_file;

    if (overwrite_)
        output_file.open(file_name_, std::ios_base::out);
    else
        output_file.open(file_name_, std::ios_base::app);

    if (output_file)
    {
        const PropertyList &properties = buffer_->properties();
        size_t properties_size = properties.size();
        size_t buffer_size = buffer_->size();
        PropertyDataType data_type;
        std::stringstream ss;
        bool null;
        std::string value_str;
        size_t row=0;

        for (size_t col=0; col < properties_size; col++)
        {
            if (col != 0)
                ss << ";";

            ss << properties.at(col).name();
        }
        output_file << ss.str() << "\n";


        for (; row < buffer_size; row++)
        {
            ss.str("");

            for (size_t col=0; col < properties_size; col++)
            {
                data_type = properties.at(col).dataType();

                if (data_type == PropertyDataType::BOOL)
                {
                    null = buffer_->getBool(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
                        // TODO
//                        if (use_presentation_)
//                            value_str = buffer_->getBool(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getBool(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::CHAR)
                {
                    null = buffer_->getChar(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getChar(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getChar(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::UCHAR)
                {
                    null = buffer_->getUChar(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getUChar(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getUChar(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::INT)
                {
                    null = buffer_->getInt(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getInt(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getInt(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::UINT)
                {
                    null = buffer_->getUInt(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getUInt(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getUInt(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::LONGINT)
                {
                    null = buffer_->getLongInt(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getLongInt(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getLongInt(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::ULONGINT)
                {
                    null = buffer_->getULongInt(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getULongInt(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getULongInt(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::FLOAT)
                {
                    null = buffer_->getFloat(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getFloat(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getFloat(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::DOUBLE)
                {
                    null = buffer_->getDouble(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
//                        if (use_presentation_)
//                            value_str = buffer_->getDouble(properties.at(col).name()).getAsRepresentationString(row);
//                        else
                            value_str = buffer_->getDouble(properties.at(col).name()).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::STRING)
                {
                    null = buffer_->getString(properties.at(col).name()).isNone(row);
                    if (!null)
                    {
                        value_str = buffer_->getString(properties.at(col).name()).getAsString(row);
                    }
                }
                else
                    throw std::domain_error ("BufferCSVExportJob: run: unknown property data type");

                if (col != 0)
                    ss << ";";

                ss << value_str;
            }

            output_file << ss.str() << "\n";
        }

        stop_time_ = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        if (diff.total_seconds() > 0)
            loginf  << "BufferCSVExportJob: run: done after " << diff << ", " << 1000.0*row/diff.total_milliseconds() << " el/s";
    }
    else
    {
        logerr << "BufferCSVExportJob: runFailure opening " << file_name_;
    }

    done_=true;

    logdbg << "BufferCSVExportJob: execute: done";
    return;
}
