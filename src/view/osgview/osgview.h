/*
 * OSGView.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEW_H_
#define OSGVIEW_H_

#include "view.h"
#include "viewselection.h"

class OSGViewWidget;
class OSGViewDataSource;

/**
 * @brief View for textual inspection of database contents
 *
 * Has a different data loading mechanism that other Views because it launches separate queries.
 * Data contents are shown in a OSGViewWidget. In the configuration area a number
 * of parameters of the SQL query can be set, the most prominent being the list of read variables.
 * Those configure a OSGViewDataSource, which is used for data loading.
 *
 * When the 'Update' button is clicked, a separate query is executed (asynchronously) and when done,
 * the resulting Buffers are delivered by callback and displayed.
 */
class OSGView : public View
{
    Q_OBJECT
public slots:
    /// @brief Is executed when update button is clicked
    //void updateData ();

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

    void mapNameChangedSignal (const std::string name);
    void mapOpacityChangedSignal (float opacity);
    void dataOpacityChangedSignal (float opacity);
    void useHeightChangedSignal (bool use);
    void useHeightScaleChangedSignal (bool use);
    void heightScaleFactorChangedSignal (float factor);
    void clampHeightChangedSignal (bool use);

public:
    /// @brief Constructor
    OSGView(const std::string& class_id, const std::string& instance_id, ViewContainer *w, ViewManager &view_manager);
    /// @brief Destructor
    virtual ~OSGView();

    void update( bool atOnce=false );
    void clearData();
    bool init();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns the used data source
    OSGViewDataSource *getDataSource () { assert (data_source_); return data_source_; }

    virtual DBOVariableSet getSet (const std::string &dbo_name);

    float mapOpacity() const;
    void mapOpacity(float opacity);

    float dataOpacity() const;
    void dataOpacity(float opacity);

    std::string mapName() const;
    void mapName(const std::string &map_name);

    float heightScaleFactor() const;
    void heightScaleFactor(float height_scale_factor);

    bool useHeight() const;
    void useHeight(bool use_height);

    bool clampHeight() const;
    void clampHeight(bool clamp_height_on_ground);

    bool useHeightScale() const;
    void useHeightScale(bool use_height_scale);

protected:
    /// For data display
    OSGViewWidget* widget_;
    /// For data loading
    OSGViewDataSource *data_source_;

    std::string map_name_;
    float map_opacity_;
    float data_opacity_;

    bool use_height_;
    bool use_height_scale_;
    float height_scale_factor_;
    bool clamp_height_;

    virtual void checkSubConfigurables ();
};

#endif /* OSGVIEW_H_ */
