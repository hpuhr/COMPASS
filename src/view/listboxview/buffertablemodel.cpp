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

#include "buffertablemodel.h"

#include "buffer.h"
#include "dbobject.h"
#include "buffercsvexportjob.h"
#include "jobmanager.h"
#include "global.h"
#include "dbovariableset.h"
#include "listboxviewdatasource.h"
#include "arraylist.h"
//#include "dbtablecolumn.h"

BufferTableModel::BufferTableModel(QObject *parent, DBObject &object, ListBoxViewDataSource& data_source)
    : QAbstractTableModel(parent), object_(object), data_source_(data_source)
{
}

BufferTableModel::~BufferTableModel()
{
    buffer_ = nullptr;
}

int BufferTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    if (buffer_)
    {
        logdbg << "BufferTableModel: rowCount: " << buffer_->size();
        return buffer_->size();
    }
    else
    {
        logdbg << "BufferTableModel: rowCount: 0";
        return 0;
    }
}

int BufferTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    logdbg << "BufferTableModel: columnCount: " << read_set_.getSize();
    return read_set_.getSize();

//    if (buffer_)
//    {
//        return buffer_->properties().size();
//    }
//    else
//        return 0;
}

QVariant BufferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        logdbg << "BufferTableModel: headerData: section " << section;
        unsigned int col = section;

//        const PropertyList &properties = buffer_->properties();
//        assert (col < properties.size());
//        return QString (properties.at(col).name().c_str());

        assert (col < read_set_.getSize());
        DBOVariable& variable = read_set_.getVariable(col);
        return QString (variable.name().c_str());
    }
    else if(orientation == Qt::Vertical)
        return section;

    return QVariant();
}

QVariant BufferTableModel::data(const QModelIndex &index, int role) const
{
    logdbg << "BufferTableModel: data: row " << index.row()-1 << " col " << index.column()-1;
    if (role == Qt::DisplayRole)
    {
        assert (buffer_);

        bool null=false;
        std::string value_str;

        unsigned int row = index.row(); // indexes start at 0 in this family
        unsigned int col = index.column();

        const PropertyList &properties = buffer_->properties();

        assert (row < buffer_->size());
        assert (col < read_set_.getSize());

        DBOVariable& variable = read_set_.getVariable(col);
        PropertyDataType data_type = variable.dataType();

        value_str = NULL_STRING;

        //const DBTableColumn &column = variable.currentDBColumn ();

        if (!properties.hasProperty(variable.name()))
        {
            logdbg << "BufferTableModel: data: variable " << variable.name() << " not present in buffer";
        }
        else
        {
            std::string property_name = variable.name();

            if (data_type == PropertyDataType::BOOL)
            {
                assert (buffer_->has<bool>(property_name));
                null = buffer_->get<bool>(property_name).isNone(row);
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
                assert (buffer_->has<char>(property_name));
                null = buffer_->get<char>(property_name).isNone(row);
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
                assert (buffer_->has<unsigned char>(property_name));
                null = buffer_->get<unsigned char>(property_name).isNone(row);
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
                assert (buffer_->has<int>(property_name));
                null = buffer_->get<int>(property_name).isNone(row);
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
                assert (buffer_->has<unsigned int>(property_name));
                null = buffer_->get<unsigned int>(properties.at(col).name()).isNone(row);
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
                assert (buffer_->has<long int>(property_name));
                null = buffer_->get<long int>(property_name).isNone(row);
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
                assert (buffer_->has<unsigned long int>(property_name));
                null = buffer_->get<unsigned long int>(property_name).isNone(row);
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
                assert (buffer_->has<float>(property_name));
                null = buffer_->get<float>(property_name).isNone(row);
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
                assert (buffer_->has<double>(property_name));
                null = buffer_->get<double>(property_name).isNone(row);
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
                assert (buffer_->has<std::string>(property_name));
                null = buffer_->get<std::string>(property_name).isNone(row);
                if (!null)
                {
                    value_str = buffer_->get<std::string>(property_name).getAsString(row);
                }
            }
            else
                throw std::domain_error ("BufferTableWidget: show: unknown property data type");

            if (null)
                return QVariant();
            else
                return QString (value_str.c_str());
        }
    }
    return QVariant();
}

void BufferTableModel::clearData ()
{
    beginResetModel();

    buffer_=nullptr;

    endResetModel();

}

void BufferTableModel::setData (std::shared_ptr <Buffer> buffer)
{
    assert (buffer);
    beginResetModel();

    buffer_=buffer;
    read_set_ = data_source_.getSet()->getFor(object_.name());

    endResetModel();
}

void BufferTableModel::saveAsCSV (const std::string &file_name, bool overwrite)
{
    loginf << "BufferTableModel: saveAsCSV: into filename " << file_name << " overwrite " << overwrite;

    assert (buffer_);
    BufferCSVExportJob *export_job = new BufferCSVExportJob (buffer_, read_set_, file_name, overwrite,
                                                             use_presentation_);

    export_job_ = std::shared_ptr<BufferCSVExportJob> (export_job);
    connect (export_job, SIGNAL(obsoleteSignal()), this, SLOT(exportJobObsoleteSlot()), Qt::QueuedConnection);
    connect (export_job, SIGNAL(doneSignal()), this, SLOT(exportJobDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(export_job_);
}

void BufferTableModel::exportJobObsoleteSlot ()
{
    logdbg << "BufferTableModel: exportJobObsoleteSlot";

    emit exportDoneSignal (true);
}

void BufferTableModel::exportJobDoneSlot()
{
    logdbg << "BufferTableModel: exportJobDoneSlot";

    emit exportDoneSignal (false);
}

void BufferTableModel::usePresentation (bool use_presentation)
{
    beginResetModel();
    use_presentation_=use_presentation;
    endResetModel();
}

