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

#include "allbuffertablemodel.h"

#include <QApplication>

#include "allbuffercsvexportjob.h"
#include "allbuffertablewidget.h"
#include "compass.h"
#include "buffer.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "global.h"
#include "jobmanager.h"
#include "listboxview.h"
#include "listboxviewdatasource.h"
#include "dbcontent/variable/metavariable.h"

AllBufferTableModel::AllBufferTableModel(AllBufferTableWidget* table_widget,
                                         ListBoxViewDataSource& data_source)
    : QAbstractTableModel(table_widget), table_widget_(table_widget), data_source_(data_source)
{
//    connect(data_source_.getSet(), &DBOVariableOrderedSet::setChangedSignal, this,
//            &AllBufferTableModel::setChangedSlot);

    connect(&data_source_, &ListBoxViewDataSource::setChangedSignal, this, &AllBufferTableModel::setChangedSlot);
}

AllBufferTableModel::~AllBufferTableModel() {}

void AllBufferTableModel::setChangedSlot()
{
    beginResetModel();
    endResetModel();
    assert(table_widget_);
    table_widget_->resizeColumns();
}

int AllBufferTableModel::rowCount(const QModelIndex& /*parent*/) const
{
    logdbg << "AllBufferTableModel: rowCount: " << row_indexes_.size();
    return row_indexes_.size();
}

int AllBufferTableModel::columnCount(const QModelIndex& /*parent*/) const
{
    logdbg << "AllBufferTableModel: columnCount: " << data_source_.getSet()->getSize();

    if (show_associations_)  // selected, DBO, UTN
        return data_source_.getSet()->getSize() + 3;
    else  // cnt, DBO
        return data_source_.getSet()->getSize() + 2;
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
            return QString();
        if (col == 1)
            return QString("DBObject");

        if (show_associations_)
        {
            if (col == 2)
                return QString("UTN");

            col -= 3;  // for the actual properties
        }
        else
            col -= 2;  // for the actual properties

        assert(col < data_source_.getSet()->getSize());
        std::string variable_name = data_source_.getSet()->variableDefinition(col).variableName();
        return QString(variable_name.c_str());
    }
    else if (orientation == Qt::Vertical)
        return section;

    return QVariant();
}

Qt::ItemFlags AllBufferTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags;

    if (index.column() == 0)
    {
        flags |= Qt::ItemIsEnabled;
        flags |= Qt::ItemIsUserCheckable;
        flags |= Qt::ItemIsEditable;
        // flags |= Qt::ItemIsSelectable;
    }
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

