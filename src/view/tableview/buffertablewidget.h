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

class BufferTableModel;
class QStringList;
class Buffer;
class VariableSet;
class DBContent;
class TableView;
class TableViewDataSource;


class BufferTableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void exportSlot();
    void exportDoneSlot(bool cancelled);

  public:
    BufferTableWidget(DBContent& object, TableView& view, TableViewDataSource& data_source,
                      QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~BufferTableWidget();

    void updateToSettingsChange();

    void clear();
    void show(std::shared_ptr<Buffer> buffer);

    void resetModel();
    void updateToSelection();

    TableView& view() const;
    void resizeColumns();

    bool hasData() const;

    const QTableView* table() const { return table_; }

  protected:
    DBContent& object_;
    TableView& view_;
    TableViewDataSource& data_source_;

    QTableView* table_{nullptr};
    BufferTableModel* model_{nullptr};

    virtual void keyPressEvent(QKeyEvent* event);
};
