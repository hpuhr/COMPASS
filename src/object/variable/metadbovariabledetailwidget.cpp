#include "metadbovariabledetailwidget.h"
#include "metadbovariable.h"
#include "logger.h"

MetaDBOVariableDetailWidget::MetaDBOVariableDetailWidget(DBObjectManager& dbo_man, QWidget *parent)
    : QWidget(parent), dbo_man_(dbo_man)
{
}


void MetaDBOVariableDetailWidget::show (MetaDBOVariable& meta_var)
{
    loginf << "MetaDBOVariableDetailWidget: show: var '" << meta_var.name() << "'";
}
