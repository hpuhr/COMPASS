#pragma once

#include <QWidget>

#include <memory>

class SimpleReconstructor;
class ReconstructorMainWidget;
class DataSourcesUseWidget;
class SimpleReconstructorAssociationWidget;
class ReconstructorTaskClassificationWidget;
class ReferenceCalculatorWidget;
class ReconstructorTaskAnalysisWidget;

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
    std::unique_ptr<ReconstructorTaskClassificationWidget> classif_widget_;
    std::unique_ptr<ReferenceCalculatorWidget> calc_widget_;
    std::unique_ptr<ReconstructorTaskAnalysisWidget> debug_widget_;
};

