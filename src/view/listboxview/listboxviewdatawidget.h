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

#ifndef LISTBOXVIEWDATAWIDGET_H_
#define LISTBOXVIEWDATAWIDGET_H_

#include <QWidget>

#include <memory>

#include "global.h"

class ListBoxView;
class ListBoxViewDataSource;
class QTabWidget;
class BufferTableWidget;
class Buffer;
class DBObject;

/**
 * @brief Widget with tab containing BufferTableWidgets in ListBoxView
 *
 */
class ListBoxViewDataWidget : public QWidget
{
    Q_OBJECT

signals:
    void exportDoneSignal (bool cancelled);
    void usePresentationSignal (bool use_presentation);

public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateData (DBObject& object, std::shared_ptr<Buffer> buffer);

    void exportDataSlot(bool overwrite);
    void exportDoneSlot (bool cancelled);

    void usePresentationSlot (bool use_presentation);


public:
    /// @brief Constructor
    ListBoxViewDataWidget(ListBoxView* view, ListBoxViewDataSource* data_source, QWidget* parent=nullptr,
                          Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~ListBoxViewDataWidget();

    /// @brief Clears the table contents
    void clearTables ();
    void resetModels();

protected:
    ListBoxView* view_ {nullptr};
    /// Data source
    ListBoxViewDataSource* data_source_ {nullptr};
    /// Main tab widget
    QTabWidget* tab_widget_ {nullptr};
    /// Container with all table widgets
    std::map <std::string, BufferTableWidget*> buffer_tables_;
};

#endif /* LISTBOXVIEWDATAWIDGET_H_ */
