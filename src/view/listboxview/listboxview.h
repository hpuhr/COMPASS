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

class ListBoxView : public View
{
    Q_OBJECT
  public slots:
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp) override;
    virtual void showViewPointSlot (const ViewableDataConfig* vp) override;
    //void allLoadingDoneSlot();

  signals:
    void showOnlySelectedSignal(bool value);
    void usePresentationSignal(bool value);
    void showAssociationsSignal(bool value);

  public:
    ListBoxView(const std::string& class_id, const std::string& instance_id, ViewContainer* w,
                ViewManager& view_manager);
    virtual ~ListBoxView() override;

    bool init() override;

    virtual void loadingStarted() override;
    virtual void loadedData(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset) override;
    virtual void loadingDone() override;

    virtual void clearData() override;


    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    /// @brief Returns the used data source
    ListBoxViewDataSource* getDataSource()
    {
        assert(data_source_);
        return data_source_;
    }

    ListBoxViewDataWidget* getDataWidget();

    virtual dbContent::VariableSet getSet(const std::string& dbcontent_name) override;

    bool usePresentation() const;
    void usePresentation(bool use_presentation);

    bool overwriteCSV() const;
    void overwriteCSV(bool overwrite_csv);

    bool showOnlySelected() const;
    void showOnlySelected(bool value);

    virtual void accept(LatexVisitor& v) override;

  protected:
    ListBoxViewWidget* widget_{nullptr};
    ListBoxViewDataSource* data_source_{nullptr};

    bool show_only_selected_{true};
    bool use_presentation_{true};

    bool overwrite_csv_{false}; // Overwrite during export, if not, it appends

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;
};

#endif /* LISTBOXVIEW_H_ */
