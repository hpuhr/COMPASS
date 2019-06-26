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

#ifndef BUFFERTABLEMODEL_H
#define BUFFERTABLEMODEL_H

#include "dbovariableset.h"

#include <memory>

#include <QAbstractTableModel>

class Buffer;
class DBObject;
class BufferCSVExportJob;
class ListBoxViewDataSource;
class BufferTableWidget;

class BufferTableModel : public QAbstractTableModel
{
    Q_OBJECT

signals:
    void exportDoneSignal (bool cancelled);

public slots:
    void exportJobObsoleteSlot ();
    void exportJobDoneSlot();

public:
    BufferTableModel(BufferTableWidget* table_widget, DBObject& object, ListBoxViewDataSource& data_source);
    virtual ~BufferTableModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex & index, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


    void clearData ();
    void setData (std::shared_ptr <Buffer> buffer);

    void saveAsCSV (const std::string& file_name, bool overwrite);

    void usePresentation (bool use_presentation);
    void showOnlySelected (bool value);
    void reset ();

    void updateToSelection();

protected:
    BufferTableWidget* table_widget_ {nullptr};
    DBObject& object_;
    ListBoxViewDataSource& data_source_;

    std::shared_ptr <Buffer> buffer_;
    DBOVariableSet read_set_;

    std::shared_ptr <BufferCSVExportJob> export_job_;

    std::map <unsigned int, unsigned int> row_to_index_;

    bool show_only_selected_ {true};
    bool use_presentation_ {true};

    void updateRows ();
};

#endif // BUFFERTABLEMODEL_H