QVariant AllBufferTableModel::data(const QModelIndex& index, int role) const
{
    logdbg << "AllBufferTableModel: data: row " << index.row() - 1 << " col " << index.column() - 1;

    bool null = false;

    assert(index.row() >= 0);
    assert((unsigned int)index.row() < row_indexes_.size());
    unsigned int dbo_num = row_indexes_.at(index.row()).first;
    unsigned int buffer_index = row_indexes_.at(index.row()).second;
    unsigned int col = index.column();

    assert(number_to_dbo_.count(dbo_num) == 1);
    const std::string& dbo_name = number_to_dbo_.at(dbo_num);

    assert(buffers_.count(dbo_name) == 1);
    std::shared_ptr<Buffer> buffer = buffers_.at(dbo_name);

    if (role == Qt::CheckStateRole)
    {
        if (col == 0)  // selected special case
        {
            assert(buffer->has<bool>(DBContent::selected_var.name()));

            if (buffer->get<bool>(DBContent::selected_var.name()).isNull(buffer_index))
                return Qt::Unchecked;

            if (buffer->get<bool>(DBContent::selected_var.name()).get(buffer_index))
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }
    else if (role == Qt::DisplayRole)
    {
        assert(buffer);

        std::string value_str;

        const PropertyList& properties = buffer->properties();

        if (buffer_index >= buffer->size())
        {
            logerr << "AllBufferTableModel: data: index " << buffer_index << " too large for "
                   << dbo_name << "  size " << buffer->size();
            return QVariant();
        }

        assert(buffer_index < buffer->size());

        if (col == 0)  // selected special case
            return QVariant();
        if (col == 1)  // selected special case
            return QVariant(dbo_name.c_str());

        if (show_associations_)
        {
            if (col == 2)
            {
                DBContentManager& manager = COMPASS::instance().objectManager();
                const DBOAssociationCollection& associations =
                    manager.object(dbo_name).associations();

                assert(buffer->has<int>("rec_num"));
                assert(!buffer->get<int>("rec_num").isNull(buffer_index));
                unsigned int rec_num = buffer->get<int>("rec_num").get(buffer_index);

                if (associations.contains(rec_num))
                {
                    return QVariant(
                        manager.object(dbo_name).associations().getUTNsStringFor(rec_num).c_str());
                }
                else
                    return QVariant();
            }

            col -= 3;  // for the actual properties
        }
        else
            col -= 2;  // for the actual properties

        //        loginf << "AllBufferTableModel: data: col " << col << " set size " <<
        //        data_source_.getSet()->getSize()
        //               << " show assoc " << show_associations_;
        assert(col < data_source_.getSet()->getSize());

        std::string variable_dbo_name = data_source_.getSet()->variableDefinition(col).dboName();
        std::string variable_name = data_source_.getSet()->variableDefinition(col).variableName();

        DBContentManager& manager = COMPASS::instance().objectManager();

        // check if data & variables exist
        if (variable_dbo_name == META_OBJECT_NAME)
        {
            assert(manager.existsMetaVariable(variable_name));
            if (!manager.metaVariable(variable_name).existsIn(dbo_name))  // not data if not exist
                return QString();
        }
        else
        {
            if (dbo_name != variable_dbo_name)  // check if other dbo
                return QString();

            assert(manager.existsObject(dbo_name));
            assert(manager.object(dbo_name).hasVariable(variable_name));
        }

        dbContent::Variable& variable = (variable_dbo_name == META_OBJECT_NAME)
                                    ? manager.metaVariable(variable_name).getFor(dbo_name)
                                    : manager.object(dbo_name).variable(variable_name);
        PropertyDataType data_type = variable.dataType();

        value_str = NULL_STRING;

        if (!properties.hasProperty(variable.name()))
        {
            logdbg << "AllBufferTableModel: data: variable " << variable.name()
                   << " not present in buffer";
        }
        else
        {
            std::string property_name = variable.name();

            if (data_type == PropertyDataType::BOOL)
            {
                assert(buffer->has<bool>(property_name));
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
                assert(buffer->has<char>(property_name));
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
                assert(buffer->has<unsigned char>(property_name));
                null = buffer->get<unsigned char>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                            buffer->get<unsigned char>(property_name).getAsString(buffer_index));
                    else
                        value_str =
                            buffer->get<unsigned char>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                assert(buffer->has<int>(property_name));
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
                assert(buffer->has<unsigned int>(property_name));
                null = buffer->get<unsigned int>(property_name).isNull(buffer_index);
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
                assert(buffer->has<long int>(property_name));
                null = buffer->get<long int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                            buffer->get<long int>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<long int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                assert(buffer->has<unsigned long int>(property_name));
                null = buffer->get<unsigned long int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                            buffer->get<unsigned long int>(property_name)
                                .getAsString(buffer_index));
                    else
                        value_str =
                            buffer->get<unsigned long int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                assert(buffer->has<float>(property_name));
                null = buffer->get<float>(property_name).isNull(buffer_index);
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
                assert(buffer->has<double>(property_name));
                null = buffer->get<double>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation_)
                        value_str = variable.getRepresentationStringFromValue(
                            buffer->get<double>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<double>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                assert(buffer->has<std::string>(property_name));
                null = buffer->get<std::string>(property_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = buffer->get<std::string>(property_name).getAsString(buffer_index);
                }
            }
            else
                throw std::domain_error("BufferTableWidget: show: unknown property data type");

            if (null)
                return QVariant();
            else
                return QString(value_str.c_str());
        }
    }
    return QVariant();
}

