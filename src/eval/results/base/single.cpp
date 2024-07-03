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

#include "eval/results/base/single.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/section_id.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/sectioncontenttext.h"

#include "eval/requirement/base/base.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "sectorlayer.h"

#include "viewpoint.h"
#include "viewpointgenerator.h"

using namespace std;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

const std::string Single::tr_details_table_name_ {"Target Reports Details"};
const std::string Single::target_table_name_     {"Targets"};

const int Single::AnnotationPointSizeOverview  = 8;
const int Single::AnnotationPointSizeHighlight = 12;
const int Single::AnnotationPointSizeError     = 16;
const int Single::AnnotationPointSizeOk        = 10;

const int Single::AnnotationLineWidthHighlight = 4;
const int Single::AnnotationLineWidthError     = 4;
const int Single::AnnotationLineWidthOk        = 2;

const QColor Single::AnnotationColorHighlight = Qt::yellow;
const QColor Single::AnnotationColorError     = QColor("#FF6666");
const QColor Single::AnnotationColorOk        = QColor("#66FF66");

/**
*/
Single::Single(const std::string& type, 
               const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement,
               const SectorLayer& sector_layer,
               unsigned int utn,
               const EvaluationTargetData* target,
               EvaluationManager& eval_man,
               const EvaluationDetails& details)
:   Base   (type, result_id, requirement, sector_layer, eval_man)
,   utn_   (utn   )
,   target_(target)
{
    setDetails(details);

    annotation_type_names_[AnnotationType::TypeHighlight] = "Selected";
    annotation_type_names_[AnnotationType::TypeError    ] = "Errors";
    annotation_type_names_[AnnotationType::TypeOk       ] = "OK";
}

/**
*/
Single::~Single() {}

/**
*/
unsigned int Single::utn() const
{
    return utn_;
}

/**
*/
const EvaluationTargetData* Single::target() const
{
    assert (target_);
    return target_;
}

/**
*/
void Single::setInterestFactor(double factor)
{
    interest_factor_ = factor;

    assert (target_);

    target_->addInterestFactor(getRequirementSectionID(), factor);
}

/**
*/
void Single::updateUseFromTarget()
{
    use_ = (resultUsable() && target_->use());
}

/**
*/
void Single::updateResult()
{
    Base::updateResult();

    updateUseFromTarget();
}

/**
*/
std::string Single::getTargetSectionID()
{
    return EvaluationResultsReport::SectionID::targetID(utn_);
}

/**
*/
std::string Single::getTargetRequirementSectionID ()
{
    return EvaluationResultsReport::SectionID::targetResultID(utn_, *this);
}

/**
*/
std::string Single::getRequirementSectionID () const // TODO hack
{
    if (eval_man_.settings().report_split_results_by_mops_)
    {
        string tmp = target()->mopsVersionStr();

        if (!tmp.size())
            tmp = "Unknown";

        tmp = "MOPS "+tmp+" Sum";

        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+tmp+":"+requirement_->name();
    }
    else if (eval_man_.settings().report_split_results_by_aconly_ms_)
    {
        string tmp = "Primary";

        if (target()->isModeS())
            tmp = "Mode S";
        else if (target()->isModeACOnly())
            tmp = "Mode A/C";
        else
            assert (target()->isPrimaryOnly());

        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+tmp+" Sum"+":"+requirement_->name();
    }
    else
    {
        return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":Sum:"+requirement_->name();
    }
}

/**
*/
void Single::addToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "Single: addToReport: " <<  requirement_->name();

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

/**
*/
void Single::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    addTargetToOverviewTable(tgt_overview_section, target_table_name_);

    if (eval_man_.settings().report_split_results_by_mops_ || 
        eval_man_.settings().report_split_results_by_aconly_ms_) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        addTargetToOverviewTable(sum_section, target_table_name_);
    }
}

