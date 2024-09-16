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

#pragma once

#include <QWidget>
#include <memory>

class QTableView;
class AllBufferTableModel;
class QStringList;
class Buffer;
class VariableSet;
class DBContent;
class TableView;
class TableViewDataSource;

class AllBufferTableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void exportSlot();
    void exportDoneSlot(bool cancelled);

  public:
    AllBufferTableWidget(TableView& view, TableViewDataSource& data_source, QWidget* parent = 0,
                         Qt::WindowFlags f = 0);
    virtual ~AllBufferTableWidget();

    void updateToSettingsChange();

    void clear();
    void show(std::map<std::string, std::shared_ptr<Buffer>> buffers);

    void resetModel();
    void updateToSelection();

    TableView& view() const;
    void resizeColumns();

    void selectSelectedRows();

    int rowCount() const;

    std::vector<std::vector<std::string>> getSelectedText (); // first is header
    std::vector<std::vector<std::string>> getText (unsigned int max_rows=30); // first is header

    const QTableView* table() const { return table_; }

  protected:
    TableView& view_;
    TableViewDataSource& data_source_;

    QTableView* table_{nullptr};
    AllBufferTableModel* model_{nullptr};


    virtual void keyPressEvent(QKeyEvent* event);
};
