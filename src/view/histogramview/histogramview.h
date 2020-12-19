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

#ifndef HISTOGRAMVIEW_H_
#define HISTOGRAMVIEW_H_

#include "view.h"
#include "viewselection.h"

class HistogramViewWidget;
class HistogramViewDataSource;
class HistogramViewDataWidget;

/**
 * @brief View for textual inspection of database contents
 *
 * Has a different data loading mechanism that other Views because it launches separate queries.
 * Data contents are shown in a HistogramViewWidget. In the configuration area a number
 * of parameters of the SQL query can be set, the most prominent being the list of read variables.
 * Those configure a HistogramViewDataSource, which is used for data loading.
 *
 * When the 'Update' button is clicked, a separate query is executed (asynchronously) and when done,
 * the resulting Buffers are delivered by callback and displayed.
 */
class HistogramView : public View
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
    HistogramView(const std::string& class_id, const std::string& instance_id, ViewContainer* w,
                ViewManager& view_manager);
    /// @brief Destructor
    virtual ~HistogramView() override;

    void update(bool atOnce = false) override;
    void clearData() override;
    bool init() override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    /// @brief Returns the used data source
    HistogramViewDataSource* getDataSource()
    {
        assert(data_source_);
        return data_source_;
    }

    HistogramViewDataWidget* getDataWidget();

    virtual DBOVariableSet getSet(const std::string& dbo_name) override;

    virtual void accept(LatexVisitor& v) override;

  protected:
    /// For data display
    HistogramViewWidget* widget_{nullptr};
    /// For data loading
    HistogramViewDataSource* data_source_{nullptr};

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;
};

#endif /* HISTOGRAMVIEW_H_ */