/**
*/
void Single::addTargetToOverviewTable(EvaluationResultsReport::Section& section, 
                                      const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        unsigned int sort_column;
        auto headers = targetTableHeaders(&sort_column);

        auto sort_order = targetTableSortOrder();

        section.addTable(table_name, headers.size(), headers, true, sort_column, sort_order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    auto values = targetTableValues();

    assert((int)values.size() == target_table.columnCount());

    target_table.addRow(values, this, { utn_ });
}

/**
*/
void Single::addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    //generate details overview table
    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table = utn_req_section.getTable("details_overview_table");

    //add common infos
    auto common_infos = targetInfosCommon();

    for (const auto& info : common_infos)
        utn_req_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    //add custom infos
    auto infos = targetInfos();

    for (const auto& info : infos)
        utn_req_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    //add condition result
    bool failed;
    auto infos_condition = targetConditionInfos(failed);

    for (const auto& info : infos_condition)
        utn_req_table.addRow({ info.info_name, info.info_comment, info.info_value }, this);

    if (failed)
    {
        // mark utn section as with issue
        root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); 
        utn_req_section.perTargetWithIssues(true);
    }

    //generate overview figure?
    if (hasIssues())
    {
        utn_req_section.addFigure("target_errors_overview", 
                                  "Target Errors Overview",
                                  [this](void) { return this->viewableData(); });
    }
    else
    {
        utn_req_section.addText("target_errors_overview_no_figure");
        utn_req_section.getText("target_errors_overview_no_figure").addText(
                    "No target errors found, therefore no figure was generated.");
    }

    //generate details table
    generateDetailsTable(utn_req_section);
}

/**
*/
void Single::generateDetailsTable(EvaluationResultsReport::Section& utn_req_section)
{
    //init table if needed
    if (!utn_req_section.hasTable(tr_details_table_name_))
    {
        auto headers = detailHeaders();

        utn_req_section.addTable(tr_details_table_name_, headers.size(), headers);
    }

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    //setup on-demand callback
    utn_req_details_table.setCreateOnDemand(
        [this, &utn_req_details_table](void)
        {
            //create details on demand

            auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int didx0, int didx1)
            {
                auto values = detailValues(detail, parent_detail);

                assert((int)values.size() == utn_req_details_table.columnCount());

                utn_req_details_table.addRow(values, this, QPoint(didx0, didx1));
            };

            this->iterateDetails(func);
        } );
}

/**
*/
std::vector<std::string> Single::targetTableHeadersCommon() const
{
    return { "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max" };
}

/**
*/
std::vector<std::string> Single::targetTableHeadersOptional() const
{
    return {};
}

/**
*/
std::vector<std::string> Single::targetTableHeaders(unsigned int* sort_column) const
{
    auto headers          = targetTableHeadersCommon();
    auto headers_custom   = targetTableHeadersCustom();
    auto headers_optional = targetTableHeadersOptional();

    size_t result_value_idx = headers.size() + headers_custom.size();

    //add custom headers
    headers.insert(headers.end(), headers_custom.begin(), headers_custom.end());

    //add result value name
    headers.push_back(requirement_->getConditionResultNameShort(false));

    //add optional headers
    headers.insert(headers.end(), headers_optional.begin(), headers_optional.end());

    //set sort column
    if (sort_column)
    {
        const int sort_column_custom = targetTableCustomSortColumn();
        *sort_column = (sort_column_custom >= 0 ? (unsigned int)sort_column_custom : result_value_idx);
    }

    return headers;
}

/**
*/
Qt::SortOrder Single::targetTableSortOrder() const
{
    //per default infer sort order from requirement
    return requirement_->resultSortOrder();
}

/**
*/
std::vector<QVariant> Single::targetTableValuesCommon() const
{
    return { utn_, 
             target_->timeBeginStr().c_str(), 
             target_->timeEndStr().c_str(),
             target_->acidsStr().c_str(), 
             target_->acadsStr().c_str(),
             target_->modeACodesStr().c_str(), 
             target_->modeCMinStr().c_str(), 
             target_->modeCMaxStr().c_str() };
}

/**
*/
std::vector<QVariant> Single::targetTableValuesOptional() const
{
    return {};
}

/**
*/
std::vector<QVariant> Single::targetTableValues() const
{
    auto values          = targetTableValuesCommon();
    auto values_custom   = targetTableValuesCustom();
    auto values_optional = targetTableValuesOptional();

    //add custom values
    values.insert(values.end(), values_custom.begin(), values_custom.end());

    //add result value
    values.push_back(resultValue());

    //add optional values
    values.insert(values.end(), values_optional.begin(), values_optional.end());

    return values;
}

