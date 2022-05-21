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

#include "evaluationdatasourcewidget.h"
#include "logger.h"
#include "dbcontent/dbcontentcombobox.h"
#include "datasourcemanager.h"
#include "evaluationmanager.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>

using namespace std;

EvaluationDataSourceWidget::EvaluationDataSourceWidget(
        const std::string& title, const std::string& dbcontent_name, unsigned int line_id,
        QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), title_(title), dbcontent_name_(dbcontent_name), line_id_(line_id)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel(title_.c_str());
    main_layout->addWidget(main_label);

    QFrame* line = new QFrame();
    //line->setObjectName(QString::fromUtf8("line"));
    //line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(line);

    // dbo
    QGridLayout* dbo_lay = new QGridLayout();

    dbo_lay->addWidget(new QLabel("DBContent"), 0, 0);

    dbo_combo_ = new DBContentComboBox(false);
    dbo_combo_->setObjectName(dbcontent_name_);
    connect (dbo_combo_, &DBContentComboBox::changedObject, this, &EvaluationDataSourceWidget::dboNameChangedSlot);

    dbo_lay->addWidget(dbo_combo_, 0, 1);


    main_layout->addLayout(dbo_lay);

    // data sources
    data_source_layout_ = new QGridLayout();

    //updateDataSources();

    main_layout->addLayout(data_source_layout_);

    main_layout->addStretch();

    //updateCheckboxesChecked();
    //updateCheckboxesDisabled();

    // line
    assert (line_id_ <= 3);

    QGridLayout* line_lay = new QGridLayout();

    line_lay->addWidget(new QLabel("Line ID"), 0, 0);

    QComboBox* line_box = new QComboBox();
    line_box->addItems({"1", "2", "3", "4"});
    line_box->setCurrentIndex(line_id_);

    connect(line_box, &QComboBox::currentTextChanged,
            this, &EvaluationDataSourceWidget::lineIDEditSlot);
    line_lay->addWidget(line_box, 0, 1);

    main_layout->addLayout(line_lay);

    // buttons

    setLayout(main_layout);
}

EvaluationDataSourceWidget::~EvaluationDataSourceWidget()
{
}

void EvaluationDataSourceWidget::updateDataSources()
{
    loginf << "EvaluationDataSourceWidget: updateDataSources";
    assert (data_source_layout_);

    QLayoutItem* child;
    while ((child = data_source_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;

    }
    data_sources_checkboxes_.clear();

    unsigned int col, row;
    unsigned int cnt = 0;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    map<unsigned int, bool> data_sources;

    if (title_ == "Reference Data")
        data_sources = COMPASS::instance().evaluationManager().dataSourcesRef();
    else
        data_sources = COMPASS::instance().evaluationManager().dataSourcesTst();

    for (auto& it : data_sources)
    {
        assert (ds_man.hasDBDataSource(it.first));

        dbContent::DBDataSource& ds = ds_man.dbDataSource(it.first);

        QCheckBox* checkbox = new QCheckBox(tr(ds.name().c_str()));
        checkbox->setChecked(it.second);
        checkbox->setProperty("id", it.first);
        connect(checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSourceSlot()));

        loginf << "EvaluationDataSourceWidget: updateDataSources: got sensor " << it.first << " name "
               << ds.name() << " active " << checkbox->isChecked();

        data_sources_checkboxes_[it.first] = checkbox;

        row = 1 + cnt / 2;
        col = cnt % 2;

        data_source_layout_->addWidget(checkbox, row, col);
        cnt++;
    }
}

void EvaluationDataSourceWidget::updateCheckboxesChecked()
{
    map<unsigned int, bool> data_sources;

    if (title_ == "Reference Data")
        data_sources = COMPASS::instance().evaluationManager().dataSourcesRef();
    else
        data_sources = COMPASS::instance().evaluationManager().dataSourcesTst();

    for (auto& checkit : data_sources_checkboxes_)
    {
        assert(data_sources.count(checkit.first));
        checkit.second->setChecked(data_sources.at(checkit.first));
        logdbg << "EvaluationDataSourceWidget: updateCheckboxesChecked: ds_id " << checkit.first
               << " active " << data_sources.at(checkit.first);
    }
}

void EvaluationDataSourceWidget::dboNameChangedSlot()
{
    assert (dbo_combo_);

    dbcontent_name_ = dbo_combo_->getObjectName();

    loginf << "EvaluationDataSourceWidget: dboNameChangedSlot: name " << dbcontent_name_;

    emit dboNameChangedSignal(dbcontent_name_);

    updateDataSources();

    updateCheckboxesChecked();
    //updateCheckboxesDisabled();
}

//void EvaluationDataSourceWidget::updateCheckboxesDisabled()
//{
//    map<unsigned int, bool> data_sources;

//    if (title_ == "Reference Data")
//        data_sources = COMPASS::instance().evaluationManager().dataSourcesRef();
//    else
//        data_sources = COMPASS::instance().evaluationManager().dataSourcesTst();

//    for (auto& checkit : data_sources_checkboxes_)
//    {
//        assert(data_sources.count(checkit.first));
//        ActiveDataSource& src = data_sources_.at(checkit.first);
//        checkit.second->setEnabled(src.isActiveInData());
//        loginf << "EvaluationDataSourceWidget: updateCheckboxesDisabled: src " << src.getName()
//               << " active " << src.isActiveInData();
//    }
//}

void EvaluationDataSourceWidget::toggleDataSourceSlot()
{
    logdbg << "EvaluationDataSourceWidget: toggleDataSource";
    QCheckBox* check = (QCheckBox*)sender();

    unsigned int ds_id = check->property("id").toInt();

    if (title_ == "Reference Data")
    {
        map<unsigned int, bool>& data_sources = COMPASS::instance().evaluationManager().dataSourcesRef();

        assert(data_sources.count(ds_id));
        data_sources.at(ds_id) = check->checkState() == Qt::Checked;

    }
    else
    {
        map<unsigned int, bool>& data_sources = COMPASS::instance().evaluationManager().dataSourcesTst();

        assert(data_sources.count(ds_id));
        data_sources.at(ds_id) = check->checkState() == Qt::Checked;
    }

    updateCheckboxesChecked();
}

void EvaluationDataSourceWidget::lineIDEditSlot(const QString& text)
{
    bool ok;
    unsigned int line_id = text.toInt(&ok);

    assert (ok);
    assert (line_id <= 4);
    --line_id;

    emit lineChangedSignal(line_id);
}

