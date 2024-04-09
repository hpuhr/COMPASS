#pragma once

#include <QWidget>

#include <memory>

class SimpleReconstructor;
class SimpleReconstructorAssociationWidget;
class SimpleReferenceCalculatorWidget;

class QLineEdit;
class QCheckBox;

class SimpleReconstructorWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateSlot(); // called if something was changed in reconstructor

  public:
    SimpleReconstructorWidget(SimpleReconstructor& reconstructor);
    virtual ~SimpleReconstructorWidget();

    void update();

  private:
    SimpleReconstructor& reconstructor_;

    std::unique_ptr<SimpleReconstructorAssociationWidget> assoc_widget_;
    std::unique_ptr<SimpleReferenceCalculatorWidget>      calc_widget_;
};

