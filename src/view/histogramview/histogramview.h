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

class HistogramViewWidget;
class HistogramViewDataSource;
class HistogramViewDataWidget;

namespace dbContent {
class Variable;
class MetaVariable;
}

class HistogramView : public View
{
    Q_OBJECT
public:
    /// @brief Constructor
    HistogramView(const std::string& class_id, const std::string& instance_id, ViewContainer* w,
                ViewManager& view_manager);
    /// @brief Destructor
    virtual ~HistogramView() override;

    virtual void loadingDone() override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    /// @brief Returns the used data source
    HistogramViewDataSource* getDataSource()
    {
        assert(data_source_);
        return data_source_;
    }

    virtual dbContent::VariableSet getSet(const std::string& dbcontent_name) override;

    virtual void accept(LatexVisitor& v) override;

    bool useLogScale() const;
    void useLogScale(bool value);

    bool hasDataVar ();
    bool isDataVarMeta ();
    dbContent::Variable& dataVar();
    void dataVar (dbContent::Variable& var);

    dbContent::MetaVariable& metaDataVar();
    void metaDataVar (dbContent::MetaVariable& var);

    std::string dataVarDBO() const;
    std::string dataVarName() const;

    bool showResults() const;
    void showResults(bool value);

    std::string evalResultGrpReq() const;
    void evalResultGrpReq(const std::string& value);

    std::string evalResultsID() const;
    void evalResultsID(const std::string& value);

    bool hasResultID() const;

    bool hasViewPoint () { return current_view_point_ != nullptr; }
    const ViewableDataConfig& viewPoint() { assert (hasViewPoint()); return *current_view_point_; }

    static const std::string ParamDataVarDBO;
    static const std::string ParamDataVarName;
    static const std::string ParamUseLogScale;

public slots:
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp) override;
    virtual void showViewPointSlot (const ViewableDataConfig* vp) override;

    void resultsChangedSlot();

signals:
    void showOnlySelectedSignal(bool value);
    void usePresentationSignal(bool value);
    void showAssociationsSignal(bool value);

protected:
    friend class LatexVisitor;

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;

    virtual bool init_impl() override;

    virtual ViewUpdate onConfigurationChanged_impl(const std::vector<std::string>& changed_params) override;

    void onShowResultsChanged();

    HistogramViewDataWidget* getDataWidget();

    /// For data display
    HistogramViewWidget*     widget_{nullptr};
    /// For data loading
    HistogramViewDataSource* data_source_{nullptr};

    std::string data_var_dbo_;
    std::string data_var_name_;

    bool use_log_scale_ {false};

    bool        show_results_{false}; // no results at first
    std::string eval_results_grpreq_;
    std::string eval_results_id_;

    const ViewableDataConfig* current_view_point_ {nullptr};
};

#endif /* HISTOGRAMVIEW_H_ */
