#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/metavariabledetailwidget.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"

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

    setWindowTitle("Configure Meta Variables");
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
        QListWidgetItem* item = new QListWidgetItem(met_it->name().c_str());
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

    loginf << "MetaVariableConfigurationDialog: itemSelectedSlot: item '" << item_name << "'";

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
            if (dbo_man_.usedInMetaVariable(*var_it.get()))
            {
                loginf << "MetaVariableConfigurationDialog: addAllMetaVariablesSlot: not adding dbovariable "
                       << var_it->name() << " since already used";
                continue;
            }

            found_dbos.clear();
            found_dbos.push_back(obj_it.first);  // original object

            for (auto& obj_it2 : dbo_man_)
            {
                if (obj_it == obj_it2)
                    continue;

                if (obj_it2.second->hasVariable(var_it->name()) &&
                        var_it->dataType() == obj_it2.second->variable(var_it->name()).dataType())
                {
                    found_dbos.push_back(obj_it2.first);
                }
            }

            if (found_dbos.size() > 1)
            {
                if (!dbo_man_.existsMetaVariable(var_it->name()))
                {
                    loginf
                            << "MetaVariableConfigurationDialog: addAllMetaVariablesSlot: adding meta variable "
                        << var_it->name();

                    std::string instance = "MetaVariable" + var_it->name() + "0";

                    Configuration& config =
                            dbo_man_.addNewSubConfiguration("MetaVariable", instance);
                    config.addParameterString("name", var_it->name());

                    dbo_man_.generateSubConfigurable("MetaVariable", instance);
                }

                assert(dbo_man_.existsMetaVariable(var_it->name()));
                MetaVariable& meta_var = dbo_man_.metaVariable(var_it->name());

                for (auto dbo_it2 = found_dbos.begin(); dbo_it2 != found_dbos.end(); dbo_it2++)
                {
                    if (!meta_var.existsIn(*dbo_it2))
                    {
                        loginf << "MetaVariableConfigurationDialog: addAllMetaVariablesSlot: adding meta "
                                  "variable "
                               << var_it->name() << " dbo variable " << var_it->name();
                        meta_var.addVariable(*dbo_it2, var_it->name());
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
