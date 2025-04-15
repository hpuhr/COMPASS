

#pragma once

#include <QWidget>

class ReconstructorBase;

class QTextEdit;
class QSpinBox;

class ReconstructorTaskClassificationWidget : public QWidget
{
    Q_OBJECT

  signals:

  public slots:

    void minAircraftModeCEditedSlot (int value);

    void vehicleACIDsChangedSlot();
    void vehicleACADsChangedSlot();

  public:
    ReconstructorTaskClassificationWidget(ReconstructorBase& reconstructor, QWidget *parent = nullptr);
    virtual ~ReconstructorTaskClassificationWidget() {}

    void updateValues();

  protected:
    ReconstructorBase& reconstructor_;

    QSpinBox* min_aircraft_modec_edit_{nullptr};

    QTextEdit* vehicle_acids_edit_ {nullptr};
    QTextEdit* vehicle_acads_edit_ {nullptr};
};

