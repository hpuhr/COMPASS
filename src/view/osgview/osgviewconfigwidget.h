/*
 * OSGViewConfigWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEWCONFIGWIDGET_H_
#define OSGVIEWCONFIGWIDGET_H_

#include <QWidget>
#include "dbovariable.h"

class DBOVariableOrderedSetWidget;
//class DBOVariableSelectionWidget;
class OSGView;
class QLineEdit;

/**
 * @brief Widget with configuration elements for a OSGView
 *
 */
class OSGViewConfigWidget : public QWidget
{
    Q_OBJECT

public slots:
    /// @brief Called when the order-by variable was changed
    void orderVariableChanged ();
    /// @brief Called when use filter checkbox is un/checked
    void toggleUseFilters();
    /// @brief Called when the use order checkbox is un/checked
    void toggleUseOrder ();
    /// @brief Called when order ascending checkbox is un/checked
    void toggleOrderAscending ();
    /// @brief Called when use selection checkbox is un/checked
    void toggleUseSelection();
    /// @brief Called when database view checkbox is un/checked
    void toggleDatabaseView ();

public:
    /// @brief Constructor
    OSGViewConfigWidget (OSGView* view, QWidget* parent=nullptr);
    /// @brief Destructor
    virtual ~OSGViewConfigWidget();

protected:
    /// Base view
    OSGView* view_;
    /// Variable read list widget
    DBOVariableOrderedSetWidget *variable_set_widget_;
    /// Order-by variable selection widget
    //DBOVariableSelectionWidget *order_variable_widget_;
};

#endif /* OSGVIEWCONFIGWIDGET_H_ */
