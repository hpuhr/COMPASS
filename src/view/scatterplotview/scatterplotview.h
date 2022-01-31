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

#ifndef SCATTERPLOTVIEW_H_
#define SCATTERPLOTVIEW_H_

#include "view.h"

class ScatterPlotViewWidget;
class ScatterPlotViewDataSource;
class ScatterPlotViewDataWidget;

namespace dbContent {
class Variable;
class MetaVariable;
}

class ScatterPlotView : public View
{
    Q_OBJECT
  public slots:
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp) override;
    virtual void showViewPointSlot (const ViewableDataConfig* vp) override;
    void allLoadingDoneSlot();

  signals:
    void showOnlySelectedSignal(bool value);
    void usePresentationSignal(bool value);
    void showAssociationsSignal(bool value);

  public:
    /// @brief Constructor
    ScatterPlotView(const std::string& class_id, const std::string& instance_id, ViewContainer* w,
                ViewManager& view_manager);
    /// @brief Destructor
    virtual ~ScatterPlotView() override;

    bool init() override;

    virtual void loadingStarted() override;
    virtual void loadedData(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset) override;
    virtual void loadingDone() override;

    virtual void clearData() override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    /// @brief Returns the used data source
    ScatterPlotViewDataSource* getDataSource()
    {
        assert(data_source_);
        return data_source_;
    }

    ScatterPlotViewDataWidget* getDataWidget();

    virtual dbContent::VariableSet getSet(const std::string& dbo_name) override;

    virtual void accept(LatexVisitor& v) override;

    bool hasDataVarX ();
    bool isDataVarXMeta ();
    dbContent::Variable& dataVarX();
    void dataVarX (dbContent::Variable& var);

    dbContent::MetaVariable& metaDataVarX();
    void metaDataVarX (dbContent::MetaVariable& var);

    std::string dataVarXDBO() const;
    std::string dataVarXName() const;

    bool hasDataVarY ();
    bool isDataVarYMeta ();
    dbContent::Variable& dataVarY();
    void dataVarY (dbContent::Variable& var);

    dbContent::MetaVariable& metaDataVarY();
    void metaDataVarY (dbContent::MetaVariable& var);

    std::string dataVarYDBO() const;
    std::string dataVarYName() const;

protected:
    /// For data display
    ScatterPlotViewWidget* widget_{nullptr};
    /// For data loading
    ScatterPlotViewDataSource* data_source_{nullptr};

    std::string data_var_x_dbo_;
    std::string data_var_x_name_;

    std::string data_var_y_dbo_;
    std::string data_var_y_name_;

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;

    void updateStatus();
};

#endif /* SCATTERPLOTVIEW_H_ */
