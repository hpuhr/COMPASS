#pragma once

#include <QWidget>

#include <memory>

class SimpleReconstructor;
class ReconstructorMainWidget;
class DataSourcesUseWidget;
class SimpleReconstructorAssociationWidget;
class SimpleReferenceCalculatorWidget;
class ReconstructorTaskDebugWidget;

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

    std::unique_ptr<ReconstructorMainWidget> main_widget_;
    std::unique_ptr<DataSourcesUseWidget> use_widget_;
    std::unique_ptr<SimpleReconstructorAssociationWidget> assoc_widget_;
    std::unique_ptr<SimpleReferenceCalculatorWidget>      calc_widget_;
    std::unique_ptr<ReconstructorTaskDebugWidget> debug_widget_;
};

