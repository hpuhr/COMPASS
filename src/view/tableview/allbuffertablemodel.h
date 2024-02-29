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

#ifndef ALLBUFFERTABLEMODEL_H
#define ALLBUFFERTABLEMODEL_H

//#include "dbcontent/variable/variableset.h"

#include <QAbstractTableModel>

#include "boost/date_time/posix_time/ptime.hpp"

#include <memory>

class Buffer;
class DBContent;
class AllBufferCSVExportJob;
class TableViewDataSource;
class AllBufferTableWidget;

class AllBufferTableModel : public QAbstractTableModel
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void setChangedSlot();
    void exportJobObsoleteSlot();
    void exportJobDoneSlot();

  public:
    AllBufferTableModel(AllBufferTableWidget* table_widget, TableViewDataSource& data_source);
    virtual ~AllBufferTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    void clearData();
    void setData(std::map<std::string, std::shared_ptr<Buffer>> buffers);

    void saveAsCSV(const std::string& file_name);

    void usePresentation(bool use_presentation);
    void showOnlySelected(bool value);
    void reset();

    void updateToSelection();

    std::pair<int,int> getSelectedRows(); // min, max, selected row

    bool showOnlySelected() const { return show_only_selected_; }
    bool usePresentation() const { return use_presentation_; }

  protected:
    AllBufferTableWidget* table_widget_{nullptr};
    TableViewDataSource& data_source_;

    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::shared_ptr<AllBufferCSVExportJob> export_job_;

    std::map<unsigned int, std::string> number_to_dbo_;
    std::map<std::string, unsigned int> dbo_to_number_;

    std::map<std::string, unsigned int> dbo_last_processed_index_;

    std::multimap<boost::posix_time::ptime, std::pair<unsigned int, unsigned int>> time_to_indexes_;
    // timestamp -> [dbo num,index]
    std::vector<std::pair<unsigned int, unsigned int>> row_indexes_;  // row index -> dbo num,index

    bool show_only_selected_{true};
    bool use_presentation_{true};

    void updateTimeIndexes();
    void rebuildRowIndexes();
};

#endif  // ALLBUFFERTABLEMODEL_H
