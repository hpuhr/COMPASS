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
            logerr << "BufferTableModel: data: variable " << variable.name() << " not present in buffer";
        }
        else
        {
            std::string property_name = variable.name();

            if (data_type == PropertyDataType::BOOL)
            {
                assert (buffer_->hasBool(property_name));
                null = buffer_->getBool(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getBool(property_name).getAsString(row));
                    else
                        value_str = buffer_->getBool(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::CHAR)
            {
                assert (buffer_->hasChar(property_name));
                null = buffer_->getChar(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getChar(property_name).getAsString(row));
                    else
                        value_str = buffer_->getChar(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::UCHAR)
            {
                assert (buffer_->hasUChar(property_name));
                null = buffer_->getUChar(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getUChar(property_name).getAsString(row));
                    else
                        value_str = buffer_->getUChar(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                assert (buffer_->hasInt(property_name));
                null = buffer_->getInt(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getInt(property_name).getAsString(row));
                    else
                        value_str = buffer_->getInt(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::UINT)
            {
                assert (buffer_->hasUInt(property_name));
                null = buffer_->getUInt(properties.at(col).name()).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getUInt(property_name).getAsString(row));
                    else
                        value_str = buffer_->getUInt(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::LONGINT)
            {
                assert (buffer_->hasLongInt(property_name));
                null = buffer_->getLongInt(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getLongInt(property_name).getAsString(row));
                    else
                        value_str = buffer_->getLongInt(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                assert (buffer_->hasULongInt(property_name));
                null = buffer_->getULongInt(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getULongInt(property_name).getAsString(row));
                    else
                        value_str = buffer_->getULongInt(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                assert (buffer_->hasFloat(property_name));
                null = buffer_->getFloat(properties.at(col).name()).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getFloat(property_name).getAsString(row));
                    else
                        value_str = buffer_->getFloat(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::DOUBLE)
            {
                assert (buffer_->hasDouble(property_name));
                null = buffer_->getDouble(property_name).isNone(row);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer_->getDouble(property_name).getAsString(row));
                    else
                        value_str = buffer_->getDouble(property_name).getAsString(row);
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                assert (buffer_->hasString(property_name));
                null = buffer_->getString(property_name).isNone(row);
                if (!null)
                {
                    value_str = buffer_->getString(property_name).getAsString(row);
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

QVariant BufferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        logdbg << "BufferTableModel: headerData: section " << section;
        const PropertyList &properties = buffer_->properties();
        unsigned int col = section;

        assert (col < properties.size());
        return QString (properties.at(col).name().c_str());
    }
    else if(orientation == Qt::Vertical)
        return section;

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

