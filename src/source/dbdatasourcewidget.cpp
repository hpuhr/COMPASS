#include "dbdatasourcewidget.h"
#include "compass.h"
#include "datasourcemanager.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>

using namespace std;
//using namespace Utils;

namespace dbContent
{


DBDataSourceWidget::DBDataSourceWidget(DBDataSource& src, QWidget *parent)
    : QWidget(parent), src_(src), ds_man_(COMPASS::instance().dataSourceManager())
{
    QGridLayout* main_layout = new QGridLayout();

    load_check_ = new QCheckBox(src_.name().c_str());
    load_check_->setChecked(src_.loadingWanted());
    connect(load_check_, &QCheckBox::clicked, this, &DBDataSourceWidget::loadingChangedSlot);

    main_layout->addWidget(load_check_, 0, 0, 1, 3, Qt::AlignTop | Qt::AlignLeft);

    setLayout(main_layout);
}

void DBDataSourceWidget::update()
{
    unsigned int button_size = 28;
    bool show_counts = ds_man_.loadWidgetShowCounts();

    // ds_id -> line str ->(ip, port)
    std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> net_lines =
            ds_man_.getNetworkLines();

    string tooltip;
}

void DBDataSourceWidget::loadingChangedSlot()
{

}

}
