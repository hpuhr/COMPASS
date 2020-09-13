#include "evaluationdatasourcewidget.h"
#include "logger.h"
#include "activedatasource.h"
#include "dbobjectcombobox.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>

EvaluationDataSourceWidget::EvaluationDataSourceWidget(const std::string& title, std::string& dbo_name,
                                                       std::map<int, ActiveDataSource>& data_sources,
                                                       QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), title_(title), dbo_name_(dbo_name), data_sources_(data_sources)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel(title_.c_str());
    main_layout->addWidget(main_label);

    QGridLayout* sensorboxlay = new QGridLayout();

    main_layout->addLayout(sensorboxlay);

    sensorboxlay->addWidget(new QLabel("DBObject"), 0, 0);

    DBObjectComboBox* obj_combo = new DBObjectComboBox(false);
    obj_combo->setObjectName(dbo_name_);
    connect (obj_combo, &DBObjectComboBox::changedObject, this, &EvaluationDataSourceWidget::dboNameChangedSlot);

    sensorboxlay->addWidget(obj_combo, 0, 1);

    unsigned int col, row;
    unsigned int cnt = 0;

    for (auto& it : data_sources_)
    {
        QCheckBox* radar_checkbox = new QCheckBox(tr(it.second.getName().c_str()));
        radar_checkbox->setChecked(true);
        radar_checkbox->setProperty("id", it.first);
        connect(radar_checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSourceSlot()));

        loginf << "EvaluationDataSourceWidget: EvaluationDataSourceWidget: got sensor " << it.first << " name "
               << it.second.getName() << " active " << radar_checkbox->isChecked();

        data_sources_checkboxes_[it.first] = radar_checkbox;

        row = 1 + cnt / 2;
        col = cnt % 2;

        sensorboxlay->addWidget(radar_checkbox, row, col);
        cnt++;
    }

    updateCheckboxesChecked();
    updateCheckboxesDisabled();

    setLayout(main_layout);
}

EvaluationDataSourceWidget::~EvaluationDataSourceWidget()
{
}

void EvaluationDataSourceWidget::updateCheckboxesChecked()
{
    for (auto& checkit : data_sources_checkboxes_)
    {
        assert(data_sources_.find(checkit.first) != data_sources_.end());
        ActiveDataSource& src = data_sources_.at(checkit.first);
        checkit.second->setChecked(src.isActive());
        logdbg << "EvaluationDataSourceWidget: updateCheckboxesChecked: name " << src.getName()
               << " active " << src.isActive();
    }
}

void EvaluationDataSourceWidget::dboNameChangedSlot()
{
    loginf << "EvaluationDataSourceWidget: dboNameChangedSlot";
}

void EvaluationDataSourceWidget::updateCheckboxesDisabled()
{
    for (auto& checkit : data_sources_checkboxes_)
    {
        assert(data_sources_.find(checkit.first) != data_sources_.end());
        ActiveDataSource& src = data_sources_.at(checkit.first);
        checkit.second->setEnabled(src.isActiveInData());
        logdbg << "EvaluationDataSourceWidget: updateCheckboxesDisabled: src " << src.getName()
               << " active " << src.isActiveInData();
    }
}

void EvaluationDataSourceWidget::toggleDataSourceSlot()
{
    logdbg << "EvaluationDataSourceWidget: toggleDataSource";
    QCheckBox* check = (QCheckBox*)sender();

    int id = check->property("id").toInt();

    assert(data_sources_.find(id) != data_sources_.end());
    data_sources_.at(id).setActive(check->checkState() == Qt::Checked);

    updateCheckboxesChecked();
}
