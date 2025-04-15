#include "reconstructortaskclassificationwidget.h"
#include "reconstructortask.h"
#include "logger.h"

#include <QTextEdit>>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

using namespace std;
using namespace Utils;

ReconstructorTaskClassificationWidget::ReconstructorTaskClassificationWidget(ReconstructorBase& reconstructor, QWidget *parent)
    : QWidget{parent}, reconstructor_(reconstructor)
{
    QFormLayout* layout = new QFormLayout;

    min_aircraft_modec_edit_ = new QSpinBox();
    min_aircraft_modec_edit_->setRange(-10000, 10000);
    connect(min_aircraft_modec_edit_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ReconstructorTaskClassificationWidget::minAircraftModeCEditedSlot);

    layout->addRow(new QLabel("Minimum Aircraft Altitude [ft]"), min_aircraft_modec_edit_);

    vehicle_acids_edit_ = new QTextEdit();
    connect(vehicle_acids_edit_, &QTextEdit::textChanged, this,
            &ReconstructorTaskClassificationWidget::vehicleACIDsChangedSlot);

    layout->addRow(new QLabel("Vehicle ACIDs"), vehicle_acids_edit_);

    vehicle_acads_edit_ = new QTextEdit();
    connect(vehicle_acads_edit_, &QTextEdit::textChanged, this,
            &ReconstructorTaskClassificationWidget::vehicleACIDsChangedSlot);

    layout->addRow(new QLabel("Vehicle ACADs"), vehicle_acads_edit_);

    setLayout(layout);

    updateValues();
}


void ReconstructorTaskClassificationWidget::updateValues()
{
    min_aircraft_modec_edit_->setValue(reconstructor_.settings().min_aircraft_modec_);
    vehicle_acids_edit_->setText(reconstructor_.settings().vehicle_acids_.c_str());
    vehicle_acads_edit_->setText(reconstructor_.settings().vehicle_acads_.c_str());
}

void ReconstructorTaskClassificationWidget::minAircraftModeCEditedSlot (int value)
{
    loginf << "ProbabilisticAssociationWidget: maxAltitudeDiffEditedSlot: value '" << value << "'";

    reconstructor_.settings().max_altitude_diff_ = value;
}

void ReconstructorTaskClassificationWidget::vehicleACIDsChangedSlot()
{
    assert (vehicle_acids_edit_);
    reconstructor_.settings().vehicle_acids_ = vehicle_acids_edit_->document()->toPlainText().toStdString();
}
void ReconstructorTaskClassificationWidget::vehicleACADsChangedSlot()
{
    assert (vehicle_acads_edit_);
    reconstructor_.settings().vehicle_acads_ = vehicle_acads_edit_->document()->toPlainText().toStdString();
}
