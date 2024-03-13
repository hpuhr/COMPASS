#pragma once

#include "reconstructorbase.h"

class SimpleReconstructor : public ReconstructorBase
{
  public:
    SimpleReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

  protected:

    virtual bool processSlice_impl() override;
};

