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
