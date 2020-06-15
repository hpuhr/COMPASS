/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LISTBOXVIEWDATASOURCE_H_
#define LISTBOXVIEWDATASOURCE_H_

#include <QObject>

#include "buffer.h"
#include "configurable.h"
#include "dbovariable.h"
#include "dbovariableorderedset.h"
#include "viewselection.h"

class Job;
class ViewPoint;

/**
 * @brief Handles database queries and resulting data for ListBoxView
 *
 * Creates database queries for all contained DBObjects when updateData () is called and
 * emits signal updateData() when resulting buffer is delivered by callback. Stores Buffers
 * and handles cleanup.
 */
class ListBoxViewDataSource : public QObject, public Configurable
{
    Q_OBJECT
  public slots:
    void loadingStartedSlot();
    void newDataSlot(DBObject& object);
    void loadingDoneSlot(DBObject& object);

  signals:
    void loadingStartedSignal();
    /// @brief Emitted when resulting buffer was delivered
    void updateDataSignal(DBObject& object, std::shared_ptr<Buffer> buffer);

  public:
    /// @brief Constructor
    ListBoxViewDataSource(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent);
    /// @brief Destructor
    virtual ~ListBoxViewDataSource();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Returns variable read list
    DBOVariableOrderedSet* getSet()
    {
        assert(set_);
        return set_;
    }
    /// @brief Returns stored result Buffers
    // std::map <DB_OBJECT_TYPE, Buffer*> &getData () { return data_; }

    /// @brief Sets use selection flag
    // void setUseSelection (bool use_selection) { use_selection_=use_selection; }
    /// @brief Returns use selection flag
    // bool getUseSelection () { return use_selection_; }

    void unshowViewPoint (const ViewPoint* vp); // vp can be nullptr
    void showViewPoint (const ViewPoint* vp);

  protected:
    /// Variable read list
    DBOVariableOrderedSet* set_{nullptr};

    /// Selected DBObject records
    ViewSelectionEntries& selection_entries_;

    std::vector<std::pair<std::string, std::string>> temporary_added_variables_; // not persisted, DBO->varname

    virtual void checkSubConfigurables();

    bool addTemporaryVariable (const std::string& dbo_name, const std::string& var_name); // only to set, true of added
    void removeTemporaryVariable (const std::string& dbo_name, const std::string& var_name); // only to set
};

#endif /* LISTBOXVIEWDATASOURCE_H_ */
