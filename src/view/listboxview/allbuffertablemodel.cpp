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

#include "allbuffertablemodel.h"

#include "buffer.h"
#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "buffercsvexportjob.h"
#include "jobmanager.h"
#include "global.h"
#include "dbovariableset.h"
#include "listboxview.h"
#include "listboxviewdatasource.h"
#include "allbuffertablewidget.h"

#include <QApplication>

AllBufferTableModel::AllBufferTableModel(AllBufferTableWidget* table_widget, ListBoxViewDataSource& data_source)
    : QAbstractTableModel(table_widget), table_widget_(table_widget), data_source_(data_source)
{
}

AllBufferTableModel::~AllBufferTableModel()
{
}

int AllBufferTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    logdbg << "AllBufferTableModel: rowCount: " << row_indexes_.size();
    return row_indexes_.size();
}

int AllBufferTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    logdbg << "AllBufferTableModel: columnCount: " << data_source_.getSet()->getSize();
    return data_source_.getSet()->getSize()+2;
}

QVariant AllBufferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        logdbg << "AllBufferTableModel: headerData: section " << section;
        unsigned int col = section;

        if (col == 0)
            return QString ();
        if (col == 1)
            return QString ("DBObject");

        col -= 2; // for the actual properties

        assert (col < data_source_.getSet()->getSize());
        std::string variable_name = data_source_.getSet()->variableDefinition(col).variableName();
        return QString (variable_name.c_str());
    }
    else if(orientation == Qt::Vertical)
        return section;

    return QVariant();
}

Qt::ItemFlags AllBufferTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags;

    if (index.column() == 0)
    {
        flags |= Qt::ItemIsEnabled;
        flags |= Qt::ItemIsUserCheckable;
        flags |= Qt::ItemIsEditable;
    }
    else
        return Qt::ItemIsEnabled;

    return flags;
}

