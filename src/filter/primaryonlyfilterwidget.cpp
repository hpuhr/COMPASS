#include "primaryonlyfilterwidget.h"
#include "dbcontent.h"

#include <QFormLayout>
#include <QLabel>

PrimaryOnlyFilterWidget::PrimaryOnlyFilterWidget(PrimaryOnlyFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QFormLayout* layout = new QFormLayout();

    layout->addRow(DBContent::meta_var_m3a_.name().c_str(), new QLabel("IS NULL"));
    layout->addRow(DBContent::meta_var_mc_.name().c_str(), new QLabel("IS NULL"));
    layout->addRow(DBContent::meta_var_ta_.name().c_str(), new QLabel("IS NULL"));
    layout->addRow(DBContent::meta_var_ti_.name().c_str(), new QLabel("IS NULL"));
    layout->addRow(DBContent::meta_var_detection_type_.name().c_str(), new QLabel("IN (1,3,6,7)"));

    child_layout_->addLayout(layout);
}

PrimaryOnlyFilterWidget::~PrimaryOnlyFilterWidget()
{

}
