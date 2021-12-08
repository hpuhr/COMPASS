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

class DBObjectManagerLoadWidget : public QWidget
{
    Q_OBJECT

  public slots:

    void toggleUseLimit();
    void limitMinChanged();
    void limitMaxChanged();

    void loadButtonSlot();

  public:
    DBObjectManagerLoadWidget(DBObjectManager& object_manager);
    virtual ~DBObjectManagerLoadWidget();

    void update();
    void loadingDone();

  private:
    DBObjectManager& dbo_manager_;

    QGridLayout* type_layout_{nullptr};

    //std::map<std::string, QGridLayout*> type_layouts_; // dbcontent type -> layout

    QLabel* associations_label_{nullptr};

    QCheckBox* limit_check_{nullptr};

    QWidget* limit_widget_{nullptr};
    QLineEdit* limit_min_edit_{nullptr};
    QLineEdit* limit_max_edit_{nullptr};

    QPushButton* load_button_{nullptr};

    std::map<std::string, QCheckBox*> ds_type_boxes_;

    std::map<std::string, QCheckBox*> ds_boxes_; // ds name -> load box
    std::map<std::string, std::map<std::string, QCheckBox*>> ds_content_boxes_; // ds name -> (cont, label)
    std::map<std::string, std::map<std::string, QLabel*>> ds_content_loaded_labels_; // ds name -> (cont, label)
    std::map<std::string, std::map<std::string, QLabel*>> ds_content_total_labels_; // ds name -> (cont, label)

    bool loading_{false};

    void clearAndCreateContent();
    void updateExistingContent();
};

#endif /* DBOBJECTMANAGERINFOWIDGET_H_ */
