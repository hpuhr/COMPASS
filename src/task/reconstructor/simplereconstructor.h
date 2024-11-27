#pragma once

#include "reconstructorbase.h"
#include "global.h"
#include "reconstructortarget.h"
#include "simpleassociator.h"
#include "referencecalculator.h"

class SimpleReconstructorSettings : public ReconstructorBaseSettings
{
  public:
    SimpleReconstructorSettings() {};

    double max_distance_notok_ {5*NM2M};
    double max_distance_dubious_ {2*NM2M};
    double max_distance_acceptable_ {1*NM2M};
};

class SimpleReconstructorWidget;

class SimpleReconstructor : public ReconstructorBase
{
    Q_OBJECT

  signals:
    void updateWidgetsSignal();

  public:
    SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                        ReconstructorTask& task, std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator);
    virtual ~SimpleReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

    virtual void reset() override;

    virtual SimpleReconstructorSettings& settings() override;

    SimpleReconstructorWidget* widget(); // ownage by caller

    virtual void updateWidgets() override;

    virtual void createAdditionalAnnotations() override;

    virtual const std::map<unsigned int, std::map<unsigned int,
                                                  std::pair<unsigned int, unsigned int>>>& assocAounts() const override
        { return associatior_.assocAounts(); };

  protected:
  
    friend class dbContent::ReconstructorTarget;
    friend class SimpleAssociator;
    friend class SimpleReferenceCalculator;

    SimpleReconstructorSettings settings_;
    SimpleAssociator associatior_;
    ReferenceCalculator ref_calculator_;

    virtual void processSlice_impl() override;
};