QVariant AllBufferTableModel::data(const QModelIndex &index, int role) const
{
    logdbg << "AllBufferTableModel: data: row " << index.row()-1 << " col " << index.column()-1;

    bool null=false;

    assert (index.row() >= 0);
    assert ((unsigned int)index.row() < row_indexes_.size());
    unsigned int dbo_num = row_indexes_.at(index.row()).first;
    unsigned int bufferindex = row_indexes_.at(index.row()).second;
    unsigned int col = index.column();

    assert (number_to_dbo_.count(dbo_num) == 1);
    std::string dbo_name = number_to_dbo_.at(dbo_num);

    assert (buffers_.count(dbo_name) == 1);
    std::shared_ptr <Buffer> buffer = buffers_.at(dbo_name);

    if (role == Qt::CheckStateRole)
    {
        if (col == 0) // selected special case
        {
            assert (buffer->has<bool>("selected"));

            if (buffer->get<bool>("selected").isNull(bufferindex))
                return Qt::Unchecked;

            if (buffer->get<bool>("selected").get(bufferindex))
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }
    else if (role == Qt::DisplayRole)
    {
        assert (buffer);

        std::string value_str;

        const PropertyList &properties = buffer->properties();

        assert (bufferindex < buffer->size());

        if (col == 0) // selected special case
            return QVariant();
        if (col == 1) // selected special case
            return QVariant(dbo_name.c_str());

        col -= 2; // for the actual properties

        assert (col < data_source_.getSet()->getSize());

        std::string variable_dbo_name = data_source_.getSet()->variableDefinition(col).dboName();
        std::string variable_name = data_source_.getSet()->variableDefinition(col).variableName();

        DBObjectManager &manager = ATSDB::instance().objectManager();

         // check if data & variables exist
        if (variable_dbo_name == META_OBJECT_NAME)
        {
            assert (manager.existsMetaVariable(variable_name));
            if (!manager.metaVariable(variable_name).existsIn(dbo_name)) // not data if not exist
                return QString();
        }
        else
        {
            if (dbo_name != variable_dbo_name) // check if other dbo
                return QString();

            assert (manager.existsObject(dbo_name));
            assert (manager.object(dbo_name).hasVariable(variable_name));
        }

        //type_set.add (manager.metaVariable(it->second->variableName()).getFor(dbo_name));
        //bool exists_in_dbo = data_source_.getSet()->variableDefinition(col).

        //return QString (variable_name.c_str());

        //DBOVariableSet dbo_read_set = data_source_.getSet()->getFor(dbo_name);

        DBOVariable& variable = (variable_dbo_name == META_OBJECT_NAME)
                ? manager.metaVariable(variable_name).getFor(dbo_name) : manager.object(dbo_name).variable(variable_name);
        PropertyDataType data_type = variable.dataType();

        value_str = NULL_STRING;

        //const DBTableColumn &column = variable.currentDBColumn ();

        if (!properties.hasProperty(variable.name()))
        {
            logdbg << "AllBufferTableModel: data: variable " << variable.name() << " not present in buffer";
        }
        else
        {
            std::string property_name = variable.name();

            if (data_type == PropertyDataType::BOOL)
            {
                assert (buffer->has<bool>(property_name));
                null = buffer->get<bool>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<bool>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<bool>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::CHAR)
            {
                assert (buffer->has<char>(property_name));
                null = buffer->get<char>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<char>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<char>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::UCHAR)
            {
                assert (buffer->has<unsigned char>(property_name));
                null = buffer->get<unsigned char>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<unsigned char>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<unsigned char>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                assert (buffer->has<int>(property_name));
                null = buffer->get<int>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<int>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<int>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::UINT)
            {
                assert (buffer->has<unsigned int>(property_name));
                null = buffer->get<unsigned int>(properties.at(col).name()).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<unsigned int>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<unsigned int>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::LONGINT)
            {
                assert (buffer->has<long int>(property_name));
                null = buffer->get<long int>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<long int>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<long int>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                assert (buffer->has<unsigned long int>(property_name));
                null = buffer->get<unsigned long int>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<unsigned long int>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<unsigned long int>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                assert (buffer->has<float>(property_name));
                null = buffer->get<float>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<float>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<float>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::DOUBLE)
            {
                assert (buffer->has<double>(property_name));
                null = buffer->get<double>(property_name).isNull(bufferindex);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                                    buffer->get<double>(property_name).getAsString(bufferindex));
                    else
                        value_str = buffer->get<double>(property_name).getAsString(bufferindex);
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                assert (buffer->has<std::string>(property_name));
                null = buffer->get<std::string>(property_name).isNull(bufferindex);
                if (!null)
                {
                    value_str = buffer->get<std::string>(property_name).getAsString(bufferindex);
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

bool AllBufferTableModel::setData(const QModelIndex& index, const QVariant & value,int role)
{
    loginf << "AllBufferTableModel: setData: checked row " << index.row() << " col " << index.column();

//    if (role == Qt::CheckStateRole && index.column() == 0)
//    {
//        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

//        assert (row_to_index_.count(index.row()) == 1);
//        unsigned int bufferindex = row_to_index_.at(index.row());

//        assert (buffer);
//        assert (buffer->has<bool>("selected"));

//        if (value == Qt::Checked)
//        {
//            loginf << "AllBufferTableModel: setData: checked row index" << bufferindex;
//            buffer->get<bool>("selected").set(bufferindex, true);
//        }
//        else
//        {
//            loginf << "AllBufferTableModel: setData: unchecked row index " << bufferindex;
//            buffer->get<bool>("selected").set(bufferindex, false);
//        }
//        assert (table_widget_);
//        table_widget_->view().emitSelectionChange();

//        if (show_only_selected_)
//        {
//            beginResetModel();
//            row_to_index_.clear();
//            updateRows();
//            endResetModel();
//        }

//        QApplication::restoreOverrideCursor();
//    }
    return true;
}

void AllBufferTableModel::clearData ()
{
    beginResetModel();

//    buffer=nullptr;
//    updateRows();

    endResetModel();
}

void AllBufferTableModel::setData (std::shared_ptr <Buffer> buffer)
{
    assert (buffer);
    beginResetModel();

    std::string dbo_name = buffer->dboName();

    if (dbo_to_number_.count(dbo_name) == 0) // new dbo from the wild
    {
        unsigned int num = dbo_to_number_.size();
        number_to_dbo_[num] = dbo_name;
        dbo_to_number_[dbo_name] = num;
    }

    assert (dbo_to_number_.size() == number_to_dbo_.size());

    buffers_[dbo_name] = buffer;

    updateTimeIndexes();
    rebuildRowIndexes();

//    buffer = buffer;
//    updateRows();
//    read_set_ = data_source_.getSet()->getFor(object_.name());

    endResetModel();
}

void AllBufferTableModel::updateTimeIndexes ()
{
    loginf << "AllBufferTableModel: updateTimeIndexes";

    unsigned int processed_index;
    std::string dbo_name;
    unsigned int dbo_num;
    unsigned int buffersize;

    unsigned int num_time_none;
    float tod;

    for (auto& buf_it : buffers_)
    {
        processed_index = 0;
        dbo_name = buf_it.first;
        num_time_none = 0;

        assert (dbo_to_number_.count(dbo_name) == 1);
        dbo_num = dbo_to_number_.at(dbo_name);

        if (dbo_last_processed_index_.count(dbo_name) == 1)
            processed_index = dbo_last_processed_index_.at(dbo_name);

        buffersize = buf_it.second->size();

        if (buffersize > processed_index+1) // new data
        {
            loginf << "AllBufferTableModel: updateTimeIndexes: new " << dbo_name <<  " data, last index "
                   << processed_index << " size " << buf_it.second->size();

            DBObjectManager& object_manager = ATSDB::instance().objectManager();
            const DBOVariable &tod_var = object_manager.metaVariable("tod").getFor(dbo_name);
            assert (buf_it.second->has<float>(tod_var.name()));
            NullableVector<float> &tods = buf_it.second->get<float> (tod_var.name());

            for (unsigned int index=processed_index+1; index < buffersize; ++index)
            {
                if (tods.isNull(index))
                {
                    num_time_none++;
                    continue;
                }

                tod = tods.get(index);
                time_to_indexes_.insert(std::make_pair(tod, std::make_pair(dbo_num, index)));
            }

            dbo_last_processed_index_[dbo_name] = buffersize-1; // set to last index

            if (num_time_none)
                loginf << "AllBufferTableModel: updateTimeIndexes: new " << dbo_name << " skipped " << num_time_none
                       << " indexes with no time";
        }
    }
}

void AllBufferTableModel::rebuildRowIndexes ()
{
    row_indexes_.clear();

    for (auto& time_index_it : time_to_indexes_)
    {
        row_indexes_.push_back(time_index_it.second);
    }
}

void AllBufferTableModel::reset ()
{
    beginResetModel();
    endResetModel();
}

void AllBufferTableModel::saveAsCSV (const std::string &file_name, bool overwrite)
{
    loginf << "AllBufferTableModel: saveAsCSV: into filename " << file_name << " overwrite " << overwrite;

//    assert (buffer);
//    BufferCSVExportJob *export_job = new BufferCSVExportJob (buffer, read_set_, file_name, overwrite,
//                                                             use_presentation_);

//    export_job_ = std::shared_ptr<BufferCSVExportJob> (export_job);
//    connect (export_job, SIGNAL(obsoleteSignal()), this, SLOT(exportJobObsoleteSlot()), Qt::QueuedConnection);
//    connect (export_job, SIGNAL(doneSignal()), this, SLOT(exportJobDoneSlot()), Qt::QueuedConnection);

//    JobManager::instance().addBlockingJob(export_job_);
}

void AllBufferTableModel::exportJobObsoleteSlot ()
{
    logdbg << "AllBufferTableModel: exportJobObsoleteSlot";

    emit exportDoneSignal (true);
}

void AllBufferTableModel::exportJobDoneSlot()
{
    logdbg << "AllBufferTableModel: exportJobDoneSlot";

    emit exportDoneSignal (false);
}

void AllBufferTableModel::usePresentation (bool use_presentation)
{
    beginResetModel();
    use_presentation_ = use_presentation;
    endResetModel();
}

void AllBufferTableModel::showOnlySelected (bool value)
{
    loginf << "AllBufferTableModel: showOnlySelected: " << value;
    show_only_selected_ = value;

    updateToSelection();
}

void AllBufferTableModel::updateToSelection()
{
    beginResetModel();

//    row_to_index_.clear();
//    updateRows ();

    endResetModel();
}

