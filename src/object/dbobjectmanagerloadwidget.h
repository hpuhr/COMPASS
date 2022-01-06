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

#ifndef DBOBJECTMANAGERINFOWIDGET_H_
#define DBOBJECTMANAGERINFOWIDGET_H_

#include <QFrame>
#include <map>

class DBObject;
class DBObjectWidget;
class DBObjectManager;

class QGridLayout;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QLabel;

class DBObjectManagerDataSourcesWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadDSTypeChangedSlot();
    void loadDSChangedSlot();

public:
    DBObjectManagerDataSourcesWidget(DBObjectManager& object_manager);
    virtual ~DBObjectManagerDataSourcesWidget();

    void update();
    void loadingDone();

private:
    DBObjectManager& dbo_manager_;

    bool show_counts_ {false};

    QGridLayout* type_layout_{nullptr};

    std::map<std::string, QCheckBox*> ds_type_boxes_;

    std::map<std::string, QCheckBox*> ds_boxes_; // ds name -> load box
    std::map<std::string, std::map<std::string, QLabel*>> ds_content_boxes_; // ds name -> (cont, label)
    std::map<std::string, std::map<std::string, QLabel*>> ds_content_loaded_labels_; // ds name -> (cont, label)
    std::map<std::string, std::map<std::string, QLabel*>> ds_content_total_labels_; // ds name -> (cont, label)

    void clearAndCreateContent();
    void updateExistingContent();
};

#endif /* DBOBJECTMANAGERINFOWIDGET_H_ */
