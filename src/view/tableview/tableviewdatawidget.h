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

#ifndef LISTBOXVIEWDATAWIDGET_H_
#define LISTBOXVIEWDATAWIDGET_H_

#include "viewdatawidget.h"

#include <memory>

//#include "global.h"

class TableView;
class TableViewWidget;
class TableViewDataSource;
class QTabWidget;
class AllBufferTableWidget;
class BufferTableWidget;
class Buffer;
class DBContent;

/**
 * @brief Widget with tab containing BufferTableWidgets in TableView
 *
 */
class TableViewDataWidget : public ViewDataWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    TableViewDataWidget(TableViewWidget* view_widget, 
                          QWidget* parent = nullptr, 
                          Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~TableViewDataWidget();

    /// @brief Clears the table contents
    void resetModels();
    void updateToSelection();

    void selectFirstSelectedRow();

    AllBufferTableWidget* getAllBufferTableWidget ();

signals:
    void exportDoneSignal(bool cancelled);
    void showOnlySelectedSignal(bool value);
    void usePresentationSignal(bool use_presentation);

public slots:
    void exportDataSlot();
    void exportDoneSlot(bool cancelled);

    void showOnlySelectedSlot(bool value);
    void usePresentationSlot(bool use_presentation);

protected:
    virtual void toolChanged_impl(int mode) override;
    virtual void loadingStarted_impl() override;
    virtual void loadingDone_impl() override;
    virtual void updateData_impl(bool requires_reset) override;
    virtual void clearData_impl() override;
    virtual bool redrawData_impl(bool recompute) override;
    virtual void liveReload_impl() override;

    void viewInfoJSON_impl(nlohmann::json& info) const override;

    TableView*           view_{nullptr};
    /// Data source
    TableViewDataSource* data_source_{nullptr};
    /// Main tab widget
    QTabWidget*            tab_widget_{nullptr};
    /// Container with all table widgets
    AllBufferTableWidget*  all_buffer_table_widget_{nullptr};

    std::map<std::string, BufferTableWidget*> buffer_tables_;
};

#endif /* LISTBOXVIEWDATAWIDGET_H_ */
