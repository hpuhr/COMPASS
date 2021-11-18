#include "asterixjsonparser.h"

#include "compass.h"
#include "buffer.h"
#include "configuration.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "util/json.h"
#include "asteriximporttask.h"
#include "files.h"

#include <QTableView>

#include <jasterix/jasterix.h>
#include <jasterix/iteminfo.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>

using namespace std;
using namespace nlohmann;
using namespace Utils;


ASTERIXJSONParser::ASTERIXJSONParser(const std::string& class_id, const std::string& instance_id,
                                     Configurable* parent, ASTERIXImportTask& task)
    : Configurable(class_id, instance_id, parent,
                   "task_import_asterix_" + boost::algorithm::to_lower_copy(instance_id) + ".json"),
      task_(task)
{
    registerParameter("name", &name_, "");
    registerParameter("category", &category_, 0);

    registerParameter("db_object_name", &db_object_name_, "");

    assert(db_object_name_.size());

    if (!name_.size())
        name_ = db_object_name_;

    assert(name_.size());

    assert (task_.jASTERIX()->hasCategory(category_));
    item_info_ = task_.jASTERIX()->category(category_)->itemInfo();

    createSubConfigurables();

    todo_icon_ = QIcon(Files::getIconFilepath("todo.png").c_str());
    unknown_icon_ = QIcon(Files::getIconFilepath("todo_maybe.png").c_str());
    hint_icon_ = QIcon(Files::getIconFilepath("hint.png").c_str());
}

void ASTERIXJSONParser::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "JSONDataMapping")
    {
        data_mappings_.emplace_back(class_id, instance_id, *this);

        mapping_checks_dirty_ = true;
    }
    else
        throw std::runtime_error("ASTERIXJSONParser: generateSubConfigurable: unknown class_id " + class_id);
}

void ASTERIXJSONParser::doMappingChecks()
{
    loginf << "ASTERIXJSONParser: doMappingChecks";

    beginResetModel();

    // update non existing keys
    not_existing_json_keys_.clear();

    for (auto& map_it : data_mappings_)
    {
        if (!dbObject().hasVariable(map_it.dboVariableName()))
            map_it.dboVariableName("");

    }

    for (auto& map_it : data_mappings_)
    {
        if (!item_info_.count(map_it.jsonKey()))
            not_existing_json_keys_.insert(map_it.jsonKey());
    }

    // update not mapped json keys
    not_added_json_keys_.clear();

    for (auto& info_it : item_info_)
    {
        if (!hasJSONKeyMapped(info_it.first))
            not_added_json_keys_.push_back(info_it.first);
    }

    // update not mapped dbo vars
    not_added_dbo_variables_.clear();


    for (auto& dbovar_it : dbObject().variables())
    {
        if (!hasDBOVariableMapped(dbovar_it->name()))
            not_added_dbo_variables_.push_back(dbovar_it->name());
    }

    mapping_checks_dirty_ = false;

    endResetModel();
}

unsigned int ASTERIXJSONParser::totalEntrySize () const
{
    return data_mappings_.size() + not_added_json_keys_.size() + not_added_dbo_variables_.size();
}

bool ASTERIXJSONParser::existsJSONKeyInCATInfo(const std::string& key)
{
    return !not_existing_json_keys_.count(key);
}

bool ASTERIXJSONParser::hasJSONKeyMapped (const std::string& key)
{
    return std::find_if(data_mappings_.begin(), data_mappings_.end(),
                        [key](const JSONDataMapping& mapping) -> bool { return mapping.jsonKey() == key; })
            != data_mappings_.end();
}

bool ASTERIXJSONParser::hasDBOVariableMapped (const std::string& var_name)
{
    return std::find_if(data_mappings_.begin(), data_mappings_.end(),
                        [var_name](const JSONDataMapping& mapping) -> bool { return mapping.dboVariableName() == var_name; })
            != data_mappings_.end();
}

bool ASTERIXJSONParser::hasJSONKeyInMapping (const std::string& key)
{
    return std::find_if(data_mappings_.begin(), data_mappings_.end(),
                        [key](const JSONDataMapping& mapping) -> bool { return mapping.jsonKey() == key; })
            != data_mappings_.end();
}

