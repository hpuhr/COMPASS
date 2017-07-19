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
    void loadingStartedSignal ();
    /// @brief Emitted when resulting buffer was delivered
    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

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

    /// @brief Sets use selection flag
    //void setUseSelection (bool use_selection) { use_selection_=use_selection; }
    /// @brief Returns use selection flag
    //bool getUseSelection () { return use_selection_; }

    bool usePresentation() const;
    void usePresentation(bool use_presentation);

    bool overwriteCSV() const;
    void overwriteCSV(bool overwrite_csv);

protected:
    /// Variable read list
    DBOVariableOrderedSet *set_;
    /// Use presentation
    bool use_presentation_;
    /// Overwrite during export, if not, it appends
    bool overwrite_csv_;

    /// Selected DBObject records
    ViewSelectionEntries &selection_entries_;

    virtual void checkSubConfigurables ();
};

#endif /* LISTBOXVIEWDATASOURCE_H_ */
