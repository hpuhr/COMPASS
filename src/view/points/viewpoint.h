#ifndef VIEWPOINT_H
#define VIEWPOINT_H

#include "json.hpp"

class ViewManager;
class LatexVisitor;

class ViewPoint
{
  public:
    ViewPoint(unsigned int id, const nlohmann::json::object_t& data, ViewManager& view_manager, bool needs_save);
    ViewPoint(unsigned int id, const std::string& json_str, ViewManager& view_manager, bool needs_save);

    unsigned int id() const;

    const nlohmann::json& data() const;

    void setStatus (const std::string& status);
    void setComment (const std::string& comment);

    void print() const;

    virtual void accept(LatexVisitor& v) const;

    const unsigned int id_;

protected:
    nlohmann::json data_;
    ViewManager& view_manager_;

    void save();
};

#endif  // VIEWPOINT_H
