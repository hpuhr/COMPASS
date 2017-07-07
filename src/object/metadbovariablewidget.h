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


#ifndef METADBOVARIABLEWIDGET_H_
#define METADBOVARIABLEWIDGET_H_

#include <QWidget>
#include <map>

class QLineEdit;
class QComboBox;
class QPushButton;
class QTextEdit;

class MetaDBOVariable;
class QGridLayout;
class DBOVariableSelectionWidget;


/**
 * @brief Edit widget for a DBObject
 */
class MetaDBOVariableWidget : public QWidget
{
    Q_OBJECT

signals:
    void metaVariableChangedSignal();

public slots:
    /// @brief Changes DBO name
    void editNameSlot ();
    /// @brief Changes DBO info
    void editDescriptionSlot ();
    void subVariableChangedSlot ();

    void updateSlot ();

public:
    /// @brief Constructor
    MetaDBOVariableWidget(MetaDBOVariable &variable, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~MetaDBOVariableWidget();

private:
    /// @brief DBObject to be managed
    MetaDBOVariable &variable_;

    /// @brief DBOVariable name
    QLineEdit *name_edit_;
    /// @brief DBOVariable info
    QLineEdit *description_edit_;

    QGridLayout *grid_layout_;

    std::map <DBOVariableSelectionWidget*, std::string> selection_widgets_;
};

#endif /* METADBOVARIABLEWIDGET_H_ */
