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

#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/metavariabledetailwidget.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "compass.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSplitter>
#include <QSettings>

using namespace std;

namespace dbContent
{

MetaVariableConfigurationDialog::MetaVariableConfigurationDialog(DBContentManager& dbo_man)
    : QDialog(), dbo_man_(dbo_man)
{
    if (COMPASS::instance().expertMode())
        setWindowTitle("Configure Meta Variables");
    else
        setWindowTitle("Show Meta Variables");

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QSettings settings("COMPASS", "MetaVariableConfigurationDialog");

    splitter_ = new QSplitter();
    splitter_->setOrientation(Qt::Horizontal);

    list_widget_ = new QListWidget();
    list_widget_->setSortingEnabled(true);
    connect(list_widget_, &QListWidget::currentTextChanged,
            this, &MetaVariableConfigurationDialog::itemSelectedSlot);

    updateList();

    splitter_->addWidget(list_widget_);

    detail_widget_ = new MetaVariableDetailWidget(dbo_man_);

    splitter_->addWidget(detail_widget_);
    splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());

    main_layout->addWidget(splitter_);

    // buttons

    if (COMPASS::instance().expertMode())
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        QPushButton* add_all_metavar_button = new QPushButton("Add All");
        connect(add_all_metavar_button, &QPushButton::clicked,
                this, &MetaVariableConfigurationDialog::addAllMetaVariablesSlot);
        button_layout->addWidget(add_all_metavar_button);

        main_layout->addLayout(button_layout);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        main_layout->addWidget(line);
    }

    // final buttons

    QHBoxLayout* final_button_layout = new QHBoxLayout();

    final_button_layout->addStretch();

    QPushButton* ok_button_ = new QPushButton("OK");
    connect(ok_button_, &QPushButton::clicked, this, &MetaVariableConfigurationDialog::okClickedSlot);
    final_button_layout->addWidget(ok_button_);

    main_layout->addLayout(final_button_layout);

    setLayout(main_layout);
}

MetaVariableConfigurationDialog::~MetaVariableConfigurationDialog()
{
    QSettings settings("COMPASS", "MetaVariableConfigurationDialog");
    settings.setValue("mainSplitterSizes", splitter_->saveState());
}

void MetaVariableConfigurationDialog::updateList()
{
    assert (list_widget_);
    list_widget_->clear();

    unsigned int row_cnt = 0;
    for (auto& met_it : dbo_man_.metaVariables())
    {
        QListWidgetItem* item = new QListWidgetItem(met_it.first.c_str());
        list_widget_->insertItem(row_cnt, item);
        ++row_cnt;
    }
}

void MetaVariableConfigurationDialog::selectMetaVariable (const std::string& name)
{
    assert (list_widget_->findItems(name.c_str(), Qt::MatchExactly).size() == 1);
    list_widget_->setCurrentItem(list_widget_->findItems(name.c_str(), Qt::MatchExactly).at(0));
}


void MetaVariableConfigurationDialog::clearDetails()
{
    assert (detail_widget_);
    detail_widget_->clear();
}

void MetaVariableConfigurationDialog::itemSelectedSlot(const QString& text)
{
    string item_name = text.toStdString();

    loginf << "item '" << item_name << "'";

    assert (detail_widget_);

    if (!item_name.size())
    {
        detail_widget_->clear();
    }
    else
    {
        assert (dbo_man_.existsMetaVariable(item_name));
        detail_widget_->show(dbo_man_.metaVariable(item_name));
    }
}

void MetaVariableConfigurationDialog::addAllMetaVariablesSlot()
{
    std::vector<std::string> found_dbos;

    bool changed = false;

    for (auto& obj_it : dbo_man_)
    {
        for (auto& var_it : obj_it.second->variables())
        {
            if (dbo_man_.usedInMetaVariable(*var_it.second.get()))
            {
                loginf << "not adding dbovariable"
                       << var_it.first << " since already used";
                continue;
            }

            found_dbos.clear();
            found_dbos.push_back(obj_it.first);  // original object

            for (auto& obj_it2 : dbo_man_)
            {
                if (obj_it == obj_it2)
                    continue;

                if (obj_it2.second->hasVariable(var_it.first) &&
                        var_it.second->dataType() == obj_it2.second->variable(var_it.first).dataType())
                {
                    found_dbos.push_back(obj_it2.first);
                }
            }

            if (found_dbos.size() > 1)
            {
                if (!dbo_man_.existsMetaVariable(var_it.first))
                {
                    loginf
                            << "adding meta variable"
                        << var_it.first;

                    std::string instance = "MetaVariable" + var_it.first + "0";

                    auto config = Configuration::create("MetaVariable", instance);
                    config->addParameter<std::string>("name", var_it.first);

                    dbo_man_.generateSubConfigurableFromConfig(std::move(config));
                }

                assert(dbo_man_.existsMetaVariable(var_it.first));
                MetaVariable& meta_var = dbo_man_.metaVariable(var_it.first);

                for (auto dbo_it2 = found_dbos.begin(); dbo_it2 != found_dbos.end(); dbo_it2++)
                {
                    if (!meta_var.existsIn(*dbo_it2))
                    {
                        loginf << "adding meta"
                                  "variable "
                               << var_it.first << " dbo variable " << var_it.first;
                        meta_var.addVariable(*dbo_it2, var_it.first);
                    }
                }

                changed = true;
            }
        }
    }

    if (changed)
        updateList();
}

void MetaVariableConfigurationDialog::okClickedSlot()
{
    emit okSignal();
}

}
