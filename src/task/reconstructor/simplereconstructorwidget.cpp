#include "simplereconstructorwidget.h"
#include "simplereconstructor.h"
#include "simplereconstructorassociationwidget.h"
#include "simplereconstructorreferencecalculationwidget.h"
#include "datasourcesusewidget.h"
#include "reconstructortask.h"

#include <QCheckBox>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;

SimpleReconstructorWidget::SimpleReconstructorWidget(SimpleReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QTabWidget* tab_widget = new QTabWidget();

    // use ds widget

    std::function<bool(const std::string&)> get_use_dstype_func =
        [this] (const std::string& ds_type) { return reconstructor_.task().useDStype(ds_type); };
    std::function<void(const std::string&,bool)> set_use_dstype_func =
        [this] (const std::string& ds_type, bool value) { return reconstructor_.task().useDSType(ds_type,value); };
    std::function<bool(unsigned int)> get_use_ds_func =
        [this] (unsigned int ds_id) { return reconstructor_.task().useDataSource(ds_id); };
    std::function<void(unsigned int,bool)> set_use_ds_func =
        [this] (unsigned int ds_id, bool value) { return reconstructor_.task().useDataSource(ds_id, value); };
    std::function<bool(unsigned int,unsigned int)> get_use_ds_line_func =
        [this] (unsigned int ds_id, unsigned int line_id) { return reconstructor_.task().useDataSourceLine(ds_id, line_id); };
    std::function<void(unsigned int,unsigned int, bool)> set_use_ds_line_func =
        [this] (unsigned int ds_id, unsigned int line_id, bool value) { return reconstructor_.task().useDataSourceLine(ds_id, line_id, value); };

    DataSourcesUseWidget* use_widget = new DataSourcesUseWidget(
        get_use_dstype_func, set_use_dstype_func,
        get_use_ds_func, set_use_ds_func,
        get_use_ds_line_func, set_use_ds_line_func);

    tab_widget->addTab(use_widget, "Data Sources");

    assoc_widget_.reset(new SimpleReconstructorAssociationWidget(reconstructor_, *this));
    tab_widget->addTab(assoc_widget_.get(), "Association");

    calc_widget_.reset(new SimpleReconstructorReferenceCalculationWidget(reconstructor_, *this));
    tab_widget->addTab(calc_widget_.get(), "Reference Calculation");

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(tab_widget);

    setLayout(main_layout);
}

SimpleReconstructorWidget::~SimpleReconstructorWidget()
{
    assoc_widget_ = nullptr;
    calc_widget_ = nullptr;
}

void SimpleReconstructorWidget::update()
{
    assoc_widget_->update();
    calc_widget_->update();
}


void SimpleReconstructorWidget::updateSlot()
{
    update();
}
