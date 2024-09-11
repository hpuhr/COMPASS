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

#ifndef BUFFERTABLEWIDGET_H_
#define BUFFERTABLEWIDGET_H_

#include <QWidget>
#include <memory>

//#include "global.h"
//#include "propertylist.h"

class QTableView;
// class QTableWidgetItem;
class BufferTableModel;
class QStringList;
class Buffer;
class VariableSet;
class DBContent;
class TableView;
class TableViewDataSource;

/**
 * @brief Widget with table representation of a Buffer's data contents
 *
 * For a specific DBContent, a table in Excel manner is created. A header is shown in the first row
 * with the variable names from the variable read list. In the first column, checkboxes are shown
 * for un/selecting DBO records. The subsequent columns show the Buffer contents in either the
 * database view or the transformed string representation.
 *
 * Using the Shift- or Ctrl-key, data items can be selected and copied using Ctrl-C. Such data is
 * stored as comma-separated list in memory and can be inserted in a text file or Excel-like editor.
 */
class BufferTableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void exportSlot();
    void exportDoneSlot(bool cancelled);

  public:
    /// @brief Constructor
    BufferTableWidget(DBContent& object, TableView& view, TableViewDataSource& data_source,
                      QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~BufferTableWidget();

    void showOnlySelected(bool value);
    void usePresentation(bool value);
    void ignoreNonTargetReports(bool value);

    void clear();
    /// @brief Shows Buffer content in table
    void show(std::shared_ptr<Buffer> buffer);

    void resetModel();
    void updateToSelection();

    TableView& view() const;
    void resizeColumns();

    // bool showOnlySelected() const;
    // bool usePresentation() const;

    bool hasData() const;

    const QTableView* table() const { return table_; }

  protected:
    DBContent& object_;
    TableView& view_;
    TableViewDataSource& data_source_;
    /// Table with items
    QTableView* table_{nullptr};
    BufferTableModel* model_{nullptr};

    /// @brief Is called when keys are pressed
    virtual void keyPressEvent(QKeyEvent* event);
};

#endif /* BUFFERTABLEWIDGET_H_ */
