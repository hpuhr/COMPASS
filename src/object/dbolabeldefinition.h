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

#ifndef DBOLABELDEFINITION_H
#define DBOLABELDEFINITION_H

#include <QObject>
#include <list>
#include <memory>

#include "configurable.h"
#include "dbovariableset.h"
//#include "DBObjectManager.h"

class DBContent;
class DBOLabelDefinition;

class DBOLabelEntry : public QObject, public Configurable
{
    Q_OBJECT

  public:
    DBOLabelEntry(const std::string& class_id, const std::string& instance_id,
                  DBOLabelDefinition* parent);

    virtual ~DBOLabelEntry();

    std::string variableName() const;
    void variableName(const std::string& variable_name);

    bool show() const;
    void show(bool show);

    std::string prefix() const;
    void prefix(const std::string& prefix);

    std::string suffix() const;
    void suffix(const std::string& suffix);

  protected:
    DBOLabelDefinition* def_parent_{nullptr};
    std::string variable_name_;

    bool show_;  // show in label
    std::string prefix_;
    std::string suffix_;

    virtual void checkSubConfigurables() {}
};

class DBContentLabelDefinitionWidget;
class Buffer;
class DBContentManager;

class DBOLabelDefinition : public QObject, public Configurable
{
    Q_OBJECT

  public slots:
    void labelDefinitionChangedSlot();

  public:
    DBOLabelDefinition(const std::string& class_id, const std::string& instance_id,
                       DBContent* parent, DBContentManager& dbo_man);
    virtual ~DBOLabelDefinition();

    DBContentVariableSet& readList();
    const std::map<std::string, DBOLabelEntry*>& entries() { return entries_; }
    DBOLabelEntry& entry(const std::string& variable_name);

    void updateReadList();
    void checkLabelDefinitions();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    DBContentLabelDefinitionWidget* widget();

    std::map<unsigned int, std::string> generateLabels(std::vector<unsigned int> rec_nums,
                                                       std::shared_ptr<Buffer> buffer, int break_item_cnt);

  protected:
    DBContent& db_object_;
    DBContentManager& dbo_man_;
    std::map<std::string, DBOLabelEntry*> entries_;  // varname -> labelentry

    DBContentVariableSet read_list_;

    DBContentLabelDefinitionWidget* widget_{nullptr};

    virtual void checkSubConfigurables();
};

#endif  // DBOLABELDEFINITION_H
