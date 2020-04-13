#ifndef VIEWPOINT_H
#define VIEWPOINT_H

#include "json.hpp"

class ViewManager;

class ViewPoint
{
public:
    ViewPoint(unsigned int id, ViewManager& view_manager);

    unsigned int id() const;

    nlohmann::json& data();

protected:
    unsigned int id_;
    ViewManager& view_manager_;

    nlohmann::json data_;
};

#endif // VIEWPOINT_H
