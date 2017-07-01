/*
 * BufferTableWidget.h
 *
 *  Created on: Nov 12, 2012
 *      Author: sk
 */

#ifndef BUFFERTABLEWIDGET_H_
#define BUFFERTABLEWIDGET_H_

#include <QWidget>

#include "global.h"
#include "propertylist.h"


class QTableWidget;
class QTableWidgetItem;
class QStringList;
class Buffer;
class DBOVariableSet;

/**
 * @brief Widget with table representation of a Buffer's data contents
 *
 * For a specific DBObject, a table in Excel manner is created. A header is shown in the first row
 * with the variable names from the variable read list. In the first column, checkboxes are shown
 * for un/selecting DBO records. The subsequent columns show the Buffer contents in either the database
 * view or the transformed string representation.
 *
 * Using the Shift- or Ctrl-key, data items can be selected and copied using Ctrl-C. Such data is stored
 * as comma-separated list in memory and can be inserted in a text file or Excel-like editor.
 */
class BufferTableWidget : public QWidget
{
    Q_OBJECT
public slots:
    /// @brief Is called when table item is clicked, un/checks selection checkboxes
    void itemChanged (QTableWidgetItem *item);

public:
    /// @brief Constructor
    BufferTableWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~BufferTableWidget();

    /// @brief Shows Buffer content in table
    void show (Buffer *buffer, DBOVariableSet *variables, bool database_view);

protected:
    /// Table with items
    QTableWidget *table_;
    /// Variable read lost
    DBOVariableSet *variables_;
    /// Table header list
    QStringList header_list_;
    /// DBObject type
    //DB_OBJECT_TYPE type_;

    /// Container with selection checkboxes
    std::map <QTableWidgetItem*, unsigned int> selection_checkboxes_;

    /// @brief Is called when keys are pressed
    virtual void keyPressEvent (QKeyEvent * event);
};

#endif /* BUFFERTABLEWIDGET_H_ */
