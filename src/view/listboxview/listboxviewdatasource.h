/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LISTBOXVIEWDATASOURCE_H_
#define LISTBOXVIEWDATASOURCE_H_

//#include "buffer.h"
#include "configurable.h"
//#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableorderedset.h"
//#include "viewselection.h"

#include <QObject>

#include <memory>

class Job;
class ViewableDataConfig;

/**
 * @brief Handles database queries and resulting data for ListBoxView
 *
 * Creates database queries for all contained DBContents when updateData () is called and
 * emits signal updateData() when resulting buffer is delivered by callback. Stores Buffers
 * and handles cleanup.
 */
class ListBoxViewDataSource : public QObject, public Configurable
{
    Q_OBJECT
  public slots:
    void setChangedSlot();

  signals:
    void setChangedSignal();

  public:
    /// @brief Constructor
    ListBoxViewDataSource(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent);
    /// @brief Destructor
    virtual ~ListBoxViewDataSource();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasCurrentSet();
    bool hasSet (const std::string& name);
    void addSet (const std::string& name);
    void copySet (const std::string& name, const std::string& new_name);
    void removeSet (const std::string& name);

    std::string currentSetName() const;
    void currentSetName(const std::string& current_set_name);

    /// @brief Returns variable read list
    dbContent::VariableOrderedSet* getSet();

    const std::map<std::string, std::unique_ptr<dbContent::VariableOrderedSet>>& getSets();

    void unshowViewPoint (const ViewableDataConfig* vp); // vp can be nullptr
    void showViewPoint (const ViewableDataConfig* vp);

protected:
    std::string current_set_name_;

    /// Variable read list
    //DBOVariableOrderedSet* set_{nullptr};
    std::map<std::string, std::unique_ptr<dbContent::VariableOrderedSet>> sets_;

    /// Selected DBContent records
    //ViewSelectionEntries& selection_entries_;

    std::vector<std::pair<std::string, std::string>> temporary_added_variables_; // not persisted, DBO->varname

    virtual void checkSubConfigurables();

    bool addTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name); // only to set, true of added
    void removeTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name); // only to set

    void addDefaultVariables (dbContent::VariableOrderedSet& set);
};

#endif /* LISTBOXVIEWDATASOURCE_H_ */