unsigned int ASTERIXJSONParser::indexOfJSONKeyInMapping (const std::string& key)
{
    assert (hasJSONKeyInMapping(key));

    auto iter = std::find_if(data_mappings_.begin(), data_mappings_.end(),
                             [key](const JSONDataMapping& mapping) -> bool { return mapping.jsonKey() == key; });

    assert (iter != data_mappings_.end());

    unsigned int pos = iter - data_mappings_.begin();

    assert (pos < data_mappings_.size());

    return pos;
}

void ASTERIXJSONParser::selectMapping (unsigned int index)
{
    loginf << "ASTERIXJSONParser: selectMapping: index " << index;

    assert (widget_);
    widget_->selectModelRow(index);
}

void ASTERIXJSONParser::selectUnmappedDBOVariable (const std::string& name)
{
    assert (widget_);

    auto iter = find(not_added_dbo_variables_.begin(), not_added_dbo_variables_.end(), name);
    assert (iter != not_added_dbo_variables_.end());

    unsigned int pos = iter - not_added_dbo_variables_.begin();

    assert (pos < not_added_dbo_variables_.size());

    widget_->selectModelRow(data_mappings_.size() + not_added_json_keys_.size() + pos);
}

//void ASTERIXJSONParser::updateToChangedIndex (unsigned int row_index)
//{
//    assert (row_index < totalEntrySize());
//    emit dataChanged(index(row_index, 0), index(row_index, table_columns_.size()-1));
//}

ASTERIXJSONParser::EntryType ASTERIXJSONParser::entryType (unsigned int index) const
{
    if (index < data_mappings_.size()) // is actual mapping
        return ASTERIXJSONParser::EntryType::ExistingMapping;

    index -= data_mappings_.size();

    if (index < not_added_json_keys_.size())
        return ASTERIXJSONParser::EntryType::UnmappedJSONKey;

    index -= not_added_json_keys_.size();

    assert (index < not_added_dbo_variables_.size());

    return ASTERIXJSONParser::EntryType::UnmappedDBOVariable;
}

JSONDataMapping& ASTERIXJSONParser::mapping (unsigned int index)
{
    assert (entryType(index) == ASTERIXJSONParser::EntryType::ExistingMapping);

    return data_mappings_.at(index);
}

const JSONDataMapping& ASTERIXJSONParser::mapping (unsigned int index) const
{
    assert (entryType(index) == ASTERIXJSONParser::EntryType::ExistingMapping);

    return data_mappings_.at(index);
}

const std::string& ASTERIXJSONParser::unmappedJSONKey (unsigned int index) const
{
    assert (entryType(index) == ASTERIXJSONParser::EntryType::UnmappedJSONKey);
    return not_added_json_keys_.at(index - data_mappings_.size());
}

const std::string& ASTERIXJSONParser::unmappedDBOVariable (unsigned int index) const
{
    assert (entryType(index) == ASTERIXJSONParser::EntryType::UnmappedDBOVariable);
    return not_added_dbo_variables_.at(index - data_mappings_.size() - not_added_json_keys_.size());
}

const jASTERIX::CategoryItemInfo& ASTERIXJSONParser::categoryItemInfo() const
{
    return item_info_;
}

const std::vector<std::string>& ASTERIXJSONParser::notAddedJSONKeys() const
{
    return not_added_json_keys_;
}

DBObject& ASTERIXJSONParser::dbObject() const
{

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (!obj_man.existsObject(db_object_name_))
        throw runtime_error ("ASTERIXJSONParser: dbObject: dbobject '" + db_object_name_+ "' does not exist");
    else
        return obj_man.object(db_object_name_);
}

void ASTERIXJSONParser::initialize()
{
    loginf << "ASTERIXJSONParser: initialize: name " << name_;

    if (!initialized_)
    {
        for (auto& mapping : data_mappings_)
        {
            if (!mapping.active())
            {
                assert(!mapping.mandatory());
                continue;
            }

            mapping.initializeIfRequired();

            list_.addProperty(mapping.variable().name(), mapping.variable().dataType());
            var_list_.add(mapping.variable());
        }

        initialized_ = true;
    }
}

