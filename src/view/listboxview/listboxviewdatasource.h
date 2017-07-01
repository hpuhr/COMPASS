/*
 * ListBoxViewDataSource.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef LISTBOXVIEWDATASOURCE_H_
#define LISTBOXVIEWDATASOURCE_H_

#include <QObject>

#include "dbovariable.h"
#include "dbovariableorderedset.h"
#include "configurable.h"
#include "buffer.h"
#include "viewselection.h"


class Job;

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
    void loadingStartedSlot ();
    void newDataSlot (DBObject &object);
    void loadingDoneSlot (DBObject &object);

signals:
    /// @brief Emitted when resulting buffer was delivered
    void updateData (unsigned int dbo_type, Buffer *buffer);

public:
    /// @brief Constructor
    ListBoxViewDataSource(const std::string &class_id, const std::string &instance_id, Configurable *parent);
    /// @brief Destructor
    virtual ~ListBoxViewDataSource();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns variable read list
    DBOVariableOrderedSet *getSet () { return set_; }
    /// @brief Returns stored result Buffers
    //std::map <DB_OBJECT_TYPE, Buffer*> &getData () { return data_; }

    /// @brief Sets use filter flag
    void setUseFilters (bool use_filters) { use_filters_=use_filters; }
    /// @brief Returns use filter flag
    bool getUseFilters () { return use_filters_; }

    /// @brief sets limit minimum
    void setLimitMin (unsigned int min) { limit_min_ = min; }
    /// @brief Returns limit minimum
    unsigned int getLimitMin () { return limit_min_; }

    /// @brief Sets limit maximum
    void setLimitMax (unsigned int max) { limit_max_ = max; }
    /// @brief Return limit maximum
    unsigned int getLimitMax () { return limit_max_; }

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

    /// @brief Creates asynchronous read jobs for all DBObjects
    void updateData ();

    /// @brief Callback function for correctly finished read jobs
    void readInfoDone (Job *job);
    /// @brief Callback function for interrupted read jobs
    void readInfoObsolete (Job *job);

    /// @brief Returns database view flag
    bool getDatabaseView () { return database_view_; }
    /// @brief Sets database view flags
    void setDatabaseView (bool database_view) { database_view_=database_view; }

protected:
    /// Variable read list
    DBOVariableOrderedSet *set_;
    /// Use filters flag
    bool use_filters_;
    /// Limit minimum
    unsigned int limit_min_;
    /// Limit maximum
    unsigned int limit_max_;
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

    /// Container with result data buffers
    //std::map <DB_OBJECT_TYPE, Buffer*> data_;
    /// Selected DBObject records
    ViewSelectionEntries &selection_entries_;

    virtual void checkSubConfigurables ();
};

#endif /* LISTBOXVIEWDATASOURCE_H_ */