bool AllBufferTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    logdbg << "AllBufferTableModel: setData: checked row " << index.row() << " col "
           << index.column();

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        assert(index.row() >= 0);
        assert((unsigned int)index.row() < row_indexes_.size());
        unsigned int dbo_num = row_indexes_.at(index.row()).first;
        unsigned int buffer_index = row_indexes_.at(index.row()).second;

        assert(number_to_dbo_.count(dbo_num) == 1);
        std::string dbo_name = number_to_dbo_.at(dbo_num);

        assert(buffers_.count(dbo_name) == 1);
        std::shared_ptr<Buffer> buffer = buffers_.at(dbo_name);

        assert(buffer);
        assert(buffer->has<bool>(DBContent::selected_var.name()));

        if (value == Qt::Checked)
        {
            logdbg << "AllBufferTableModel: setData: checked row index" << buffer_index;
            buffer->get<bool>(DBContent::selected_var.name()).set(buffer_index, true);
        }
        else
        {
            logdbg << "AllBufferTableModel: setData: unchecked row index " << buffer_index;
            buffer->get<bool>(DBContent::selected_var.name()).set(buffer_index, false);
        }
        assert(table_widget_);
        table_widget_->view().emitSelectionChange();

        if (show_only_selected_)
        {
            beginResetModel();

            dbo_last_processed_index_.clear();
            time_to_indexes_.clear();
            row_indexes_.clear();

            updateTimeIndexes();
            rebuildRowIndexes();

            endResetModel();
        }

        QApplication::restoreOverrideCursor();
    }
    return true;
}

void AllBufferTableModel::clearData()
{
    logdbg << "AllBufferTableModel: clearData";

    beginResetModel();

    dbo_last_processed_index_.clear();
    time_to_indexes_.clear();
    row_indexes_.clear();
    buffers_.clear();

    endResetModel();
}

void AllBufferTableModel::setData(std::map<std::string, std::shared_ptr<Buffer>> buffers)
{

    beginResetModel();

    for (auto& buf_it : buffers)
    {
        std::string dbo_name = buf_it.first;

        if (dbo_to_number_.count(dbo_name) == 0)  // new dbo from the wild
        {
            unsigned int num = dbo_to_number_.size();
            number_to_dbo_[num] = dbo_name;
            dbo_to_number_[dbo_name] = num;
        }
    }

    assert(dbo_to_number_.size() == number_to_dbo_.size());

    buffers_ = buffers;

    updateTimeIndexes();
    rebuildRowIndexes();

    //    buffer = buffer;
    //    updateRows();
    //    read_set_ = data_source_.getSet()->getFor(object_.name());

    endResetModel();
}

void AllBufferTableModel::updateTimeIndexes()
{
    logdbg << "AllBufferTableModel: updateTimeIndexes";

    unsigned int buffer_index;
    std::string dbo_name;
    unsigned int dbo_num;
    unsigned int buffer_size;

    unsigned int num_time_none;

    for (auto& buf_it : buffers_)
    {
        buffer_index = 0;
        dbo_name = buf_it.first;
        num_time_none = 0;

        assert(dbo_to_number_.count(dbo_name) == 1);
        dbo_num = dbo_to_number_.at(dbo_name);

        if (dbo_last_processed_index_.count(dbo_name) == 1)
            buffer_index = dbo_last_processed_index_.at(dbo_name) + 1;  // last one + 1

        buffer_size = buf_it.second->size();

        if (buffer_size > buffer_index + 1)  // new data
        {
            logdbg << "AllBufferTableModel: updateTimeIndexes: new " << dbo_name
                   << " data, last index " << buffer_index << " size " << buf_it.second->size();

            DBContentManager& object_manager = COMPASS::instance().objectManager();
            const dbContent::Variable& tod_var =
                    object_manager.metaVariable(DBContent::meta_var_tod_id_.name()).getFor(dbo_name);

            assert(buf_it.second->has<float>(tod_var.name()));
            NullableVector<float>& tods = buf_it.second->get<float>(tod_var.name());

            assert(buf_it.second->has<bool>(DBContent::selected_var.name()));
            NullableVector<bool> selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

            for (; buffer_index < buffer_size; ++buffer_index)
            {
                if (tods.isNull(buffer_index))
                {
                    num_time_none++;
                    continue;
                }

                if (show_only_selected_)
                {
                    if (selected_vec.isNull(buffer_index))  // check if null, skip if so
                        continue;

                    if (selected_vec.get(buffer_index))  // add if set
                        time_to_indexes_.insert(std::make_pair(
                            tods.get(buffer_index), std::make_pair(dbo_num, buffer_index)));
                }
                else
                    time_to_indexes_.insert(std::make_pair(tods.get(buffer_index),
                                                           std::make_pair(dbo_num, buffer_index)));
            }

            dbo_last_processed_index_[dbo_name] = buffer_size - 1;  // set to last index

            if (num_time_none)
                logwrn << "AllBufferTableModel: updateTimeIndexes: new " << dbo_name << " skipped "
                       << num_time_none << " indexes with no time";
        }
    }
}