/**
*/
std::vector<Single::TargetInfo> Single::targetInfosCommon() const
{
    return { { "UTN"         , "Unique Target Number"           , utn_                             },
             { "Begin"       , "Begin time of target"           , target_->timeBeginStr().c_str()  },
             { "End"         , "End time of target"             , target_->timeEndStr().c_str()    },
             { "Callsign"    , "Mode S target identification(s)", target_->acidsStr().c_str()      }, 
             { "Target Addr.", "Mode S target address(es)"      , target_->acadsStr().c_str()      },
             { "Mode 3/A"    , "Mode 3/A code(s)"               , target_->modeACodesStr().c_str() }, 
             { "Mode C Min"  , "Minimum Mode C code [ft]"       , target_->modeCMinStr().c_str()   },
             { "Mode C Max"  , "Maximum Mode C code [ft]"       , target_->modeCMaxStr().c_str()   },
             { "Use"         , "To be used in results"          , use_                             } };
}

/**
*/
std::vector<Single::TargetInfo> Single::targetConditionInfos(bool& failed) const
{
    std::vector<TargetInfo> infos;

    QVariant result_val = resultValue();

    infos.push_back({ requirement_->getConditionResultNameShort(true).c_str(), 
                       requirement_->getConditionResultName().c_str(), 
                       result_val });

    infos.push_back({"Condition", "", requirement_->getConditionStr().c_str() });

    string result {"Unknown"};

    if (result_val.isValid())
        result = conditionResultString();

    infos.push_back({"Condition Fulfilled", "", result.c_str()});

    if (requirement_->mustHoldForAnyTarget().has_value())
        infos.emplace_back("Must hold for any target ", "", requirement_->mustHoldForAnyTarget().value());

    failed = (result == "Failed");

    return infos;
}

/**
*/
bool Single::hasReference (const EvaluationResultsReport::SectionContentTable& table, 
                           const QVariant& annotation) const
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;
}

