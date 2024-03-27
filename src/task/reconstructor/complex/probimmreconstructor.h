#pragma once

#include <QObject>

#include "reconstructorbase.h"
//#include "targetreportdefs.h"
//#include "global.h"
//#include "simpleassociator.h"
//#include "simpleaccuracyestimator.h"
//#include "simplereferencecalculator.h"

class ProbIMMReconstructorSettings
{
  public:
    ProbIMMReconstructorSettings() {};
};


class ProbIMMReconstructorWidget;

class ProbIMMReconstructor : public QObject, public ReconstructorBase
{
    Q_OBJECT

  signals:
    void updateWidgetsSignal();

  public:
    ProbIMMReconstructor(const std::string& class_id, const std::string& instance_id,
                         ReconstructorTask& task);
    virtual ~ProbIMMReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

    virtual void reset() override;

    ProbIMMReconstructorSettings& settings();

    ProbIMMReconstructorWidget* widget(); // ownage by caller

    void updateWidgets();

  protected:

    friend class dbContent::ReconstructorTarget;
//    friend class SimpleAssociator;
//    friend class SimpleReferenceCalculator;

    ProbIMMReconstructorSettings settings_;


    virtual bool processSlice_impl() override;

    void clearOldTargetReports();
    void createTargetReports();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> createAssociations();
    void saveAssociations(std::map<unsigned int, std::map<unsigned long, unsigned int>> associations);
    void saveReferences();
    void saveTargets();
};

