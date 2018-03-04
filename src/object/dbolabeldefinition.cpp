#include "dbolabeldefinition.h"
#include "dbolabeldefinitionwidget.h"
#include "buffer.h"
#include "dbobject.h"
#include "dbovariable.h"

#include "buffer.h"
#include "propertylist.h"
#include "global.h"

#include <iostream>
#include <string>


DBOLabelEntry::DBOLabelEntry(const std::string& class_id, const std::string& instance_id, DBOLabelDefinition* parent)
    : Configurable (class_id, instance_id, parent), def_parent_(parent)
{
    registerParameter("variable_name", &variable_name_, "");
    registerParameter("show", &show_, false);
    registerParameter("prefix", &prefix_, "");
    registerParameter("suffix", &suffix_, "");

    createSubConfigurables ();
}

DBOLabelEntry::~DBOLabelEntry()
{

}

std::string DBOLabelEntry::variableName() const
{
    return variable_name_;
}

void DBOLabelEntry::variableName(const std::string& variable_name)
{
    variable_name_ = variable_name;
}

bool DBOLabelEntry::show() const
{
    return show_;
}

void DBOLabelEntry::show(bool show)
{
    show_ = show;

    assert (def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

std::string DBOLabelEntry::prefix() const
{
    return prefix_;
}

void DBOLabelEntry::prefix(const std::string& prefix)
{
    prefix_ = prefix;

    assert (def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

std::string DBOLabelEntry::suffix() const
{
    return suffix_;
}

void DBOLabelEntry::suffix(const std::string& suffix)
{
    suffix_ = suffix;

    assert (def_parent_);
    def_parent_->labelDefinitionChangedSlot();
}

DBOLabelDefinition::DBOLabelDefinition(const std::string& class_id, const std::string& instance_id, DBObject* parent)
    : Configurable (class_id, instance_id, parent), db_object_(parent)
{
    assert (db_object_);

    createSubConfigurables ();
}

DBOLabelDefinition::~DBOLabelDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : entries_)
    {
        delete it.second;
    }
    entries_.clear();

    read_list_.clear();
}

DBOVariableSet& DBOLabelDefinition::readList ()
{
    updateReadList();
    return read_list_;
}

void DBOLabelDefinition::updateReadList ()
{
    read_list_.clear();

    for (auto it : entries_)
    {
        if (it.second->show())
        {
            assert (db_object_->hasVariable(it.second->variableName()));
            read_list_.add(db_object_->variable(it.second->variableName()));
        }
    }
}

void DBOLabelDefinition::checkLabelDefintions()
{
    //TODO check if up to date
}

void DBOLabelDefinition::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    if (class_id == "LabelEntry")
    {
        logdbg  << "DBOLabelDefinition: generateSubConfigurable: instance_id " << instance_id;

        DBOLabelEntry* entry = new DBOLabelEntry (class_id, instance_id, this);
        assert (entries_.find(entry->variableName()) == entries_.end());
        entries_ [entry->variableName()] = entry;
    }
    else
        throw std::runtime_error ("DBOLabelDefinition: generateSubConfigurable: unknown class_id "+class_id );
}

void DBOLabelDefinition::checkSubConfigurables ()
{
    logdbg  << "DBOLabelDefinition: checkSubConfigurables: object " << db_object_->name();

    assert (db_object_);
    const std::map<std::string, DBOVariable*>& variables = db_object_->variables();

    std::string variable_name;
    bool show=false;
    std::string prefix;
    std::string suffix;

    for (auto it : variables)
    {
        if (entries_.find (it.second->name()) == entries_.end())
        {
            variable_name = it.second->name();
            show=false;
            prefix="";
            suffix="";

            logdbg  << "DBOLabelDefinition: checkSubConfigurables: new var " << variable_name;

            if (variable_name == "tod")
            {
                show=true;
                prefix="T ";
            }
            else if (variable_name == "mode3a_code")
            {
                show=true;
                prefix="A ";
            }
            else if (variable_name == "modec_code_ft")
            {
                show=true;
                prefix="C ";
            }
            else if (variable_name == "rec_num")
            {
                show=true;
                prefix="R ";
            }
            std::string instance_id = "LabelEntry"+variable_name+"0";

            Configuration &configuration = addNewSubConfiguration ("LabelEntry", instance_id);
            configuration.addParameterString("variable_name", variable_name);
            configuration.addParameterBool("show", show);
            configuration.addParameterString("prefix", prefix);
            configuration.addParameterString("suffix", suffix);
            generateSubConfigurable ("LabelEntry", instance_id);
        }
    }
}
DBOLabelEntry& DBOLabelDefinition::entry (const std::string& variable_name)
{
    return *entries_.at(variable_name);
}

DBOLabelDefinitionWidget* DBOLabelDefinition::widget ()
{
    if (!widget_)
    {
        widget_ = new DBOLabelDefinitionWidget (this);
        assert (widget_);
    }
    return widget_;
}

std::map<int, std::string> DBOLabelDefinition::generateLabels (
        std::vector<int> rec_nums, std::shared_ptr<Buffer> buffer, int break_item_cnt)
{
    const PropertyList &buffer_properties = buffer->properties();
    assert (buffer_properties.size() >= read_list_.getSize());

    assert (buffer->size() == rec_nums.size());

    // check and insert strings for with rec_num
    std::map<int, std::string> labels;

    std::map<int, size_t> rec_num_to_index;
    ArrayListTemplate<int>& rec_num_list = buffer->getInt("rec_num");
    for (size_t cnt=0; cnt < rec_num_list.size(); cnt++)
    {
        assert (!rec_num_list.isNone(cnt));
        int rec_num = rec_num_list.get(cnt);
        assert (labels.count(rec_num) == 0);
        rec_num_to_index[rec_num] = cnt;
    }

    PropertyDataType data_type;
    std::string value_str;
    DBOLabelEntry* entry;
    bool null;

    int var_count = 0;
    for (DBOVariable* variable : read_list_.getSet())
    {
        data_type = variable->dataType();
        value_str = NULL_STRING;
        entry = entries_[variable->name()];
        assert (entry);

        for (size_t cnt=0; cnt < rec_num_list.size(); cnt++)
        {
            int rec_num = rec_num_list.get(cnt); //already checked

            assert (rec_num_to_index.count(rec_num) == 1);
            size_t buffer_index = rec_num_to_index[rec_num];

            if (data_type == PropertyDataType::BOOL)
            {
                null = buffer->getBool(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getBool(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::CHAR)
            {
                null = buffer->getChar(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getChar(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::UCHAR)
            {
                null = buffer->getUChar(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getUChar(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                null = buffer->getInt(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getInt(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::UINT)
            {
                null = buffer->getUInt(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getUInt(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::LONGINT)
            {
                null = buffer->getLongInt(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getLongInt(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                null = buffer->getULongInt(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getULongInt(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                null = buffer->getFloat(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getFloat(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::DOUBLE)
            {
                null = buffer->getDouble(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getDouble(variable->name()).getAsString(buffer_index));
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                null = buffer->getString(variable->name()).isNone(buffer_index);
                if (!null)
                {
                    value_str = variable->getRepresentationStringFromValue(
                                buffer->getString(variable->name()).getAsString(buffer_index));
                }
            }
            else
                throw std::domain_error ("DBOLabelDefinition::generateLabels: unknown property data type");


            if (!null)
            {
                if (var_count == break_item_cnt - 1)
                {
                    labels[rec_num] += "\n";
                }
                else if (labels[rec_num].size() != 0)
                {
                    labels[rec_num] += " ";
                }

                labels[rec_num] += entry->prefix()+value_str+entry->suffix();
            }
        }

        var_count++;

        if (var_count == break_item_cnt)
            var_count = 0;
    }

    return labels;
}

void DBOLabelDefinition::labelDefinitionChangedSlot ()
{
    assert (db_object_);

    emit db_object_->labelDefinitionChangedSignal();
}