/**
*/
std::string Single::reference(const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const
{
    assert (hasReference(table, annotation));
    return EvaluationResultsReport::SectionID::createForTargetResult(utn_, *this);
}

/**
*/
bool Single::hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const
{
    //check validity of annotation index
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && detailIndex(annotation).has_value())
        return true;
    
    return false;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Single::viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                               const QVariant& annotation) const
{
    assert (hasViewableData(table, annotation));

    if (table.name() == target_table_name_)
    {
        //return target overview viewable
        return createViewable(AnnotationOptions().overview());
    }
    else if (table.name() == tr_details_table_name_ && annotation.isValid())
    {
        //obtain detail key from annotation
        auto detail_key = detailIndex(annotation);
        assert(detail_key.has_value());

        //return target detail viewable
        return createViewable(AnnotationOptions().highlight(detail_key.value()));
    }

    return nullptr;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Single::viewableData() const
{
    //per default return target overview annotations
    return createViewable(AnnotationOptions().overview());
}

/**
 * Retrieves (and possibly creates) a json point coordinate array for the given annotation type.
*/
nlohmann::json& Single::annotationPointCoords(nlohmann::json& annotations_json, AnnotationType type, bool overview) const
{
    nlohmann::json& annotation = getOrCreateAnnotation(annotations_json, type, overview);

    auto& feat_json = ViewPointGenAnnotation::getFeatureJSON(annotation, 1);

    return ViewPointGenFeaturePointGeometry::getCoordinatesJSON(feat_json);
}

/**
 * Retrieves (and possibly creates) a json line coordinate array for the given annotation type.
*/
nlohmann::json& Single::annotationLineCoords(nlohmann::json& annotations_json, AnnotationType type, bool overview) const
{
    nlohmann::json& annotation = getOrCreateAnnotation(annotations_json, type, overview);
    
    auto& feat_json = ViewPointGenAnnotation::getFeatureJSON(annotation, 0);

    return ViewPointGenFeaturePointGeometry::getCoordinatesJSON(feat_json);
}

/**
 * Retrieves (and possibly creates) a json annotation for the given annotation type.
*/
nlohmann::json& Single::getOrCreateAnnotation(nlohmann::json& annotations_json, AnnotationType type, bool overview) const
{
    assert (annotations_json.is_array());

    string anno_name = annotation_type_names_.at(type);

    logdbg << "Single: getOrCreateAnnotation: anno_name '" << anno_name << "' overview " << overview;
    
    auto insertAnnotation = [ & ] (const std::string& name,
                                   unsigned int position,
                                   const QColor& symbol_color,
                                   const ViewPointGenFeaturePoints::Symbol& point_symbol,
                                   const QColor& point_color,
                                   int point_size,
                                   const QColor& line_color,
                                   int line_width)
    {
        logdbg << "Single: getOrCreateAnnotation: size " << annotations_json.size()
               << " creating '" << name << "' at pos " << position;

        for (unsigned int cnt=0; cnt < annotations_json.size(); ++cnt)
            logdbg << "Single: getOrCreateAnnotation: start: index " << cnt <<" '" << annotations_json.at(cnt).at("name") << "'";

        annotations_json.insert(annotations_json.begin() + position, json::object()); // errors
        assert (position < annotations_json.size());

        nlohmann::json& annotation_json = annotations_json.at(position);

        ViewPointGenAnnotation annotation(name);
        annotation.setSymbolColor(symbol_color);

        //ATTENTION: !ORDER IMPORTANT!

        // lines
        std::unique_ptr<ViewPointGenFeatureLines> feature_lines;
        feature_lines.reset(new ViewPointGenFeatureLines(line_width, {}, {}, false));
        feature_lines->setColor(line_color);

        annotation.addFeature(std::move(feature_lines));

        // symbols
        std::unique_ptr<ViewPointGenFeaturePoints> feature_points;
        feature_points.reset(new ViewPointGenFeaturePoints(point_symbol, point_size, {}, {}, false));
        feature_points->setColor(point_color);

        annotation.addFeature(std::move(feature_points));

        //convert to json
        annotation.toJSON(annotation_json);

        for (unsigned int cnt=0; cnt < annotations_json.size(); ++cnt)
            logdbg << "Single: getOrCreateAnnotation: end: index " << cnt <<" '" << annotations_json.at(cnt).at("name") << "'";
    };

    //ATTENTION: !ORDER IMPORTANT!

    const std::string FieldName = ViewPointGenAnnotation::AnnotationFieldName;

    if (type == AnnotationType::TypeHighlight)
    {
        // should be first

        if (!annotations_json.size() || annotations_json.at(0).at(FieldName) != anno_name)
        { 
            // empty or not selected, add at first position
            insertAnnotation(anno_name, 
                             0,
                             AnnotationColorHighlight,
                             overview ? ViewPointGenFeaturePoints::Symbol::Circle : ViewPointGenFeaturePoints::Symbol::Border,
                             AnnotationColorHighlight,
                             overview ? AnnotationPointSizeOverview : AnnotationPointSizeHighlight,
                             AnnotationColorHighlight,
                             AnnotationLineWidthHighlight);
        }

        assert (annotations_json.at(0).at(FieldName) == anno_name);
        return annotations_json.at(0);
    }
    else if (type == AnnotationType::TypeError)
    {
        // should be first or second

        bool         insert_needed = false;
        unsigned int anno_pos      = 0;

        if (!annotations_json.size()) // empty, add at first pos
        {
            insert_needed = true;
            anno_pos = 0;
        }
        else if (annotations_json.at(0).at(FieldName) != anno_name)
        {
            if (annotations_json.size() > 1 && annotations_json.at(1).at(FieldName) == anno_name)
            {
                // found at second pos
                anno_pos = 1;
            }
            else if (annotations_json.at(0).at(FieldName) == annotation_type_names_.at(AnnotationType::TypeHighlight))
            {
                // insert after
                insert_needed = true;
                anno_pos = 1;
            }
            else
            {
                assert (annotations_json.at(0).at(FieldName) == annotation_type_names_.at(AnnotationType::TypeOk));
                // insert before
                insert_needed = true;
                anno_pos = 0;
            }
        }
        else
        {
            anno_pos = 0; // only for you
        }

        if (insert_needed)
        {
            insertAnnotation(anno_name, 
                             anno_pos,
                             AnnotationColorError,
                             overview ? ViewPointGenFeaturePoints::Symbol::Circle : ViewPointGenFeaturePoints::Symbol::BorderThick,
                             AnnotationColorError,
                             overview ? AnnotationPointSizeOverview : AnnotationPointSizeError,
                             AnnotationColorError,
                             AnnotationLineWidthError);
        }

        assert (annotations_json.at(anno_pos).at(FieldName) == anno_name);
        return annotations_json.at(anno_pos);
    }
    else
    {
        assert (type == AnnotationType::TypeOk);
        // should be last

        if (!annotations_json.size() || annotations_json.back().at(FieldName) != anno_name)
        {
            //unsigned int anno_pos = annotations_json.size() ? annotations_json.size() - 1 : 0;

            // insert at last
            insertAnnotation(anno_name, annotations_json.size(),
                             AnnotationColorOk,
                             overview ? ViewPointGenFeaturePoints::Symbol::Circle : ViewPointGenFeaturePoints::Symbol::Border,
                             AnnotationColorOk,
                             overview ? AnnotationPointSizeOverview : AnnotationPointSizeOk,
                             AnnotationColorOk,
                             AnnotationLineWidthOk);
        }

        assert (annotations_json.back().at(FieldName) == anno_name);
        return annotations_json.back();
    }
}

/**
 * Adds a new position to the position annotation.
*/
void Single::addAnnotationPos(nlohmann::json& annotations_json, 
                              const EvaluationDetail::Position& pos,
                              AnnotationType type) const
{
    auto& coords = annotationPointCoords(annotations_json, type);
    coords.push_back(pos.asVector());
}

/**
 * Adds a new line to the line annotation.
*/
void Single::addAnnotationLine(nlohmann::json& annotations_json,
                               const EvaluationDetail::Position& pos0,
                               const EvaluationDetail::Position& pos1,
                               AnnotationType type) const
{
    auto& coords = annotationLineCoords(annotations_json, type);
    coords.push_back(pos0.asVector());
    coords.push_back(pos1.asVector());
}

/**
*/
void Single::addAnnotationDistance(nlohmann::json& annotations_json,
                                   const EvaluationDetail& detail,
                                   AnnotationType type,
                                   bool add_line,
                                   bool add_ref) const
{
    assert(detail.numPositions() >= 1);

    addAnnotationPos(annotations_json, detail.position(0), type);

    if (add_ref  && detail.numPositions() >= 2) addAnnotationPos(annotations_json, detail.position(1), type);
    if (add_line && detail.numPositions() >= 2) addAnnotationLine(annotations_json, detail.position(0), detail.position(1), type);
}

/**
*/
std::unique_ptr<Single::EvaluationDetails> Single::generateDetails() const
{
    if (!requirement_)
        return {};

    if (!eval_man_.getData().hasTargetData(utn_))
        return {};

    const auto& data = eval_man_.getData().targetData(utn_);

    auto result = requirement_->evaluate(data, requirement_, sector_layer_);
    if (!result)
        return {};

    auto eval_details = new EvaluationDetails;
    *eval_details = result->getDetails();

    return std::unique_ptr<EvaluationDetails>(eval_details);
}

/**
*/
std::vector<double> Single::getValues(int value_id, const boost::optional<int>& check_value_id) const
{
    std::vector<double> values;
    values.reserve(totalNumDetails());

    auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int idx0, int idx1)
    {
        //if provided, the check value must be an existing bool value
        if (check_value_id.has_value())
        {
            auto value_valid = detail.getValueAsOrAssert<bool>(check_value_id.value());

            //value not valid => skip
            if (!value_valid)
                return;
        }

        auto value = detail.getValue(value_id);

        //value might not be set => skip
        if (!value.isValid())
            return;

        //value must be convertable to double
        bool ok;
        double v = value.toDouble(&ok);
        assert(ok);

        values.push_back(v);
    };

    iterateDetails(func);

    values.shrink_to_fit();

    return values;
}

