#ifndef VIEWPOINT_H
#define VIEWPOINT_H

#include "json.hpp"

class ViewManager;

class ViewPoint
{
  public:
    ViewPoint(unsigned int id, ViewManager& view_manager);
    ViewPoint(unsigned int id, const std::string& json_str, ViewManager& view_manager);

    unsigned int id() const;

    nlohmann::json& data();
    const nlohmann::json& data() const;

    void print();

    bool dirty() const;
    void dirty(bool value);

protected:
    unsigned int id_;
    nlohmann::json data_;
    ViewManager& view_manager_;

    bool dirty_ {false};
};

#endif  // VIEWPOINT_H
