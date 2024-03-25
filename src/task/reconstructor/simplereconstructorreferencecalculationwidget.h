#pragma once

#include <QWidget>

class SimpleReconstructor;
class SimpleReconstructorWidget;

class SimpleReconstructorReferenceCalculationWidget : public QWidget
{
    Q_OBJECT

  signals:

  public:
    explicit SimpleReconstructorReferenceCalculationWidget(
        SimpleReconstructor& reconstructor, SimpleReconstructorWidget& parent);
    virtual ~SimpleReconstructorReferenceCalculationWidget();

    void update();

  private:

    SimpleReconstructor& reconstructor_;

    SimpleReconstructorWidget& parent_;
};

