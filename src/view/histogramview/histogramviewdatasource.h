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

#ifndef HISTOGRAMVIEWDATASOURCE_H_
#define HISTOGRAMVIEWDATASOURCE_H_

#include <QObject>

#include "buffer.h"
#include "configurable.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableorderedset.h"

class Job;
class ViewableDataConfig;

class HistogramViewDataSource : public QObject, public Configurable
{
    Q_OBJECT
  public slots:

  signals:
    /// @brief Emitted when resulting buffer was delivered
    void updateDataSignal(DBContent& object, std::shared_ptr<Buffer> buffer);

  public:
    /// @brief Constructor
    HistogramViewDataSource(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent);
    /// @brief Destructor
    virtual ~HistogramViewDataSource();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Returns variable read list
    dbContent::VariableOrderedSet* getSet()
    {
        assert(set_);
        return set_;
    }
    void unshowViewPoint (const ViewableDataConfig* vp); // vp can be nullptr
    void showViewPoint (const ViewableDataConfig* vp);

  protected:
    /// Variable read list
    dbContent::VariableOrderedSet* set_{nullptr};

    /// Selected DBContent records
    //ViewSelectionEntries& selection_entries_;

    std::vector<std::pair<std::string, std::string>> temporary_added_variables_; // not persisted, DBO->varname

    virtual void checkSubConfigurables();

    bool addTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name); // only to set, true of added
    void removeTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name); // only to set
};

#endif /* HISTOGRAMVIEWDATASOURCE_H_ */
