/*
 * ListBoxViewDataWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef LISTBOXVIEWDATAWIDGET_H_
#define LISTBOXVIEWDATAWIDGET_H_

#include <QWidget>
#include "global.h"

class ListBoxViewDataSource;
class QTabWidget;
class BufferTableWidget;
class Buffer;

/**
 * @brief Widget with tab containing BufferTableWidgets in ListBoxView
 *
 */
class ListBoxViewDataWidget : public QWidget
{
    Q_OBJECT
public slots:
    /// @brief Called when new result Buffer was delivered
    void updateData (unsigned int dbo_type, Buffer *buffer);

public:
    /// @brief Constructor
    ListBoxViewDataWidget(ListBoxViewDataSource *data_source, QWidget* parent=nullptr, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~ListBoxViewDataWidget();

    /// @brief Clears the table contents
    void clearTables ();

protected:
    /// Data source
    ListBoxViewDataSource *data_source_;
    /// Main tab widget
    QTabWidget *tab_widget_;
    /// Container with all table widgets
    //std::map <DB_OBJECT_TYPE, BufferTableWidget*> buffer_tables_;
};

#endif /* LISTBOXVIEWDATAWIDGET_H_ */
