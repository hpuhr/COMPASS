/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "allbuffercsvexportjob.h"

#include <fstream>
#include <sstream>

#include "atsdb.h"
#include "dboassociationcollection.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbovariableorderedset.h"
#include "metadbovariable.h"

AllBufferCSVExportJob::AllBufferCSVExportJob(
    std::map<std::string, std::shared_ptr<Buffer>> buffers, DBOVariableOrderedSet* read_set,
    std::map<unsigned int, std::string> number_to_dbo,
    const std::vector<std::pair<unsigned int, unsigned int>>& row_indexes,
    const std::string& file_name, bool overwrite, bool only_selected, bool use_presentation,
    bool show_associations)
    : Job("AllBufferCSVExportJob"),
      buffers_(buffers),
      read_set_(read_set),
      number_to_dbo_(number_to_dbo),
      row_indexes_(row_indexes),
      file_name_(file_name),
      overwrite_(overwrite),
      only_selected_(only_selected),
      use_presentation_(use_presentation),
      show_associations_(show_associations)
{
    assert(read_set_);
    assert(file_name_.size());
}

AllBufferCSVExportJob::~AllBufferCSVExportJob() {}

void AllBufferCSVExportJob::run()
{
    logdbg << "AllBufferCSVExportJob: execute: start";
    started_ = true;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    std::ofstream output_file;

    if (overwrite_)
        output_file.open(file_name_, std::ios_base::out);
    else
        output_file.open(file_name_, std::ios_base::app);

    if (output_file)
    {
        unsigned int dbo_num;
        unsigned int buffer_index;

        unsigned int read_set_size = read_set_->getSize();
        std::shared_ptr<Buffer> buffer;

        std::string dbo_name;
        std::string variable_dbo_name;
        std::string variable_name;

        std::stringstream ss;
        bool null;
        std::string value_str;

        // write the columns
        ss << "Selected;DBObject";

        if (show_associations_)
            ss << ";UTN";

        for (size_t col = 0; col < read_set_size; col++)
        {
            ss << ";" << read_set_->variableDefinition(col).variableName();
        }
        output_file << ss.str() << "\n";

        // write the data
        DBObjectManager& manager = ATSDB::instance().objectManager();

        for (auto& row_index_it : row_indexes_)
        {
            // set up everything to access the data
            dbo_num = row_index_it.first;
            buffer_index = row_index_it.second;

            assert(number_to_dbo_.count(dbo_num) == 1);
            dbo_name = number_to_dbo_.at(dbo_num);

            assert(buffers_.count(dbo_name) == 1);
            buffer = buffers_.at(dbo_name);

            assert(buffer_index < buffer->size());

            assert(buffer->has<bool>("selected"));
            NullableVector<bool>& selected_vec = buffer->get<bool>("selected");

            assert(buffer->has<int>("rec_num"));
            NullableVector<int>& rec_num_vec = buffer->get<int>("rec_num");

            // check if skipped because not selected
            if (only_selected_ &&
                (selected_vec.isNull(buffer_index) || !selected_vec.get(buffer_index)))
                continue;

            const PropertyList& properties = buffer->properties();
            ss.str("");

            // set selected flag
            if (selected_vec.isNull(buffer_index))
                ss << "0;";
            else
                ss << selected_vec.get(buffer_index) << ";";

            ss << dbo_name;  // set dboname

            if (show_associations_)
            {
                ss << ";";

                assert(!rec_num_vec.isNull(buffer_index));
                unsigned int rec_num = rec_num_vec.get(buffer_index);

                ss << manager.object(dbo_name).associations().getUTNsStringFor(rec_num);
            }

            for (unsigned int col = 0; col < read_set_size; ++col)
            {
                value_str = "";

                variable_dbo_name = read_set_->variableDefinition(col).dboName();
                variable_name = read_set_->variableDefinition(col).variableName();

                // check if data & variables exist
                if (variable_dbo_name == META_OBJECT_NAME)
                {
                    assert(manager.existsMetaVariable(variable_name));
                    if (!manager.metaVariable(variable_name)
                             .existsIn(dbo_name))  // not data if not exist
                    {
                        ss << ";";
                        continue;
                    }
                }
                else
                {
                    if (dbo_name != variable_dbo_name)  // check if other dbo
                    {
                        ss << ";";
                        continue;
                    }

                    assert(manager.existsObject(dbo_name));
                    assert(manager.object(dbo_name).hasVariable(variable_name));
                }

                DBOVariable& variable = (variable_dbo_name == META_OBJECT_NAME)
                                            ? manager.metaVariable(variable_name).getFor(dbo_name)
                                            : manager.object(dbo_name).variable(variable_name);

                PropertyDataType data_type = variable.dataType();

                std::string property_name = variable.name();

                if (data_type == PropertyDataType::BOOL)
                {
                    if (!buffer->has<bool>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<bool>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<bool>(property_name).getAsString(buffer_index));
                        else
                            value_str = buffer->get<bool>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::CHAR)
                {
                    if (!buffer->has<char>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<char>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<char>(property_name).getAsString(buffer_index));
                        else
                            value_str = buffer->get<char>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::UCHAR)
                {
                    if (!buffer->has<unsigned char>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<unsigned char>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<unsigned char>(property_name)
                                    .getAsString(buffer_index));
                        else
                            value_str =
                                buffer->get<unsigned char>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::INT)
                {
                    if (!buffer->has<int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<int>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<int>(property_name).getAsString(buffer_index));
                        else
                            value_str = buffer->get<int>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::UINT)
                {
                    if (!buffer->has<unsigned int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null =
                        buffer->get<unsigned int>(properties.at(col).name()).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<unsigned int>(property_name).getAsString(buffer_index));
                        else
                            value_str =
                                buffer->get<unsigned int>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::LONGINT)
                {
                    if (!buffer->has<long int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<long int>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<long int>(property_name).getAsString(buffer_index));
                        else
                            value_str =
                                buffer->get<long int>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::ULONGINT)
                {
                    if (!buffer->has<unsigned long int>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<unsigned long int>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<unsigned long int>(property_name)
                                    .getAsString(buffer_index));
                        else
                            value_str = buffer->get<unsigned long int>(property_name)
                                            .getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::FLOAT)
                {
                    if (!buffer->has<float>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<float>(properties.at(col).name()).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<float>(property_name).getAsString(buffer_index));
                        else
                            value_str = buffer->get<float>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::DOUBLE)
                {
                    if (!buffer->has<double>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<double>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        if (use_presentation_)
                            value_str = variable.getRepresentationStringFromValue(
                                buffer->get<double>(property_name).getAsString(buffer_index));
                        else
                            value_str =
                                buffer->get<double>(property_name).getAsString(buffer_index);
                    }
                }
                else if (data_type == PropertyDataType::STRING)
                {
                    if (!buffer->has<std::string>(property_name))
                    {
                        ss << ";";
                        continue;
                    }

                    null = buffer->get<std::string>(property_name).isNull(buffer_index);
                    if (!null)
                    {
                        value_str =
                            buffer->get<std::string>(property_name).getAsString(buffer_index);
                    }
                }
                else
                    throw std::domain_error(
                        "AllBufferCSVExportJob: run: unknown property data type");

                ss << ";";
                ss << value_str;
            }
            output_file << ss.str() << "\n";
        }

        //        assert (buffer_->has<bool>("selected"));
        //        NullableVector<bool> selected_vec = buffer_->get<bool>("selected");

        //        for (; row < buffer_size; ++row)
        //        {
        //            if (only_selected_ && (selected_vec.isNull(row) || !selected_vec.get(row)))
        //                continue;

        //            ss.str("");

        //            if (selected_vec.isNull(row))
        //                ss << "0;";
        //            else
        //                ss << selected_vec.get(row)<< ";";

        //            for (size_t col=0; col < read_set_size; col++)
        //            {
        //                value_str = "";

        //                DBOVariable& variable = read_set_.getVariable(col);
        //                PropertyDataType data_type = variable.dataType();

        //                std::string property_name = variable.name();

        //                if (data_type == PropertyDataType::BOOL)
        //                {
        //                    if (!buffer_->has<bool>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<bool>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<bool>(property_name).getAsString(row));
        //                        else
        //                            value_str =
        //                            buffer_->get<bool>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::CHAR)
        //                {
        //                    if (!buffer_->has<char>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<char>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<char>(property_name).getAsString(row));
        //                        else
        //                            value_str =
        //                            buffer_->get<char>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::UCHAR)
        //                {
        //                    if (!buffer_->has<unsigned char>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<unsigned char>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<unsigned
        //                                        char>(property_name).getAsString(row));
        //                        else
        //                            value_str = buffer_->get<unsigned
        //                            char>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::INT)
        //                {
        //                    if (!buffer_->has<int>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<int>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<int>(property_name).getAsString(row));
        //                        else
        //                            value_str = buffer_->get<int>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::UINT)
        //                {
        //                    if (!buffer_->has<unsigned int>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<unsigned
        //                    int>(properties.at(col).name()).isNull(row); if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<unsigned
        //                                        int>(property_name).getAsString(row));
        //                        else
        //                            value_str = buffer_->get<unsigned
        //                            int>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::LONGINT)
        //                {
        //                    if (!buffer_->has<long int>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<long int>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<long
        //                                        int>(property_name).getAsString(row));
        //                        else
        //                            value_str = buffer_->get<long
        //                            int>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::ULONGINT)
        //                {
        //                    if (!buffer_->has<unsigned long int>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<unsigned long int>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<unsigned long
        //                                        int>(property_name).getAsString(row));
        //                        else
        //                            value_str = buffer_->get<unsigned long
        //                            int>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::FLOAT)
        //                {
        //                    if (!buffer_->has<float>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<float>(properties.at(col).name()).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<float>(property_name).getAsString(row));
        //                        else
        //                            value_str =
        //                            buffer_->get<float>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::DOUBLE)
        //                {
        //                    if (!buffer_->has<double>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<double>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        if (use_presentation_)
        //                            value_str = variable.getRepresentationStringFromValue(
        //                                        buffer_->get<double>(property_name).getAsString(row));
        //                        else
        //                            value_str =
        //                            buffer_->get<double>(property_name).getAsString(row);
        //                    }
        //                }
        //                else if (data_type == PropertyDataType::STRING)
        //                {
        //                    if (!buffer_->has<std::string>(property_name))
        //                    {
        //                        ss << ";";
        //                        continue;
        //                    }

        //                    null = buffer_->get<std::string>(property_name).isNull(row);
        //                    if (!null)
        //                    {
        //                        value_str =
        //                        buffer_->get<std::string>(property_name).getAsString(row);
        //                    }
        //                }
        //                else
        //                    throw std::domain_error ("AllBufferCSVExportJob: run: unknown property
        //                    data type");

        //                ss << ";";
        //                ss << value_str;
        //            }

        //            output_file << ss.str() << "\n";
        //        }

        //        stop_time_ = boost::posix_time::microsec_clock::local_time();
        //        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        //        if (diff.total_seconds() > 0)
        //            loginf  << "AllBufferCSVExportJob: run: done after " << diff << ", " <<
        //            1000.0*row/diff.total_milliseconds() << " el/s";
    }
    else
    {
        logerr << "AllBufferCSVExportJob: runFailure opening " << file_name_;
    }

    done_ = true;

    logdbg << "AllBufferCSVExportJob: execute: done";
    return;
}
