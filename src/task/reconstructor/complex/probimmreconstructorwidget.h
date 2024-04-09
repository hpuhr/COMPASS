#pragma once

#include <QWidget>

#include <memory>

class ProbIMMReconstructor;
class SimpleReferenceCalculatorWidget;

class ProbIMMReconstructorWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateSlot(); // called if something was changed in reconstructor

  public:
    ProbIMMReconstructorWidget(ProbIMMReconstructor& reconstructor);
    virtual ~ProbIMMReconstructorWidget();

    void update();

  private:
    ProbIMMReconstructor& reconstructor_;

    std::unique_ptr<SimpleReferenceCalculatorWidget> calc_widget_;
};

