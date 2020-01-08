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
#include "dbovariable.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "atsdb.h"

BufferCSVExportJob::BufferCSVExportJob(std::shared_ptr<Buffer> buffer, const DBOVariableSet& read_set,
                                       const std::string& file_name, bool overwrite, bool only_selected,
                                       bool use_presentation, bool show_associations)
    : Job("BufferCSVExportJob"), buffer_(buffer), read_set_(read_set), file_name_(file_name), overwrite_(overwrite),
      only_selected_(only_selected), use_presentation_(use_presentation), show_associations_(show_associations)
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
        size_t read_set_size = read_set_.getSize();
        size_t buffer_size = buffer_->size();
        std::stringstream ss;
        bool null;
        std::string value_str;
        size_t row=0;

        ss << "Selected";

        if (show_associations_)
            ss << ";UTN";

        for (size_t col=0; col < read_set_size; col++)
        {
            //if (col != 0)
            ss << ";";
            ss << read_set_.getVariable(col).name();
        }
        output_file << ss.str() << "\n";

        assert (buffer_->has<bool>("selected"));
        NullableVector<bool> selected_vec = buffer_->get<bool>("selected");

        assert (buffer_->has<int>("rec_num"));
        NullableVector<int> rec_num_vec = buffer_->get<int>("rec_num");

        std::string dbo_name = buffer_->dboName();
        assert (dbo_name.size());

        DBObjectManager& manager = ATSDB::instance().objectManager();

        for (; row < buffer_size; ++row)
        {
            if (only_selected_ && (selected_vec.isNull(row) || !selected_vec.get(row)))
                continue;

            ss.str("");

            if (selected_vec.isNull(row))
                ss << "0";
            else
                ss << selected_vec.get(row);

            if (show_associations_)
            {
                ss << ";";

                assert (!rec_num_vec.isNull(row));
                unsigned int rec_num = rec_num_vec.get(row);

                std::vector<unsigned int> utns = manager.object(dbo_name).associations().getUTNSFor(rec_num);

                for (unsigned int cnt=0; cnt < utns.size(); ++cnt)
                {
                    if (cnt == 0)
                        ss << std::to_string(utns.at(cnt));
                    else
                        ss << "," << std::to_string(utns.at(cnt));
                }

//                typedef DBOAssociationCollection::const_iterator MMAPIterator;
//                const DBOAssociationCollection& associations = manager.object(dbo_name).associations();

//                std::pair<MMAPIterator, MMAPIterator> result = associations.equal_range(rec_num);

//                for (MMAPIterator it = result.first; it != result.second; it++)
//                    if (it == result.first)
//                        ss << std::to_string(it->second.utn_);
//                    else
//                        ss << "," << std::to_string(it->second.utn_);
            }

            for (size_t col=0; col < read_set_size; col++)
            {
                value_str = "";

                DBOVariable& variable = read_set_.getVariable(col);
                PropertyDataType data_type = variable.dataType();

                std::string property_name = variable.name();

                if (data_type == PropertyDataType::BOOL)
                {
                    if (!buffer_->has<bool>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<bool>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<bool>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<bool>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::CHAR)
                {
                    if (!buffer_->has<char>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<char>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<char>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<char>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::UCHAR)
                {
                    if (!buffer_->has<unsigned char>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<unsigned char>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<unsigned char>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<unsigned char>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::INT)
                {
                    if (!buffer_->has<int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<int>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<int>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<int>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::UINT)
                {
                    if (!buffer_->has<unsigned int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<unsigned int>(properties.at(col).name()).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<unsigned int>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<unsigned int>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::LONGINT)
                {
                    if (!buffer_->has<long int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<long int>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<long int>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<long int>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::ULONGINT)
                {
                    if (!buffer_->has<unsigned long int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<unsigned long int>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<unsigned long int>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<unsigned long int>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::FLOAT)
                {
                    if (!buffer_->has<float>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<float>(properties.at(col).name()).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<float>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<float>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::DOUBLE)
                {
                    if (!buffer_->has<double>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<double>(property_name).isNull(row);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                        buffer_->get<double>(property_name).getAsString(row));
                        else
                            value_str = buffer_->get<double>(property_name).getAsString(row);
                    }
                }
                else if (data_type == PropertyDataType::STRING)
                {
                    if (!buffer_->has<std::string>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer_->get<std::string>(property_name).isNull(row);
                    if (!null)
                    {
                        value_str = buffer_->get<std::string>(property_name).getAsString(row);
                    }
                }
                else
                    throw std::domain_error ("BufferCSVExportJob: run: unknown property data type");

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
