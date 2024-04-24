#include "probimmreconstructorwidget.h"

#include "probimmreconstructor.h"
#include "reconstructormainwidget.h"
#include "datasourcesusewidget.h"
#include "reconstructortask.h"
#include "simplereferencecalculatorwidget.h"
#include "reconstructortaskdebugwidget.h"
#include "compass.h"
#include "probabilisticassociationwidget.h"

#include <QCheckBox>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;


ProbIMMReconstructorWidget::ProbIMMReconstructorWidget(ProbIMMReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QTabWidget* tab_widget = new QTabWidget();

    // main widget
    main_widget_.reset(new ReconstructorMainWidget(reconstructor, tab_widget));
    tab_widget->addTab(main_widget_.get(), "Main");

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

    use_widget_.reset(new DataSourcesUseWidget(
        get_use_dstype_func, set_use_dstype_func,
        get_use_ds_func, set_use_ds_func,
        get_use_ds_line_func, set_use_ds_line_func));
    use_widget_->disableDataSources(reconstructor_.task().disabledDataSources());

    tab_widget->addTab(use_widget_.get(), "Data Sources");

    assoc_widget_.reset(new ProbabilisticAssociationWidget(reconstructor_, *this));

    tab_widget->addTab(assoc_widget_.get(), "Association");

    //    assoc_widget_.reset(new SimpleReconstructorAssociationWidget(reconstructor_, *this));
    //    tab_widget->addTab(assoc_widget_.get(), "Association");

    calc_widget_.reset(new SimpleReferenceCalculatorWidget(reconstructor_));
    tab_widget->addTab(calc_widget_.get(), "Reference Calculation");

    debug_widget_.reset(new ReconstructorTaskDebugWidget(reconstructor_.task()));

    if (!COMPASS::instance().isAppImage())
        tab_widget->addTab(debug_widget_.get(), "Debug");

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(tab_widget);

    setLayout(main_layout);
}

ProbIMMReconstructorWidget::~ProbIMMReconstructorWidget()
{
    use_widget_ = nullptr;
    //    assoc_widget_ = nullptr;
    calc_widget_ = nullptr;
    debug_widget_ = nullptr;
}

void ProbIMMReconstructorWidget::update()
{
    assert (use_widget_);
    use_widget_->updateContent();
    use_widget_->disableDataSources(reconstructor_.task().disabledDataSources());

    //    assoc_widget_->update();

    assert (calc_widget_);
    calc_widget_->update();

    assert (debug_widget_);
    debug_widget_->updateValues();
}

void ProbIMMReconstructorWidget::updateSlot()
{
    update();

}
