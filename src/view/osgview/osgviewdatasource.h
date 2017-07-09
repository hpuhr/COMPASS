/*
 * OSGViewDataSource.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEWDATASOURCE_H_
#define OSGVIEWDATASOURCE_H_

#include <QObject>

#include "dbovariable.h"
#include "dbovariableorderedset.h"
#include "configurable.h"
#include "buffer.h"
#include "viewselection.h"


class Job;

/**
 * @brief Handles database queries and resulting data for OSGView
 *
 * Creates database queries for all contained DBObjects when updateData () is called and
 * emits signal updateData() when resulting buffer is delivered by callback. Stores Buffers
 * and handles cleanup.
 */
class OSGViewDataSource : public QObject, public Configurable
{
    Q_OBJECT
public slots:
    void loadingStartedSlot ();
    void newDataSlot (DBObject &object);
    void loadingDoneSlot (DBObject &object);

signals:
    void loadingStartedSignal ();
    /// @brief Emitted when resulting buffer was delivered
    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

public:
    /// @brief Constructor
    OSGViewDataSource(const std::string &class_id, const std::string &instance_id, Configurable *parent);
    /// @brief Destructor
    virtual ~OSGViewDataSource();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns variable read list
    DBOVariableOrderedSet *getSet () { return set_; }
    /// @brief Returns stored result Buffers
    //std::map <DB_OBJECT_TYPE, Buffer*> &getData () { return data_; }

    /// @brief Sets use filter flag
    void setUseFilters (bool use_filters) { use_filters_=use_filters; }
    /// @brief Returns use filter flag
    bool getUseFilters () { return use_filters_; }

    /// @brief Sets use selection flag
    void setUseSelection (bool use_selection) { use_selection_=use_selection; }
    /// @brief Returns use selection flag
    bool getUseSelection () { return use_selection_; }

    /// @brief Sets use order flags
    void setUseOrder (bool use_order) { use_order_=use_order; }
    /// @brief Return use order flag
    bool getUseOrder () { return use_order_; }

    /// @brief Sets order variable name
    void setOrderVariableName (std::string name) { order_variable_name_=name; }
    /// @brief Returns order variable name
    std::string getOrderVariableName () { return order_variable_name_; }

    /// @brief Sets order variable DBObject type
    //void setOrderVariableType (DB_OBJECT_TYPE type) { order_variable_type_int_=type; }
    /// @brief Returns order variable DBObject type
    //DB_OBJECT_TYPE getOrderVariableType () { return (DB_OBJECT_TYPE) order_variable_type_int_; }

    /// @brief Sets order ascending flag
    void setOrderAscending (bool asc) { order_ascending_=asc; }
    /// @brief Returns order ascending flag
    bool getOrderAscending () { return order_ascending_; }

    /// @brief Returns database view flag
    bool getDatabaseView () { return database_view_; }
    /// @brief Sets database view flags
    void setDatabaseView (bool database_view) { database_view_=database_view; }

protected:
    /// Variable read list
    DBOVariableOrderedSet *set_;
    /// Use filters flag
    bool use_filters_;
    /// Use selection flag
    bool use_selection_;
    /// Use order flag
    bool use_order_;
    /// Order variable DBObject type
    unsigned int order_variable_type_int_;
    /// Order variable name
    std::string order_variable_name_;
    /// Order ascending flag
    bool order_ascending_;
    /// Use database view
    bool database_view_;

    /// Selected DBObject records
    ViewSelectionEntries &selection_entries_;

    virtual void checkSubConfigurables ();
};

#endif /* OSGVIEWDATASOURCE_H_ */
