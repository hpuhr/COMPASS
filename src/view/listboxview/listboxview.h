/*
 * ListBoxView.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef LISTBOXVIEW_H_
#define LISTBOXVIEW_H_

#include "view.h"
#include "viewselection.h"

class ListBoxViewWidget;
class ListBoxViewDataSource;

/**
 * @brief View for textual inspection of database contents
 *
 * Has a different data loading mechanism that other Views because it launches separate queries.
 * Data contents are shown in a ListBoxViewWidget. In the configuration area a number
 * of parameters of the SQL query can be set, the most prominent being the list of read variables.
 * Those configure a ListBoxViewDataSource, which is used for data loading.
 *
 * When the 'Update' button is clicked, a separate query is executed (asynchronously) and when done,
 * the resulting Buffers are delivered by callback and displayed.
 */
class ListBoxView : public View
{
    Q_OBJECT
public slots:
    /// @brief Is executed when selection is changed. Does nothing.
    void selectionChanged();
    /// @brief Is executed when selection is to be cleared. Does nothing.
    void selectionToBeCleared();

signals:
    /// @brief Is emitted when selection was changed locally
    void setSelection (const ViewSelectionEntries& entries);
    /// @brief Is emitted when somthing was added to the selection
    void addSelection (const ViewSelectionEntries& entries);
    /// @brief Is emitted when selection should be cleared
    void clearSelection();

public:
    /// @brief Constructor
    ListBoxView(const std::string& class_id, const std::string& instance_id, ViewContainer *w, ViewManager &view_manager);
    /// @brief Destructor
    virtual ~ListBoxView();

    void update( bool atOnce=false );
    void clearData();
    bool init();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns the used data source
    ListBoxViewDataSource *getDataSource () { assert (data_source_); return data_source_; }

    virtual DBOVariableSet getSet (const std::string &dbo_name);

protected:
    /// For data display
    ListBoxViewWidget* widget_;
    /// For data loading
    ListBoxViewDataSource *data_source_;

    virtual void checkSubConfigurables ();
};

#endif /* LISTBOXVIEW_H_ */