std::shared_ptr<Buffer> ASTERIXJSONParser::getNewBuffer() const
{
    assert(initialized_);
    return std::make_shared<Buffer>(list_, db_object_name_);
}

void ASTERIXJSONParser::appendVariablesToBuffer(Buffer& buffer) const
{
    assert(initialized_);

    for (auto& p_it : list_.properties())
    {
        if (!buffer.properties().hasProperty(p_it.name()))
        {
            buffer.addProperty(p_it.name(), p_it.dataType());
        }
    }
}

bool ASTERIXJSONParser::parseJSON(nlohmann::json& j, Buffer& buffer) const
{
    assert(initialized_);

    size_t row_cnt = buffer.size();

    bool parsed_any = false;

    logdbg << "ASTERIXJSONParser: parseJSON: single target report";
    assert(j.is_object());

    parsed_any = parseTargetReport(j, buffer, row_cnt);

    return parsed_any;
}

void ASTERIXJSONParser::createMappingStubs(nlohmann::json& j)
{
    assert(initialized_);


    logdbg << "ASTERIXJSONParser: createMappingStubs: single target report";
    assert(j.is_object());

    createMappingsFromTargetReport(j);

    return;
}

bool ASTERIXJSONParser::parseTargetReport(const nlohmann::json& tr, Buffer& buffer,
                                          size_t row_cnt) const
{
    PropertyDataType data_type;
    std::string current_var_name;

    bool mandatory_missing{false};

    for (const auto& map_it : data_mappings_)
    {
        if (!map_it.active())
        {
            assert(!map_it.mandatory());
            continue;
        }

        // logdbg << "setting data mapping key " << data_it.jsonKey();

        try
        {

            data_type = map_it.variable().dataType();
            current_var_name = map_it.variable().name();

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: bool " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<bool>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<bool>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::CHAR:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: char " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<char>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<char>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::UCHAR:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: uchar " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned char>(current_var_name));
                mandatory_missing = map_it.findAndSetValue(
                            tr, buffer.get<unsigned char>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::INT:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: int " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<int>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::UINT:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: uint " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned int>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<unsigned int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::LONGINT:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: long " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<long int>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<long int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::ULONGINT:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: ulong " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned long>(current_var_name));
                mandatory_missing = map_it.findAndSetValue(
                            tr, buffer.get<unsigned long>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::FLOAT:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: float " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<float>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<float>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::DOUBLE:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: double " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<double>(current_var_name));
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<double>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::STRING:
            {
                logdbg << "ASTERIXJSONParser: parseTargetReport: string " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<std::string>(current_var_name));

                //                if (buffer.dboName() == "Tracker" && current_var_name == "ground_bit")
                //                {
                //                    loginf << "ASTERIXJSONParser: parseTargetReport: string " << current_var_name
                //                           << " format '" << map_it.jsonValueFormat() << "' mand " << mandatory_missing;

                //                    mandatory_missing =
                //                        map_it.findAndSetValue(tr, buffer.get<std::string>(current_var_name), row_cnt, true);
                //                }
                //                else
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<std::string>(current_var_name), row_cnt);

                break;
            }
            default:
                logerr << "ASTERIXJSONParser: parseTargetReport: impossible for property type "
                         << Property::asString(data_type);
                throw std::runtime_error(
                            "JsonMapping: parseTargetReport: impossible property type " +
                            Property::asString(data_type));
            }

        }
        catch (exception& e)
        {
            logerr << "ASTERIXJSONParser: parseTargetReport: caught exception '" << e.what() << "' in \n'"
                     << tr.dump(4) << "' mapping " << map_it.jsonKey();
            throw e;
        }

        if (mandatory_missing)
        {
            // TODO make configurable
            logdbg << "ASTERIXJSONParser '" << name_ << "': parseTargetReport: mandatory variable '"
                     << current_var_name << "' missing in: \n"
                     << tr.dump(4);
            break;
        }
    }

    if (mandatory_missing)
    {
        // cleanup
        if (buffer.size() > row_cnt)
            buffer.cutToSize(row_cnt);
    }

    return !mandatory_missing;
}