/**
 * Generates binary grid values from a certain bool detail param.
 */
void Single::addValuesToGridBinary(Grid2D& grid, int detail_key, bool invert, bool use_ref_pos) const
{
    const auto& details = getDetails();

    auto is_ok = [ & ] (size_t idx)
    {
        auto check_passed = details[ idx ].getValueAs<bool>(detail_key);
        assert(check_passed.has_value());

        bool ok = ((!invert &&  check_passed.value()) ||
                   ( invert && !check_passed.value()));
        
        return ok;
    };

    addValuesToGridBinary(grid, details, is_ok, use_ref_pos);
}

/**
*/
void Single::addValuesToGridBinary(Grid2D& grid, 
                                   const EvaluationDetails& details, 
                                   const std::function<bool(size_t)>& is_ok, 
                                   bool use_ref_pos) const
{
    assert(is_ok);

    for (size_t i = 0; i < details.size(); ++i)
    {
        const auto& d = details[ i ];

        if (use_ref_pos && d.numPositions() == 1) // no ref pos
            continue;

        assert (d.numPositions() >= 1 && d.numPositions() <= 2);

        bool ok = is_ok(i);

        //interpolate between 0 = green and 1 = red
        grid.addValue(d.position(use_ref_pos ? 1 : 0).longitude_,
                      d.position(use_ref_pos ? 1 : 0).latitude_,
                      ok ? 0.0 : 1.0);
    }
}

