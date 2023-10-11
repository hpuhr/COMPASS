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

#ifndef ALLBUFFERTABLEWIDGET_H_
#define ALLBUFFERTABLEWIDGET_H_

#include <QWidget>
#include <memory>

//#include "global.h"
//#include "propertylist.h"

class QTableView;
class AllBufferTableModel;
class QStringList;
class Buffer;
class VariableSet;
class DBContent;
class ListBoxView;
class ListBoxViewDataSource;

class AllBufferTableWidget : public QWidget
{
    Q_OBJECT

  signals:
    void exportDoneSignal(bool cancelled);

  public slots:
    void exportSlot(bool overwrite);
    void exportDoneSlot(bool cancelled);

    void showOnlySelectedSlot(bool value);
    void usePresentationSlot(bool use_presentation);

  public:
    AllBufferTableWidget(ListBoxView& view, ListBoxViewDataSource& data_source, QWidget* parent = 0,
                         Qt::WindowFlags f = 0);
    virtual ~AllBufferTableWidget();

    void clear();
    void show(std::map<std::string, std::shared_ptr<Buffer>> buffers);

    void resetModel();
    void updateToSelection();

    ListBoxView& view() const;
    void resizeColumns();

    void selectSelectedRows();

    int rowCount() const;

    std::vector<std::vector<std::string>> getSelectedText (); // first is header
    std::vector<std::vector<std::string>> getText (unsigned int max_rows=30); // first is header

  protected:
    ListBoxView& view_;
    ListBoxViewDataSource& data_source_;
    /// Table with items
    QTableView* table_{nullptr};
    AllBufferTableModel* model_{nullptr};

    /// @brief Is called when keys are pressed
    virtual void keyPressEvent(QKeyEvent* event);
};

#endif /* ALLBUFFERTABLEWIDGET_H_ */
