#include "dbdatasourcewidget.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "stringconv.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVariant>

using namespace std;
using namespace Utils;

namespace dbContent
{


DBDataSourceWidget::DBDataSourceWidget(DBDataSource& src, QWidget *parent)
    : QWidget(parent), src_(src), ds_man_(COMPASS::instance().dataSourceManager())
{
    main_layout_ = new QVBoxLayout();

    grid_layout_ = new QGridLayout();

    updateContent();

    main_layout_->addLayout(grid_layout_);

    setLayout(main_layout_);

    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void DBDataSourceWidget::updateContent()
{
    if (needsRecreate())
        recreateWidgets();

    updateWidgets();

}

bool DBDataSourceWidget::needsRecreate()
{
    if (!line_buttons_.size())
        return true;

    // check counts shown
    bool show_counts = ds_man_.loadWidgetShowCounts();

    if (last_show_counts_ != show_counts)
        return true;

    if (show_counts)
    {
        for (auto& cnt_it : src_.numInsertedSummedLinesMap())
        {
            if (!content_labels_.count(cnt_it.first)) // content name, not yet created
                return true;
        }

        return src_.numInsertedSummedLinesMap().size() != content_labels_.size(); // check that not too many
    }

    return false;
}

void DBDataSourceWidget::recreateWidgets()
{
    bool show_counts = ds_man_.loadWidgetShowCounts();

    loginf << "DBDataSourceWidget " << src_.name() << ": recreateWidgets: show_counts " << show_counts;

    QLayoutItem* child;
    while ((child = grid_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    load_check_ = nullptr;
    line_buttons_.clear();
    content_labels_.clear();
    loaded_cnt_labels_.clear();
    total_cnt_labels_.clear();

    // update load check
    load_check_ = new QCheckBox(src_.name().c_str());
    connect(load_check_, &QCheckBox::clicked, this, &DBDataSourceWidget::loadingChangedSlot);

    unsigned int row = 0;

    grid_layout_->addWidget(load_check_, row, 0, 1, 2);
    grid_layout_->addWidget(createLinesWidget(), row, 2, 1, 2);
    ++row;

    if (show_counts)
    {
        string ds_content_name;

        for (auto& cnt_it : src_.numInsertedSummedLinesMap())
        {
            ds_content_name = cnt_it.first;

            assert (!content_labels_.count(ds_content_name));
            assert (!loaded_cnt_labels_.count(ds_content_name));
            assert (!total_cnt_labels_.count(ds_content_name));

            content_labels_[ds_content_name] = new QLabel(ds_content_name.c_str());
            content_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            loaded_cnt_labels_[ds_content_name] = new QLabel(QString::number(src_.numLoaded(ds_content_name)));
            loaded_cnt_labels_.at(ds_content_name)->setAlignment(Qt::AlignRight);
            loaded_cnt_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            total_cnt_labels_[ds_content_name] = new QLabel(QString::number(cnt_it.second));
            total_cnt_labels_.at(ds_content_name)->setAlignment(Qt::AlignRight);
            total_cnt_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            grid_layout_->addWidget(content_labels_.at(ds_content_name), row, 1);
            grid_layout_->addWidget(loaded_cnt_labels_.at(ds_content_name), row, 2);
            grid_layout_->addWidget(total_cnt_labels_.at(ds_content_name), row, 3);
        }
    }

    for(int c=0; c < grid_layout_->columnCount(); c++)
    {
        if (c == 0) // first is half placeholder
            grid_layout_->setColumnStretch(c, 1);
        else
            grid_layout_->setColumnStretch(c, 2);
    }

    last_show_counts_ = show_counts;

    update();
}

QWidget* DBDataSourceWidget::createLinesWidget()
{
    QWidget* widget = new QWidget();
    widget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* button_lay = new QHBoxLayout();
    button_lay->setContentsMargins(0, 0, 0, 0);
    button_lay->addStretch();

    string line_str;

    unsigned int button_size = 26;
    widget->setMinimumHeight(button_size);

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        line_str = "L"+to_string(cnt+1);

        QPushButton* button = new QPushButton (line_str.c_str());
        button->setFixedSize(button_size,button_size);
        button->setCheckable(true);
        button->setStyleSheet(" QPushButton:pressed { border: 3px outset; } " \
        " QPushButton:checked { border: 3px outset; }");
        button->setChecked(true);

        button->setProperty("Line ID", cnt);
        connect (button, &QPushButton::clicked, this, &DBDataSourceWidget::lineButtonClickedSlot);

        QSizePolicy sp_retain = widget->sizePolicy();
        sp_retain.setRetainSizeWhenHidden(true);
        button->setSizePolicy(sp_retain);

        line_buttons_[line_str] = button;

        button_lay->addWidget(button);
    }

    widget->setLayout(button_lay);

    return widget;
}

void DBDataSourceWidget::updateWidgets()
{
    logdbg << "DBDataSourceWidget: updateWidgets";

    bool show_counts = ds_man_.loadWidgetShowCounts();

    assert (load_check_);
    load_check_->setText(src_.name().c_str());
    load_check_->setChecked(src_.loadingWanted());

    bool net_lines_shown = COMPASS::instance().appMode() == AppMode::LivePaused
            || COMPASS::instance().appMode() == AppMode::LiveRunning;

    if (net_lines_shown)
    {
        // ds_id -> line str ->(ip, port)
        std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> net_lines =
                ds_man_.getNetworkLines();

        string line_str;

        for (unsigned int cnt=0; cnt < 4; ++cnt)
        {
            line_str = "L"+to_string(cnt+1);

            assert (line_buttons_.count(line_str));

            line_buttons_.at(line_str)->setHidden(!net_lines.at(src_.id()).count(line_str)); // hide if no data
        }

        string tooltip;
    }
    else
    {
        // LX -> cnt
        std::map<unsigned int, unsigned int> inserted_lines = src_.numInsertedLinesMap();

        string line_str;

        for (unsigned int cnt=0; cnt < 4; ++cnt)
        {
            line_str = "L"+to_string(cnt+1);

            assert (line_buttons_.count(line_str));

            line_buttons_.at(line_str)->setHidden(!inserted_lines.count(cnt)); // hide if no data
        }
    }


    if (show_counts)
    {
        string ds_content_name;
        for (auto& cnt_it : src_.numInsertedSummedLinesMap())
        {
            ds_content_name = cnt_it.first;

            // content label

            assert (content_labels_.count(ds_content_name));
            assert (loaded_cnt_labels_.count(ds_content_name));
            assert (total_cnt_labels_.count(ds_content_name));

            loaded_cnt_labels_[ds_content_name]->setText(QString::number(src_.numLoaded(ds_content_name)));
            total_cnt_labels_[ds_content_name]->setText(QString::number(cnt_it.second));
        }
    }
}

void DBDataSourceWidget::loadingChangedSlot()
{
    loginf << "DBDataSourceWidget: loadingChangedSlot";

    src_.loadingWanted(!src_.loadingWanted());
}

void DBDataSourceWidget::lineButtonClickedSlot()
{
    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    assert (sender);

    unsigned int line_id = sender->property("Line ID").toUInt();

    loginf << "DBDataSourceWidget: lineButtonClickedSlot: line " << line_id;

    src_.lineLoadingWanted(line_id, !src_.lineLoadingWanted(line_id));
}

}
