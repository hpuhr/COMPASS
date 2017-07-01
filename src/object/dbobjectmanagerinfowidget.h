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

#ifndef DBOBJECTMANAGERINFOWIDGET_H_
#define DBOBJECTMANAGERINFOWIDGET_H_

#include <QFrame>
#include <map>

class DBObject;
class DBObjectWidget;
class DBObjectManager;
class QVBoxLayout;
class QPushButton;
class QCheckBox;
class QLineEdit;

/**
 * @brief Shows all DBObjects, allows editing and adding new ones
 */
class DBObjectManagerInfoWidget : public QFrame
{
    Q_OBJECT

public slots:
    void toggleUseLimit ();
    /// @brief Called when limit minimum is changed
    void limitMinChanged();
    /// @brief Called when limit maximum is changed
    void limitMaxChanged();



    void loadAllSlot ();
    void updateSlot ();

public:
    /// @brief Constructor
    DBObjectManagerInfoWidget(DBObjectManager &object_manager);
    /// @brief Destructor
    virtual ~DBObjectManagerInfoWidget();

private:
    DBObjectManager &object_manager_;
    QVBoxLayout *info_layout_;

    QCheckBox *limit_check_;
    /// Limit minimum edit field
    QLineEdit *limit_min_edit_;
    /// Limit maximum edit field
    QLineEdit *limit_max_edit_;

    QPushButton *load_all_button_;

};

#endif /* DBOBJECTMANAGERINFOWIDGET_H_ */