void AllBufferTableModel::rebuildRowIndexes()
{
    row_indexes_.clear();

    for (auto& time_index_it : time_to_indexes_)
    {
        row_indexes_.push_back(time_index_it.second);
    }
}

void AllBufferTableModel::reset()
{
    beginResetModel();
    endResetModel();
}

void AllBufferTableModel::saveAsCSV(const std::string& file_name, bool overwrite)
{
    loginf << "AllBufferTableModel: saveAsCSV: into filename " << file_name << " overwrite "
           << overwrite;

    if (!buffers_.size())
        return;

    AllBufferCSVExportJob* export_job = new AllBufferCSVExportJob(
        buffers_, data_source_.getSet(), number_to_dbo_, row_indexes_, file_name, overwrite,
        show_only_selected_, use_presentation_, show_associations_);

    export_job_ = std::shared_ptr<AllBufferCSVExportJob>(export_job);
    connect(export_job, &AllBufferCSVExportJob::obsoleteSignal, this,
            &AllBufferTableModel::exportJobObsoleteSlot, Qt::QueuedConnection);
    connect(export_job, &AllBufferCSVExportJob::doneSignal, this,
            &AllBufferTableModel::exportJobDoneSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(export_job_);
}

void AllBufferTableModel::exportJobObsoleteSlot()
{
    logdbg << "AllBufferTableModel: exportJobObsoleteSlot";

    emit exportDoneSignal(true);
}

void AllBufferTableModel::exportJobDoneSlot()
{
    logdbg << "AllBufferTableModel: exportJobDoneSlot";

    emit exportDoneSignal(false);
}

void AllBufferTableModel::usePresentation(bool use_presentation)
{
    loginf << "AllBufferTableModel: usePresentation: " << use_presentation;
    beginResetModel();
    use_presentation_ = use_presentation;
    endResetModel();
}

void AllBufferTableModel::showOnlySelected(bool value)
{
    loginf << "AllBufferTableModel: showOnlySelected: " << value;
    show_only_selected_ = value;

    updateToSelection();
}

void AllBufferTableModel::showAssociations(bool value)
{
    loginf << "AllBufferTableModel: showAssociations: " << value;
    beginResetModel();
    show_associations_ = value;
    endResetModel();
}

void AllBufferTableModel::updateToSelection()
{
    beginResetModel();

    dbo_last_processed_index_.clear();
    time_to_indexes_.clear();
    row_indexes_.clear();

    updateTimeIndexes();
    rebuildRowIndexes();

    endResetModel();
}

std::pair<int,int> AllBufferTableModel::getSelectedRows()
{
    loginf << "AllBufferTableModel: getFirstSelectedRow";

    unsigned int dbo_num;
    unsigned int buffer_index;

    int first_row = -1;
    int last_row = -1;

    for (unsigned int cnt=0; cnt < row_indexes_.size(); ++cnt)
    {
        dbo_num = row_indexes_.at(cnt).first;
        buffer_index = row_indexes_.at(cnt).second;

        assert(number_to_dbo_.count(dbo_num) == 1);
        const std::string& dbo_name = number_to_dbo_.at(dbo_num);

        assert(buffers_.count(dbo_name) == 1);

        std::shared_ptr<Buffer> buffer = buffers_.at(dbo_name);

        assert(buffer->has<bool>(DBContent::selected_var.name()));
        if (buffer->get<bool>(DBContent::selected_var.name()).isNull(buffer_index))
            continue;

        if (buffer->get<bool>(DBContent::selected_var.name()).get(buffer_index))
        {
            if (first_row == -1)
                first_row = cnt;

            last_row = cnt;
        }
    }

    return std::make_pair(first_row, last_row);
}
