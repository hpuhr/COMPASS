/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LISTBOXVIEW_H_
#define LISTBOXVIEW_H_

#include "view.h"
#include "listboxviewdatasource.h"

class ListBoxViewWidget;
//class ListBoxViewDataSource;
class ListBoxViewDataWidget;

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
    //    /// @brief Is executed when selection is changed. Does nothing.
    //    void selectionChanged();
    //    /// @brief Is executed when selection is to be cleared. Does nothing.
    //    void selectionToBeCleared();
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp) override;
    virtual void showViewPointSlot (const ViewableDataConfig* vp) override;
    void allLoadingDoneSlot();

  signals:
    //    /// @brief Is emitted when selection was changed locally
    //    void setSelection (const ViewSelectionEntries& entries);
    //    /// @brief Is emitted when somthing was added to the selection
    //    void addSelection (const ViewSelectionEntries& entries);
    //    /// @brief Is emitted when selection should be cleared
    //    void clearSelection();

    void showOnlySelectedSignal(bool value);
    void usePresentationSignal(bool value);
    void showAssociationsSignal(bool value);

  public:
    /// @brief Constructor
    ListBoxView(const std::string& class_id, const std::string& instance_id, ViewContainer* w,
                ViewManager& view_manager);
    /// @brief Destructor
    virtual ~ListBoxView() override;

    void update(bool atOnce = false) override;
    void clearData() override;
    bool init() override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    /// @brief Returns the used data source
    ListBoxViewDataSource* getDataSource()
    {
        assert(data_source_);
        return data_source_;
    }

    ListBoxViewDataWidget* getDataWidget();

    virtual DBOVariableSet getSet(const std::string& dbo_name) override;

    bool usePresentation() const;
    void usePresentation(bool use_presentation);

    bool overwriteCSV() const;
    void overwriteCSV(bool overwrite_csv);

    bool showOnlySelected() const;
    void showOnlySelected(bool value);

    bool canShowAssociations() const;
    bool showAssociations() const;
    void showAssociations(bool show_associations);

    virtual void accept(LatexVisitor& v) override;

  protected:
    /// For data display
    ListBoxViewWidget* widget_{nullptr};
    /// For data loading
    ListBoxViewDataSource* data_source_{nullptr};

    bool show_only_selected_{true};
    /// Use presentation
    bool use_presentation_{true};
    /// Overwrite during export, if not, it appends
    bool overwrite_csv_{false};
    bool can_show_associations_{false};
    bool show_associations_{false};

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;
};

#endif /* LISTBOXVIEW_H_ */
