#include "primaryonlyfilterwidget.h"
#include "dbcontent.h"

#include <QFormLayout>
#include <QLabel>

PrimaryOnlyFilterWidget::PrimaryOnlyFilterWidget(PrimaryOnlyFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    addNameValuePair(DBContent::meta_var_m3a_.name(), "IS NULL");
    addNameValuePair(DBContent::meta_var_mc_.name(), "IS NULL");
    addNameValuePair(DBContent::meta_var_acad_.name(), "IS NULL");
    addNameValuePair(DBContent::meta_var_acid_.name(), "IS NULL");
    addNameValuePair(DBContent::meta_var_detection_type_.name(), "IN (1,3,6,7) or NULL");
}

PrimaryOnlyFilterWidget::~PrimaryOnlyFilterWidget() = default;
