#ifndef ASTERIXJSONPARSER_H
#define ASTERIXJSONPARSER_H

#include "configurable.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "format.h"
#include "jsondatamapping.h"
#include "asterixjsonparserwidget.h"
#include "propertylist.h"
#include "stringconv.h"

#include <jasterix/iteminfo.h>

#include <QAbstractItemModel>
#include <QIcon>

#include <memory>
#include <string>

class DBObject;
class DBOVariable;
class Buffer;
class ASTERIXImportTask;

class ASTERIXJSONParser : public Configurable,  public QAbstractItemModel
{
    using MappingIterator = std::vector<JSONDataMapping>::iterator;

public:
    enum EntryType {
        ExistingMapping=0, UnmappedJSONKey, UnmappedDBOVariable
    };


    ASTERIXJSONParser(const std::string& class_id, const std::string& instance_id,
                      Configurable* parent, ASTERIXImportTask& task);

    DBObject& dbObject() const;

    MappingIterator begin() { return data_mappings_.begin(); }
    MappingIterator end() { return data_mappings_.end(); }
    bool hasMapping(unsigned int index) const;
    void removeMapping(unsigned int index);

    // returns true on successful parse
    bool parseJSON(nlohmann::json& j, Buffer& buffer) const;
    void createMappingStubs(nlohmann::json& j);

    const DBOVariableSet& variableList() const;

    bool initialized() const { return initialized_; }
    void initialize();

    std::shared_ptr<Buffer> getNewBuffer() const;
    void appendVariablesToBuffer(Buffer& buffer) const;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    ASTERIXJSONParserWidget* widget();

    std::string dbObjectName() const;

    void setMappingActive(JSONDataMapping& mapping, bool active);

    //void updateMappings();

    std::string name() const;
    void name(const std::string& name);

    unsigned int category() const;

    // item stuff

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool mappingChecksDirty() const;
    void mappingChecksDirty(bool mapping_checks_dirty);
    void doMappingChecks();  // to be called when total size is changed

    unsigned int totalEntrySize () const;
    bool existsJSONKeyInCATInfo(const std::string& key);
    bool hasJSONKeyMapped (const std::string& key);
    bool hasDBOVariableMapped (const std::string& var_name);

    bool hasJSONKeyInMapping (const std::string& key);
    unsigned int indexOfJSONKeyInMapping (const std::string& key);

    //void updateToChangedIndex (unsigned int index); // to be called when existing row is changed
    void selectMapping (unsigned int index);
    void selectUnmappedDBOVariable (const std::string& name);

    // index is not model index but directly in data_mappings_ + not_added_json_keys_ + not_added_dbo_variables_
    EntryType entryType (unsigned int index) const;
    JSONDataMapping& mapping (unsigned int index);
    const JSONDataMapping& mapping (unsigned int index) const;
    const std::string& unmappedJSONKey (unsigned int index) const;
    const std::string& unmappedDBOVariable (unsigned int index) const;

    const jASTERIX::CategoryItemInfo& categoryItemInfo() const;

    const std::vector<std::string>& notAddedJSONKeys() const;

private:
    ASTERIXImportTask& task_;
    std::string name_;
    unsigned int category_;

    std::string db_object_name_;

    jASTERIX::CategoryItemInfo item_info_;

    DBOVariableSet var_list_;

    bool initialized_{false};

    PropertyList list_;

    std::unique_ptr<ASTERIXJSONParserWidget> widget_;

    std::vector<JSONDataMapping> data_mappings_;
    bool mapping_checks_dirty_ {true};
    std::set<std::string> not_existing_json_keys_; // mapped keys not existing in cat info
    std::vector<std::string> not_added_json_keys_; // keys existing in cat info not in mappings
    std::vector<std::string> not_added_dbo_variables_; // existing dbovars not in mappings


    QStringList table_columns_ {"JSON Key", "DBObject Variable"};

    QIcon todo_icon_;
    QIcon unknown_icon_;
    QIcon hint_icon_;

    // returns true on successful parse
    bool parseTargetReport(const nlohmann::json& tr, Buffer& buffer, size_t row_cnt) const;
    void createMappingsFromTargetReport(const nlohmann::json& tr);

    void checkIfKeysExistsInMappings(const std::string& location, const nlohmann::json& tr,
                                     bool is_in_array = false);

protected:
    virtual void checkSubConfigurables() {}

};

#endif // ASTERIXJSONPARSER_H