/**
 * Grid layer definition suitable for getGridValuesBinary().
 */
Single::LayerDefinition Single::getGridLayerDefBinary() const
{
    Single::LayerDefinition def;
    def.value_type = grid2d::ValueType::ValueTypeMax;
    def.render_settings.color_map.create(QColor(0, 255, 0), QColor(255, 0, 0), 2, ColorMap::Type::Binary);

    def.render_settings.pixels_per_cell = eval_man_.settings().grid_pixels_per_cell;
    def.render_settings.min_value = 0.0;
    def.render_settings.max_value = 1.0;
    
    return def;
}

/**
 * Returns a detail key given an annotation id.
 * Default behaviour will return a first level detail index.
*/
boost::optional<Base::DetailIndex> Single::detailIndex(const QVariant& annotation) const
{
    if (!annotation.isValid())
        return {};

    QPoint index = annotation.toPoint();
    if (index.isNull())
        return {};

    DetailIndex dindex = { index.x(), index.y() };
    if (!detailIndexValid(dindex))
        return {};

    return dindex;
}

/**
 * Iterates over all details for which skip_func does not return true, and applies the given functor func.
 * Default behaviour will iterate over all first level details.
*/
void Single::iterateDetails(const DetailFunc& func,
                            const DetailSkipFunc& skip_func) const
{
    assert(func);

    auto nesting_mode = detailNestingMode();

    const auto& details0 = getDetails();

    if (nesting_mode == DetailNestingMode::Vector)
    {
        for (size_t i = 0; i < details0.size(); ++i)
            if (!skip_func || !skip_func(details0[ i ]))
                func(details0[ i ], nullptr, (int)i, -1);
    }
    else if (nesting_mode == DetailNestingMode::Nested)
    {
        for (size_t i = 0; i < details0.size(); ++i)
        {
            if (!details0[ i ].hasDetails())
                continue;

            const auto& details1 = details0[ i ].details();

            for (size_t j = 0; j < details1.size(); ++j)
                if (!skip_func || !skip_func(details1[ j ]))
                    func(details1[ j ], &details0[ i ], (int)i, (int)j);
        }
    }
    else if (nesting_mode == DetailNestingMode::SingleNested)
    {
        assert(details0.size() == 1);

        if (details0[ 0 ].hasDetails())
        {
            const auto& details1 = details0[ 0 ].details();

            for (size_t i = 0; i < details1.size(); ++i)
                if (!skip_func || !skip_func(details1[ i ]))
                    func(details1[ i ], &details0[ 0 ], 0, (int)i);
        }
    }
}

