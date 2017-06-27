/*
 * ManagementWidget.h
 *
 *  Created on: Mar 31, 2012
 *      Author: sk
 */

#ifndef MANAGEMENTWIDGET_H_
#define MANAGEMENTWIDGET_H_

#include <QWidget>

//class FilterConfigWidget;
//class DBInfoWidget;
//class ResultSetWidget;
//class WorkerThreadWidget;
//class ViewsWidget;

class ManagementWidget : public QWidget
{
  Q_OBJECT

public slots:
    void startSlot ();

public:
  ManagementWidget();
  virtual ~ManagementWidget();

protected:
//  DBInfoWidget *db_info_;
//  ResultSetWidget *result_;
//  FilterConfigWidget *filter_config_gui_;
//  WorkerThreadWidget *worker_widget_;
//  ViewsWidget *views_;

  virtual void resizeEvent (QResizeEvent *event);
};

#endif /* MANAGMENTWIDGET_H_ */
