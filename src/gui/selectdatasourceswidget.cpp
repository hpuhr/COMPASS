#include "selectdatasourceswidget.h"
#include "logger.h"
#include "datasourcemanager.h"
#include "compass.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>

using namespace std;

SelectDataSourcesWidget::SelectDataSourcesWidget(
        const std::string& title, const std::string& ds_type,
        QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), title_(title), ds_type_(ds_type)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel(title_.c_str());
    main_layout->addWidget(main_label);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(line);

    // data sources
    data_source_layout_ = new QGridLayout();

    main_layout->addLayout(data_source_layout_);

    main_layout->addStretch();

    setLayout(main_layout);
}

SelectDataSourcesWidget::~SelectDataSourcesWidget()
{
}

void SelectDataSourcesWidget::updateSelected(std::map<std::string, bool> selection)
{
    loginf << "SelectDataSourcesWidget: updateSelected: ds_type_ " << ds_type_;
    assert (data_source_layout_);

    QLayoutItem* child;
    while (!data_source_layout_->isEmpty() && (child = data_source_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;

    }
    data_sources_checkboxes_.clear();

    unsigned int col, row;
    unsigned int cnt = 0;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id;
    string ds_id_str;
    bool selected;

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (ds_it->dsType() != ds_type_)
            continue;

        ds_id = ds_it->id();

        ds_id_str = to_string(ds_id);

        selected = selection.count(ds_id_str) ? selection.at(ds_id_str) : true; // auto selected

        QCheckBox* checkbox = new QCheckBox(ds_it->name().c_str());
        checkbox->setChecked(selected);
        checkbox->setProperty("id", ds_id);
        connect(checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSourceSlot()));

        loginf << "SelectDataSourcesWidget: updateSelected: got sensor " << ds_id << " name "
               << ds_it->name() << " active " << checkbox->isChecked();

        data_sources_checkboxes_[ds_id] = checkbox;

        row = 1 + cnt / 2;
        col = cnt % 2;

        data_source_layout_->addWidget(checkbox, row, col);
        cnt++;
    }
}

void SelectDataSourcesWidget::toggleDataSourceSlot()
{
    logdbg << "SelectDataSourcesWidget: toggleDataSource";

    std::map<std::string, bool> selection;

    for (auto& check_it : data_sources_checkboxes_)
        selection[to_string(check_it.first)] = check_it.second->checkState() == Qt::Checked;


    emit selectionChangedSignal(selection);
}

