
#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include "singleton.h"


/**
@brief Base class for common settings of a view category.

This class is thought as a common settings container for a specific class of views.
There is not much to it at the moment.

@todo Not much purpose for this class at the moment, maybe later...
  */
class ViewSettings : public Singleton
{
public:
    ViewSettings();
    virtual ~ViewSettings();

    /// @brief Reimplement to init the settings, e.g. load common resources etc.
    virtual void init() {}

    /// @brief Returns the instance of the singleton
    static ViewSettings& getInstance()
    {
        static ViewSettings instance;
        return instance;
    }
};

#endif //VIEWSETTINGS_H
