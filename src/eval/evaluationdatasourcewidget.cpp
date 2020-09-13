#include "evaluationdatasourcewidget.h"
#include "logger.h"
#include "activedatasource.h"
#include "dbobjectcombobox.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>

EvaluationDataSourceWidget::EvaluationDataSourceWidget(const std::string& title, const std::string& dbo_name,
                                                       std::map<int, ActiveDataSource>& data_sources,
                                                       QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), title_(title), dbo_name_(dbo_name), data_sources_(data_sources)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(2);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel(title_.c_str());
    main_layout->addWidget(main_label);

    // dbo
    QGridLayout* dbo_lay = new QGridLayout();

    dbo_lay->addWidget(new QLabel("DBObject"), 0, 0);

    dbo_combo_ = new DBObjectComboBox(false);
    dbo_combo_->setObjectName(dbo_name_);
    connect (dbo_combo_, &DBObjectComboBox::changedObject, this, &EvaluationDataSourceWidget::dboNameChangedSlot);

    dbo_lay->addWidget(dbo_combo_, 0, 1);


    main_layout->addLayout(dbo_lay);

    // data sources
    data_source_layout_ = new QGridLayout();

    updateDataSources();

    main_layout->addLayout(data_source_layout_);


    updateCheckboxesChecked();
    updateCheckboxesDisabled();

    setLayout(main_layout);
}

EvaluationDataSourceWidget::~EvaluationDataSourceWidget()
{
}

void EvaluationDataSourceWidget::updateDataSources()
{
    assert (data_source_layout_);

    QLayoutItem* child;
    while ((child = data_source_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;

//        if (child->widget())
//            data_source_layout_->removeWidget(child->widget());

//        data_source_layout_->removeItem(child);
//        delete child;
    }
    data_sources_checkboxes_.clear();

    unsigned int col, row;
    unsigned int cnt = 0;

    for (auto& it : data_sources_)
    {
        QCheckBox* checkbox = new QCheckBox(tr(it.second.getName().c_str()));
        checkbox->setChecked(true);
        checkbox->setProperty("id", it.first);
        connect(checkbox, SIGNAL(clicked()), this, SLOT(toggleDataSourceSlot()));

        loginf << "EvaluationDataSourceWidget: EvaluationDataSourceWidget: got sensor " << it.first << " name "
               << it.second.getName() << " active " << checkbox->isChecked();

        data_sources_checkboxes_[it.first] = checkbox;

        row = 1 + cnt / 2;
        col = cnt % 2;

        data_source_layout_->addWidget(checkbox, row, col);
        cnt++;
    }
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
    assert (dbo_combo_);

    dbo_name_ = dbo_combo_->getObjectName();

    loginf << "EvaluationDataSourceWidget: dboNameChangedSlot: name " << dbo_name_;

    emit dboNameChangedSignal(dbo_name_);

    updateDataSources();

    updateCheckboxesChecked();
    updateCheckboxesDisabled();
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
