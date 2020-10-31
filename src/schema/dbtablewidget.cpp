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

#include "dbtablewidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>

#include "configuration.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "formatselectionwidget.h"
#include "logger.h"
#include "stringconv.h"
#include "unitselectionwidget.h"

using namespace Utils;

DBTableWidget::DBTableWidget(DBTable& table, QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), table_(table)
{
    setMinimumSize(QSize(800, 600));

    QSettings settings("ATSDB", "DBTableWidget");
    restoreGeometry(settings.value("DBTableWidget/geometry").toByteArray());

    unsigned int frame_width = 1;

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Edit Table");
    main_label->setFont(font_big);
    main_layout->addWidget(main_label);

    QVBoxLayout* info_layout = new QVBoxLayout();

    // info
    info_layout->addWidget(new QLabel("Description"));

    info_edit_ = new QLineEdit(table_.info().c_str());
    connect(info_edit_, SIGNAL(textChanged(const QString&)), this, SLOT(infoSlot(const QString&)));
    info_layout->addWidget(info_edit_);

    main_layout->addLayout(info_layout);

    // dbtable cols
    QLabel* column_label = new QLabel("Table columns");
    column_label->setFont(font_big);
    main_layout->addWidget(column_label);

    QFrame* colum_frame = new QFrame();
    colum_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    colum_frame->setLineWidth(frame_width);

    column_grid_ = new QGridLayout();
    updateColumnGrid();

    colum_frame->setLayout(column_grid_);

    QScrollArea* column_scroll = new QScrollArea();
    column_scroll->setWidgetResizable(true);
    column_scroll->setWidget(colum_frame);

    main_layout->addWidget(column_scroll);

    QPushButton* update_button = new QPushButton("Update");
    connect(update_button, &QPushButton::clicked, this, &DBTableWidget::updateSlot);
    main_layout->addWidget(update_button);

    setLayout(main_layout);

    show();
}

DBTableWidget::~DBTableWidget()
{
    QSettings settings("ATSDB", "DBTableWidget");
    settings.setValue("DBTableWidget/geometry", saveGeometry());

    column_unit_selection_widgets_.clear();
}

void DBTableWidget::lock()
{
    assert(info_edit_);
    info_edit_->setDisabled(true);

    for (auto& it : column_unit_selection_widgets_)
    {
        it.first->setDisabled(true);
    }
}

void DBTableWidget::infoSlot(const QString& value) { table_.info(value.toStdString()); }

void DBTableWidget::updateSlot()
{
    table_.update();
    updateColumnGrid();
}

void DBTableWidget::updateColumnGrid()
{
    assert(column_grid_);

    QLayoutItem* child;
    while ((child = column_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
        {
            if (!dynamic_cast<UnitSelectionWidget*>(child->widget()) &&
                !dynamic_cast<FormatSelectionWidget*>(child->widget()))
                delete child->widget();
        }
        delete child;
    }

    // column_grid_special_nulls_.clear();
    column_unit_selection_widgets_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    column_grid_->addWidget(name_label, 0, 0);

    QLabel* type_label = new QLabel("Data type");
    type_label->setFont(font_bold);
    column_grid_->addWidget(type_label, 0, 1);

    QLabel* key_label = new QLabel("Is key");
    key_label->setFont(font_bold);
    column_grid_->addWidget(key_label, 0, 2);

    QLabel* unit_label = new QLabel("Unit");
    unit_label->setFont(font_bold);
    column_grid_->addWidget(unit_label, 0, 3);

    //    QLabel *null_label = new QLabel ("Special null"); //TODO
    //    null_label->setFont(font_bold);
    //    column_grid_->addWidget (null_label, 0,4);

    QLabel* data_format_label = new QLabel("Data Format");
    data_format_label->setFont(font_bold);
    column_grid_->addWidget(data_format_label, 0, 4);

    QLabel* comment_label = new QLabel("Comment");
    comment_label->setFont(font_bold);
    column_grid_->addWidget(comment_label, 0, 5);

    unsigned int row = 1;
    for (auto& it : table_.columns())
    {
        QLabel* name = new QLabel(it.second->name().c_str());
        column_grid_->addWidget(name, row, 0);

        QLabel* type = new QLabel(it.second->type().c_str());
        column_grid_->addWidget(type, row, 1);

        QLabel* key = new QLabel(std::to_string((int)it.second->isKey()).c_str());
        column_grid_->addWidget(key, row, 2);

        UnitSelectionWidget* unit_widget = it.second->unitWidget();
        column_grid_->addWidget(unit_widget, row, 3);
        column_unit_selection_widgets_[unit_widget] = it.second;

        FormatSelectionWidget* data_format_widget =
            new FormatSelectionWidget(it.second->propertyType(), it.second->dataFormat());
        column_grid_->addWidget(data_format_widget, row, 4);

        //        QLineEdit *edit = new QLineEdit (it.second->specialNull().c_str());
        //        connect (edit, SIGNAL(textChanged(const QString &)), this, SLOT
        //        (setSpecialNull(const QString &))); column_grid_->addWidget (edit, row, 4); assert
        //        (column_grid_special_nulls_.find (edit) == column_grid_special_nulls_.end());
        //        column_grid_special_nulls_ [edit] = it.second;

        QLabel* comment = new QLabel(it.second->comment().c_str());
        column_grid_->addWidget(comment, row, 5);

        row++;
    }
}

// void DBTableWidget::setSpecialNull (const QString &text)
//{
//    loginf << "DBTableWidget: setSpecialNull";

//    QLineEdit *edit = (QLineEdit*) sender();

//    assert (column_grid_special_nulls_.find (edit) != column_grid_special_nulls_.end());

//    column_grid_special_nulls_[edit]->specialNull(text.toStdString());
//}
