/*
 * ProjectionManagerWidget.h
 *
 *  Created on: Nov 21, 2014
 *      Author: sk
 */

#ifndef PROJECTIONMANAGERWIDGET_H_
#define PROJECTIONMANAGERWIDGET_H_

#include <QWidget>

class QLineEdit;
class QLabel;

class ProjectionManagerWidget : public QWidget
{
    Q_OBJECT

public slots:
    void changeEPSG();

public:
    ProjectionManagerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~ProjectionManagerWidget();

protected:
    QLabel *world_proj_info_label_;
    QLineEdit *epsg_edit_;
    QLabel *cart_proj_info_label_;

    void createGUIElements ();
};


#endif /* PROJECTIONMANAGERWIDGET_H_ */
