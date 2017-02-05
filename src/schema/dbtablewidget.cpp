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

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>

//#include "ConfigurationManager.h"
#include "configuration.h"
#include "dbtablewidget.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
//#include "ATSDB.h"
#include "logger.h"
#include "unitselectionwidget.h"

#include "stringconv.h"

using namespace Utils;

DBTableWidget::DBTableWidget(DBTable &table, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), table_(table), info_edit_(0), column_grid_(0)
{
    setMinimumSize(QSize(800, 600));

    QSettings settings("ATSDB", "DBTableWidget");
    restoreGeometry(settings.value("DBTableWidget/geometry").toByteArray());

    unsigned int frame_width = 1;

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Edit Table");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QVBoxLayout *info_layout = new QVBoxLayout ();

    // info
    info_layout->addWidget (new QLabel ("Description"));

    info_edit_ = new QLineEdit (table_.info().c_str());
    connect(info_edit_, SIGNAL( textChanged(const QString &) ), this, SLOT( infoSlot(const QString &) ));
    info_layout->addWidget (info_edit_);

    main_layout->addLayout (info_layout);

    // dbtable cols
    QLabel *column_label = new QLabel ("Table columns");
    column_label->setFont (font_big);
    main_layout->addWidget (column_label);

    QFrame *colum_frame = new QFrame ();
    colum_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    colum_frame->setLineWidth(frame_width);

    column_grid_ = new QGridLayout ();
    updateColumnGrid ();

    colum_frame->setLayout (column_grid_);

    QScrollArea *column_scroll = new QScrollArea ();
    column_scroll->setWidgetResizable (true);
    column_scroll->setWidget(colum_frame);


    main_layout->addWidget (column_scroll);

    // buttons
//    QHBoxLayout *button_layout = new QHBoxLayout ();

//    QPushButton *create_columns = new QPushButton ("Create columns from Database");
//    connect(create_columns, SIGNAL( clicked() ), this, SLOT( createColumnsFromDB() ));
//    create_columns->setDisabled (table_.numColumns() != 0);
//    button_layout->addWidget (create_columns);

//    QPushButton *create_new_columns = new QPushButton ("Create new columns from Database");
//    connect(create_new_columns, SIGNAL( clicked() ), this, SLOT( createNewColumnsFromDB() ));
//    //create_columns->setDisabled (table_.getNumColumns() != 0);
//    button_layout->addWidget (create_new_columns);

//    main_layout->addLayout(button_layout);
//    main_layout->addStretch ();

    setLayout (main_layout);

    show();
}

DBTableWidget::~DBTableWidget()
{
    QSettings settings("ATSDB", "DBTableWidget");
    settings.setValue("DBTableWidget/geometry", saveGeometry());
}

void DBTableWidget::infoSlot (const QString &value)
{
    table_.info (value.toStdString());
}

void DBTableWidget::updateColumnGrid ()
{
    assert (column_grid_);

    QLayoutItem *child;
    while ((child = column_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    column_grid_special_nulls_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont(font_bold);
    column_grid_->addWidget (name_label, 0,0);

    QLabel *type_label = new QLabel ("Data type");
    type_label->setFont(font_bold);
    column_grid_->addWidget (type_label, 0,1);

    QLabel *key_label = new QLabel ("Is key");
    key_label->setFont(font_bold);
    column_grid_->addWidget (key_label, 0,2);

    QLabel *unit_label = new QLabel ("Unit");
    unit_label->setFont(font_bold);
    column_grid_->addWidget (unit_label, 0,3);

    QLabel *null_label = new QLabel ("Special null");
    null_label->setFont(font_bold);
    column_grid_->addWidget (null_label, 0,4);

    QLabel *comment_label = new QLabel ("Comment");
    comment_label->setFont(font_bold);
    column_grid_->addWidget (comment_label, 0,5);


    unsigned int row=1;
    for (auto it : table_.columns ())
    {
        QLabel *name = new QLabel (it.second->name().c_str());
        column_grid_->addWidget (name, row, 0);

        QLabel *type = new QLabel (it.second->type().c_str());
        column_grid_->addWidget (type, row, 1);

        QLabel *key = new QLabel (String::intToString((int)it.second->isKey()).c_str());
        column_grid_->addWidget (key, row, 2);

        UnitSelectionWidget *unit_widget = it.second->unitWidget();
        column_grid_->addWidget (unit_widget, row, 3);

        QLineEdit *edit = new QLineEdit (it.second->specialNull().c_str());
        connect (edit, SIGNAL(returnPressed()), this, SLOT (setSpecialNull()));
        column_grid_->addWidget (edit, row, 4);
        assert (column_grid_special_nulls_.find (edit) == column_grid_special_nulls_.end());
        column_grid_special_nulls_ [edit] = it.second;

        QLabel *comment = new QLabel (it.second->comment().c_str());
        column_grid_->addWidget (comment, row, 5);

        row++;
    }
}

void DBTableWidget::setSpecialNull ()
{
    loginf << "DBTableWidget: setSpecialNull";

    QLineEdit *edit = (QLineEdit*) sender();

    assert (column_grid_special_nulls_.find (edit) != column_grid_special_nulls_.end());

    column_grid_special_nulls_[edit]->specialNull(edit->text().toStdString());
}