void ASTERIXJSONParser::createMappingsFromTargetReport(const nlohmann::json& tr)
{
    checkIfKeysExistsInMappings("", tr);
}

void ASTERIXJSONParser::checkIfKeysExistsInMappings(const std::string& location,
                                                    const nlohmann::json& j, bool is_in_array)
{
    if (j.is_array())  // do map arrays
    {
        if (!j.size())  // do not map if empty
            return;

        bool j_array_contains_only_primitives = true;  // only

        for (auto& j_it : j.get<json::array_t>())  // iterate over array
        {
            if (j_it.is_object())  // only parse sub-objects
            {
                j_array_contains_only_primitives = false;
                checkIfKeysExistsInMappings(location, j_it, true);
            }
            else if (j_it.is_array())
                j_array_contains_only_primitives = false;
        }

        if (!j_array_contains_only_primitives)
            return;  // if objects inside, only parse objects
    }

    if (j.is_object())
    {
        for (auto& j_it : j.get<json::object_t>())
        {
            if (location.size())
                checkIfKeysExistsInMappings(location + "." + j_it.first, j_it.second, is_in_array);
            else
                checkIfKeysExistsInMappings(j_it.first, j_it.second, is_in_array);
        }
        return;
    }

    // loginf << "ASTERIXJSONParser: checkIfKeysExistsInMappings: checking key '" << location << "'";

    bool found = false;

    for (auto& map_it : data_mappings_)
    {
        if (map_it.jsonKey() == location)
        {
            found = true;

            if (!map_it.comment().size())
            {
                std::stringstream ss;

                ss << "Type " << j.type_name() << ", value " << j.dump();
                map_it.comment(ss.str());
            }
            break;
        }
    }

    if (!found)
    {
        loginf << "ASTERIXJSONParser: checkIfKeysExistsInMappings: creating new mapping for dbo "
                 << db_object_name_ << "'" << location << "' type " << j.type_name() << " value "
                 << j.dump() << " in array " << is_in_array;

        Configuration& new_cfg = configuration().addNewSubConfiguration("JSONDataMapping");
        new_cfg.addParameterString("json_key", location);
        new_cfg.addParameterString("db_object_name", db_object_name_);

        if (is_in_array)
            new_cfg.addParameterBool("in_array", true);

        std::stringstream ss;
        ss << "Type " << j.type_name() << ", value " << j.dump();
        new_cfg.addParameterString("comment", ss.str());

        generateSubConfigurable("JSONDataMapping", new_cfg.getInstanceId());
    }
}

bool ASTERIXJSONParser::hasMapping(unsigned int index) const
{
    return index < data_mappings_.size();
}
void ASTERIXJSONParser::removeMapping(unsigned int index)
{
    assert(hasMapping(index));

    JSONDataMapping& mapping = data_mappings_.at(index);

    loginf << "ASTERIXJSONParser: removeMapping: index " << index << " key " << mapping.jsonKey()
           << " instance " << mapping.instanceId();

    logdbg << "ASTERIXJSONParser: removeMapping: size " << data_mappings_.size();

    if (mapping.active() && mapping.initialized() && mapping.hasVariable())
    {
        if (list_.hasProperty(mapping.dboVariableName()))
            list_.removeProperty(mapping.dboVariableName());

        if (var_list_.hasVariable(mapping.variable()))
            var_list_.removeVariable(mapping.variable());
    }

    logdbg << "ASTERIXJSONParser: removeMapping: removing";
    data_mappings_.erase(data_mappings_.begin() + index);

    logdbg << "ASTERIXJSONParser: removeMapping: size " << data_mappings_.size();
}

const DBOVariableSet& ASTERIXJSONParser::variableList() const { return var_list_; }

ASTERIXJSONParserWidget* ASTERIXJSONParser::widget()
{
    if (mapping_checks_dirty_)
        doMappingChecks();

    if (!widget_)
    {
        widget_.reset(new ASTERIXJSONParserWidget(*this));
        assert(widget_);
    }

    return widget_.get();  // needed for qt integration, not pretty
}

std::string ASTERIXJSONParser::dbObjectName() const { return db_object_name_; }

