#include "simplereconstructorwidget.h"
#include "simplereconstructor.h"
#include "simplereconstructorassociationwidget.h"
#include "simplereconstructorreferencecalculationwidget.h"

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

    assoc_widget_.reset(new SimpleReconstructorAssociationWidget(reconstructor_, *this));
    tab_widget->addTab(assoc_widget_.get(), "Association");

    calc_widget_.reset(new SimpleReconstructorReferenceCalculationWidget(reconstructor_, *this));
    tab_widget->addTab(calc_widget_.get(), "Reference Calculation");

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(tab_widget);
    //main_layout->addStretch();

    //expertModeChangedSlot();

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
