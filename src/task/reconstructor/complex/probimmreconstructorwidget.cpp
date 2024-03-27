#include "probimmreconstructorwidget.h"
#include "probimmreconstructor.h"

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

    tab_widget->addTab(new QWidget(), "Association");
    tab_widget->addTab(new QWidget(), "Reference Calculation");

//    assoc_widget_.reset(new SimpleReconstructorAssociationWidget(reconstructor_, *this));
//    tab_widget->addTab(assoc_widget_.get(), "Association");

//    calc_widget_.reset(new SimpleReconstructorReferenceCalculationWidget(reconstructor_, *this));
//    tab_widget->addTab(calc_widget_.get(), "Reference Calculation");

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(tab_widget);

    setLayout(main_layout);
}

ProbIMMReconstructorWidget::~ProbIMMReconstructorWidget()
{
//    assoc_widget_ = nullptr;
//    calc_widget_ = nullptr;
}

void ProbIMMReconstructorWidget::update()
{
//    assoc_widget_->update();
//    calc_widget_->update();
}


void ProbIMMReconstructorWidget::updateSlot()
{
    update();
}

