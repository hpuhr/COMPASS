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
    void changedForeignKeySlot ();
    void changedLocalKeySlot();
    void changedMetaTableSlot();
    void changedShortNameColumnSlot ();
    void changedNameColumnSlot ();
    void changedSacColumnSlot ();
    void changedSicColumnSlot ();
    void changedLatitudeColumnSlot ();
    void changedLongitudeColumnSlot ();
    void changedAltitudeColumnSlot ();

    /// @brief Updates data source local key selection
    void updateLocalKeySlot();
    /// @brief Updates data source meta table name selection
    void updateMetaTableSlot ();
    /// @brief Updates data source foreign key selection
    void updateForeignKeySlot ();
    /// @brief Updates data source name column selection
    void updateShortNameColumnSlot ();
    void updateNameColumnSlot ();
    void updateSacColumnSlot ();
    void updateSicColumnSlot ();
    void updateLatitudeColumnSlot ();
    void updateLongitudeColumnSlot ();
    void updateAltitudeColumnSlot ();

//signals:
//    void definitionChangedSignal();
public:
    DBODataSourceDefinitionWidget(DBObject& object, DBODataSourceDefinition& definition, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~DBODataSourceDefinitionWidget();

private:
    DBObject& object_;
    DBODataSourceDefinition &definition_;
    DBSchemaManager &schema_manager_;

    /// @brief Add new data source local key selection
    QComboBox* local_key_box_{nullptr};
    /// @brief Add new data source meta table selection
    QComboBox* meta_name_box_{nullptr};
    /// @brief Add new data source foreign key selection
    QComboBox* foreign_key_box_{nullptr};
    /// @brief Add new data source foreign name selection
    QComboBox* short_name_box_{nullptr};
    QComboBox* name_box_{nullptr};
    QComboBox* sac_box_{nullptr};
    QComboBox* sic_box_{nullptr};
    QComboBox* latitude_box_{nullptr};
    QComboBox* longitude_box_{nullptr};
    QComboBox* altitude_box_{nullptr};

    /// @brief Updates a variable selection box
    void updateVariableSelectionBox (QComboBox* box, const std::string& schema_name, const std::string& meta_table_name, const std::string& value, bool empty_allowed=false);

};

#endif // DBODATASOURCEWIDGET_H
