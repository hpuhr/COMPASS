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

#include "asteriximportertaskwidget.h"
#include "asteriximportertask.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

ASTERIXImporterTaskWidget::ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    main_layout_ = new QHBoxLayout ();

    QVBoxLayout* left_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Import ASTERIX data");
    main_label->setFont (font_big);
    left_layout->addWidget (main_label);

    main_layout_->addLayout(left_layout);

    setLayout (main_layout_);

    show();
}

ASTERIXImporterTaskWidget::~ASTERIXImporterTaskWidget()
{

}