/**
*/
size_t Single::totalNumDetails() const
{
    auto nesting_mode = detailNestingMode();

    const auto& details0 = getDetails();

    if (nesting_mode == DetailNestingMode::Vector)
    {
        return details0.size();
    }
    else if (nesting_mode == DetailNestingMode::Nested)
    {
        size_t n = 0;

        for (size_t i = 0; i < details0.size(); ++i)
        {
            if (!details0[ i ].hasDetails())
                continue;

            const auto& details1 = details0[ i ].details();

            n += details1.size();
        }
    }
    else if (nesting_mode == DetailNestingMode::SingleNested)
    {
        assert(details0.size() == 1);

        return details0[ 0 ].hasDetails() ? details0[ 0 ].details().size() : 0;
    }

    return 0;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> Single::createBaseViewable() const
{
    return eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
}

/**
*/
Base::ViewableInfo Single::createViewableInfo(const AnnotationOptions& options) const
{
    assert(options.valid());

    Base::ViewableInfo info;
    info.viewable_type = options.viewable_type;

    const double BoundsEps = 1e-12;

    if (options.viewable_type == ViewableType::Overview)
    {
        //overview => iterate over failed details and compute bounds
        auto skip_func = [ this ] (const EvaluationDetail& detail)
        {
            return this->detailIsOk(detail);
        };

        QRectF bounds;

        auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int didx0, int didx1)
        {
            assert(detail.numPositions() >= 1);

            bounds |= detail.bounds(BoundsEps);
        };

        iterateDetails(func, skip_func);

        info.bounds = bounds;
    }
    else if (options.viewable_type == ViewableType::Highlight)
    {
        //highlight single detail
        const auto& d = getDetail(options.detail_index.value());

        assert(d.numPositions() >= 1);

        info.bounds    = d.bounds(BoundsEps);
        info.timestamp = d.timestamp();
    }
    else
    {
        bool valid_viewable_type = false;
        assert(valid_viewable_type);
    }

    return info;
}

/**
*/
void Single::createSumOverviewAnnotations(nlohmann::json& annotations_json,
                                          bool add_ok_details_to_overview) const
{
    createTargetAnnotations(annotations_json, TargetAnnotationType::SumOverview, {}, add_ok_details_to_overview);
}

/**
*/
void Single::createTargetAnnotations(nlohmann::json& annotations_json,
                                     TargetAnnotationType type,
                                     const boost::optional<DetailIndex>& detail_index,
                                     bool add_ok_details_to_overview) const
{
    if (type == TargetAnnotationType::SumOverview)
    {
        //add sum overview annotations
        auto skip_func = [ & ] (const EvaluationDetail& detail)
        {
            //skip none?
            if (add_ok_details_to_overview)
                return false;

            //skip ok details
            return detailIsOk(detail);
        };

        auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int didx0, int didx1)
        {
            addAnnotationForDetail(annotations_json, detail, TargetAnnotationType::SumOverview, detailIsOk(detail));
        };

        iterateDetails(func, skip_func);

        return;
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        //add target overview annotations
        auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int didx0, int didx1)
        {
            addAnnotationForDetail(annotations_json, detail, TargetAnnotationType::TargetOverview, detailIsOk(detail));
        };

        iterateDetails(func);
    }
    else if (type == TargetAnnotationType::Highlight)
    {
        assert(detail_index.has_value());

        //add target overview annotations?
        if (addOverviewAnnotationsToDetail())
        {
            auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int didx0, int didx1)
            {
                addAnnotationForDetail(annotations_json, detail, TargetAnnotationType::TargetOverview, detailIsOk(detail));
            };

            iterateDetails(func);
        }

        //add detail annotation
        const auto& d = getDetail(detail_index.value());
        addAnnotationForDetail(annotations_json, d, TargetAnnotationType::Highlight, detailIsOk(d));
    }
}

/**
*/
void Single::createAnnotations(nlohmann::json& annotations_json, const AnnotationOptions& options) const
{
    assert (options.valid());

    if (options.viewable_type == ViewableType::Overview)
        createTargetAnnotations(annotations_json, TargetAnnotationType::TargetOverview);
    else if (options.viewable_type == ViewableType::Highlight)
        createTargetAnnotations(annotations_json, TargetAnnotationType::Highlight, options.detail_index);
}

}
