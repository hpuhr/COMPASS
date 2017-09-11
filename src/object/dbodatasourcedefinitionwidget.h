#ifndef DBODATASOURCEWIDGET_H
#define DBODATASOURCEWIDGET_H


#include <QWidget>

class QComboBox;
class DBObject;
class DBODataSourceDefinition;
class DBSchemaManager;

class DBODataSourceDefinitionWidget: public QWidget
{
    Q_OBJECT
public slots:
    /// @brief Updates data source local key selection
    void updateDSLocalKeySelection();
    /// @brief Updates data source meta table name selection
    void updateDSMetaTableNameSelection ();
    /// @brief Updates data source foreign key selection
    void updateDSForeignKeySelection ();
    /// @brief Updates data source name column selection
    void updateDSNameColumnSelection ();

signals:
    void definitionChangedSignal();
public:
    DBODataSourceDefinitionWidget(DBObject& object, DBODataSourceDefinition& definition, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~DBODataSourceDefinitionWidget();

private:
    DBObject& object_;
    DBODataSourceDefinition &definition_;
    DBSchemaManager &schema_manager_;

    /// @brief Add new data source local key selection
    QComboBox* ds_local_key_box_{nullptr};
    /// @brief Add new data source meta table selection
    QComboBox* ds_meta_name_box_{nullptr};
    /// @brief Add new data source foreign key selection
    QComboBox* ds_foreign_key_box_{nullptr};
    /// @brief Add new data source foreign name selection
    QComboBox* ds_foreign_name_box_{nullptr};

    /// @brief Updates a variable selection box
    void updateVariableSelectionBox (QComboBox* box, std::string schema_name, std::string meta_table_name);

};

#endif // DBODATASOURCEWIDGET_H