void ASTERIXJSONParser::setMappingActive(JSONDataMapping& mapping, bool active)
{
    if (!mapping.initialized())
        mapping.initializeIfRequired();

    if (active)
    {
        list_.addProperty(mapping.variable().name(), mapping.variable().dataType());
        var_list_.add(mapping.variable());
    }
    else if (mapping.hasVariable())  // remove if was added
    {
        if (list_.hasProperty(mapping.variable().name()))
            list_.removeProperty(mapping.variable().name());
        if (var_list_.hasVariable(mapping.variable()))
            var_list_.removeVariable(mapping.variable());
    }

    mapping.active(active);
}

//void ASTERIXJSONParser::updateMappings()
//{
////    if (widget_)
////        widget_->updateMappingsGrid();
//}

std::string ASTERIXJSONParser::name() const { return name_; }

void ASTERIXJSONParser::name(const std::string& name) { name_ = name; }

unsigned int ASTERIXJSONParser::category() const
{
    return category_;
}

// item stuff

int ASTERIXJSONParser::rowCount(const QModelIndex& parent) const
{
    assert (!mapping_checks_dirty_);

    return totalEntrySize();
}

int ASTERIXJSONParser::columnCount(const QModelIndex& parent) const
{
    assert (!mapping_checks_dirty_);

    return table_columns_.size();
}

QVariant ASTERIXJSONParser::data(const QModelIndex& index, int role) const
{
    assert (!mapping_checks_dirty_);

    if (!index.isValid())
        return QVariant();

    assert (index.row() >= 0);
    unsigned int row = index.row();

    assert (row < totalEntrySize());

    assert (index.column() < table_columns_.size());
    std::string col_name = table_columns_.at(index.column()).toStdString();

    ASTERIXJSONParser::EntryType entry_type = entryType(row);

    switch (role)
    {
    case Qt::DisplayRole:
        //case Qt::EditRole:
    {
        logdbg << "ASTERIXJSONParser: data: display role: row " << index.row() << " col " << index.column();

        if (entry_type == ASTERIXJSONParser::EntryType::ExistingMapping)
        {
            const JSONDataMapping& current_mapping = mapping(row);

            logdbg << "ASTERIXJSONParser: data: got json key " << current_mapping.jsonKey();

            if (col_name == "JSON Key")
                return current_mapping.jsonKey().c_str();
            else if (col_name == "DBObject Variable")
                return current_mapping.dboVariableName().c_str();
            else
                return QVariant();
        }
        else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedJSONKey)
        {
            if (col_name == "JSON Key")
                return unmappedJSONKey(row).c_str();
            else
                return QVariant();
        }
        else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedDBOVariable)
        {
            if (col_name == "DBObject Variable")
                return unmappedDBOVariable(row).c_str();
            else
                return QVariant();
        }
        else
            return QVariant();
    }
    case Qt::DecorationRole:
    {
        if (entry_type == ASTERIXJSONParser::EntryType::ExistingMapping)
        {
            const JSONDataMapping& mapping = data_mappings_.at(index.row());

            logdbg << "ASTERIXJSONParser: data: got json key " << mapping.jsonKey();

            if (col_name == "JSON Key")
            {
                if (not_existing_json_keys_.count(mapping.jsonKey()))
                    return hint_icon_;
                else
                    return QVariant();
            }
            else
                return QVariant();
        }
        else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedJSONKey)
        {
            if (col_name == "JSON Key")
                return todo_icon_;
            else
                return QVariant();
        }
        else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedDBOVariable)
        {

            if (col_name == "DBObject Variable")
                return todo_icon_;
            else
                return QVariant();
        }
        else
            return QVariant();
    }
    default:
    {
        return QVariant();
    }
    }
}

QVariant ASTERIXJSONParser::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex ASTERIXJSONParser::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex ASTERIXJSONParser::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags ASTERIXJSONParser::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    //    if (table_columns_.at(index.column()) == "comment")
    //        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    //    else
    return QAbstractItemModel::flags(index);
}

bool ASTERIXJSONParser::mappingChecksDirty() const
{
    return mapping_checks_dirty_;
}

void ASTERIXJSONParser::mappingChecksDirty(bool mapping_checks_dirty)
{
    mapping_checks_dirty_ = mapping_checks_dirty;
}
