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
}

void ASTERIXJSONParser::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "JSONDataMapping")
    {
        data_mappings_.emplace_back(class_id, instance_id, *this);
    }
    else
        throw std::runtime_error("ASTERIXJSONParser: generateSubConfigurable: unknown class_id " + class_id);
}

DBObject& ASTERIXJSONParser::dbObject() const
{
    assert(db_object_);
    return *db_object_;
}

void ASTERIXJSONParser::initialize()
{
    assert(!db_object_);

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (!obj_man.existsObject(db_object_name_))
        logwrn << "ASTERIXJSONParser: initialize: dbobject '" << db_object_name_
               << "' does not exist";
    else
        db_object_ = &obj_man.object(db_object_name_);

    assert(db_object_);

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
    assert(db_object_);
    return std::make_shared<Buffer>(list_, db_object_->name());
}

void ASTERIXJSONParser::appendVariablesToBuffer(Buffer& buffer) const
{
    assert(initialized_);
    assert(db_object_);

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
    size_t skipped_cnt = 0;

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

    if (mapping.active() && mapping.initialized())
    {
        if (list_.hasProperty(mapping.variable().name()))
            list_.removeProperty(mapping.variable().name());
        if (mapping.hasVariable() && var_list_.hasVariable(mapping.variable()))
            var_list_.removeVariable(mapping.variable());
    }

    logdbg << "ASTERIXJSONParser: removeMapping: removing";
    data_mappings_.erase(data_mappings_.begin() + index);

    logdbg << "ASTERIXJSONParser: removeMapping: size " << data_mappings_.size();
}

const DBOVariableSet& ASTERIXJSONParser::variableList() const { return var_list_; }

ASTERIXJSONParserWidget* ASTERIXJSONParser::widget()
{
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

void ASTERIXJSONParser::updateMappings()
{
//    if (widget_)
//        widget_->updateMappingsGrid();
}

std::string ASTERIXJSONParser::name() const { return name_; }

void ASTERIXJSONParser::name(const std::string& name) { name_ = name; }

unsigned int ASTERIXJSONParser::category() const
{
    return category_;
}

// item stuff

int ASTERIXJSONParser::rowCount(const QModelIndex& parent) const
{
    return data_mappings_.size();
}

int ASTERIXJSONParser::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QVariant ASTERIXJSONParser::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
        //case Qt::EditRole:
            {
                logdbg << "ASTERIXJSONParser: data: display role: row " << index.row() << " col " << index.column();

                assert (index.row() >= 0);
                assert (index.row() < data_mappings_.size());

                const JSONDataMapping& vp = data_mappings_.at(index.row());

                logdbg << "ASTERIXJSONParser: data: got json key " << vp.jsonKey();

                assert (index.column() < table_columns_.size());
                std::string col_name = table_columns_.at(index.column()).toStdString();

                if (col_name == "JSON Key")
                    return vp.jsonKey().c_str();
                else if (col_name == "DBObject Variable")
                    return vp.dboVariableName().c_str();
                else if (col_name == "Comment")
                    return vp.comment().c_str();
                else
                    return QVariant();
            }
        case Qt::DecorationRole:
            {
//                assert (index.column() < table_columns_.size());

//                if (table_columns_.at(index.column()) == "status")
//                {
//                    assert (index.row() >= 0);
//                    assert (index.row() < view_points_.size());

//                    const ViewPoint& vp = view_points_.at(index.row());

//                    const json& data = vp.data().at("status");
//                    assert (data.is_string());

//                    std::string status = data;

//                    if (status == "open")
//                        return open_icon_;
//                    else if (status == "closed")
//                        return closed_icon_;
//                    else if (status == "todo")
//                        return todo_icon_;
//                    else
//                        return unknown_icon_;
//                }
//                else
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
