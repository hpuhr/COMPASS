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

#pragma once

#include "referencecalculator.h"
#include "test_target.h"

#include <vector>

#include <QColor>

class ReconstructorBase;

namespace dbContent 
{
    class ReconstructorTarget;
}

class ViewPointGenAnnotation;

namespace reconstruction
{
    class KalmanEstimator;
}

class QColor;

namespace rec_annotations
{
    /**
     * Target report annotation data.
    */
    struct TRAnnotation
    {
        boost::posix_time::ptime         ts;
        Eigen::Vector2d                  pos_wgs84;
        boost::optional<Eigen::Vector2d> speed_pos;
        boost::optional<Eigen::Vector2d> acc_pos;
        boost::optional<Eigen::Vector3d> accuracy;
        boost::optional<Eigen::Vector2d> pos_mm;
        boost::optional<double>          speed;
        boost::optional<double>          accel;
        boost::optional<double>          Q_var;
        bool                             reset = false;
        bool                             proj_change = false;
    };

    /**
     * Annotation for a single RTS submodel step.
    */
    struct RTSStepAnnotation
    {
        TRAnnotation state1;
        TRAnnotation state1_smooth;
        TRAnnotation state0;
        TRAnnotation state0_smooth;
        QColor       color;
    };

    /**
     * Annotation for RTS smoother debugging.
    */
    struct RTSAnnotation
    {
        std::vector<RTSStepAnnotation> rts_step_models;
        RTSStepAnnotation              rts_step;
        std::string                    extra_info;
    };

    /**
     * Annotation for an IMM step.
    */
    struct IMMAnnotation
    {
        std::vector<TRAnnotation> imm_step_models;
        TRAnnotation              imm_step;
        std::string               extra_info;
    };

    /**
     * Reconstruction annotation style.
    */
    struct AnnotationStyle
    {
        AnnotationStyle() {}
        AnnotationStyle(const QColor& base_color,
                        float point_size,
                        float line_width)
        :   base_color_(base_color)
        ,   point_size_(point_size)
        ,   line_width_(line_width)
        {}

        QColor base_color_;
        float  point_size_;
        float  line_width_;
    };

    /**
     * Annotation data for a single slice reconstruction run.
    */
    struct AnnotationData
    {
        std::string                  name;
        AnnotationStyle              style;
        AnnotationStyle              style_osg;
        std::vector<TRAnnotation>    annotations;
        std::vector<size_t>          slice_begins;
        std::vector<Eigen::Vector2d> fail_positions;
        std::vector<Eigen::Vector2d> skip_positions;
        std::vector<double>          Q_vars;
        std::vector<bool>            is_reset_pos;
        std::vector<bool>            is_proj_change_pos;
        std::vector<RTSAnnotation>   rts_annotations;
    };
}

/**
*/
class SimpleReferenceCalculator
{
public:
    typedef ReferenceCalculatorSettings Settings;

    typedef std::vector<reconstruction::Measurement>                       Measurements;
    typedef std::multimap<boost::posix_time::ptime, unsigned long>         TargetReports;
    typedef std::map<unsigned int, std::vector<reconstruction::Reference>> References;

    SimpleReferenceCalculator(ReconstructorBase& reconstructor);
    virtual ~SimpleReferenceCalculator();

    void prepareForNextSlice();
    void prepareForCurrentSlice();
    bool computeReferences();

    void reset();

    void createAnnotations();

    Settings& settings() { return settings_; }
    
    static std::vector<std::vector<reconstruction::Measurement>> splitMeasurements(const Measurements& measurements,
                                                                                   double max_dt);
    static const QColor ColorMeasurements;
    static const QColor ColorKalman;
    static const QColor ColorKalmanSmoothed;

    static const float PointSizeOSG;
    static const float PointSizeMeasurements;
    static const float PointSizeKalman;
    static const float PointSizeKalmanSmoothed;

    static const float LineWidthBase;
                                
private:
    /**
     * Data for target export.
    */
    struct TargetExportData
    {
        TestTarget test_target;
        boost::optional<boost::posix_time::ptime> t0;
        double lat_;
        double lon_;
    };

    /**
     * Target reference data.
    */
    struct TargetReferences
    {
        void reset();
        void resetCounts();

        unsigned int utn;

        std::vector<reconstruction::Measurement> measurements;
        std::vector<kalman::KalmanUpdate>        updates;
        std::vector<kalman::KalmanUpdate>        updates_smooth;
        std::vector<double>                      updates_smooth_Qvars;
        std::vector<reconstruction::Reference>   references;

        boost::optional<kalman::KalmanUpdate>    init_update;
        boost::optional<size_t>                  start_index;

        size_t num_updates                 = 0;
        size_t num_updates_valid           = 0;
        size_t num_updates_failed          = 0;
        size_t num_updates_failed_numeric  = 0;
        size_t num_updates_failed_badstate = 0;
        size_t num_updates_failed_other    = 0;
        size_t num_updates_skipped         = 0;
        size_t num_smooth_steps_failed     = 0;
        size_t num_smoothing_failed        = 0;
        size_t num_interp_steps_failed     = 0;

        std::map<std::string, rec_annotations::AnnotationData> annotation_data;

        std::unique_ptr<TargetExportData> export_data;
    };

    enum class InitRecResult 
    {
        NoMeasurements = 0,
        NoStartIndex,
        Success
    };

    void resetDataStructs();
    void updateInterpOptions();

    void generateMeasurements();
    void generateTargetMeasurements(const dbContent::ReconstructorTarget& target);
    void generateLineMeasurements(const dbContent::ReconstructorTarget& target,
                                  unsigned int dbcontent_id,
                                  unsigned int sensor_id,
                                  unsigned int line_id,
                                  const TargetReports& target_reports);
    
    void addMeasurements(unsigned int utn,
                         unsigned int dbcontent_id, 
                         Measurements& measurements);
    void preprocessMeasurements(unsigned int dbcontent_id, 
                                Measurements& measurements);
    void interpolateMeasurements(Measurements& measurements, 
                                 const reconstruction::InterpOptions& options) const;
    
    void reconstructMeasurements();
    InitRecResult initReconstruction(TargetReferences& refs);
    void reconstructMeasurements(TargetReferences& refs);

    void updateReferences();

    boost::posix_time::ptime getJoinThreshold() const;

    bool writeTargetData(TargetReferences& refs,
                         const std::string& fn);

    bool shallAddAnnotationData() const;

    void addAnnotationData(TargetReferences& target_references,
                           const std::string& name,
                           const rec_annotations::AnnotationStyle& style,
                           const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                           const std::vector<rec_annotations::TRAnnotation>& annotations,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs,
                           const std::vector<QPointF>* fail_pos = nullptr,
                           const std::vector<QPointF>* skip_pos = nullptr,
                           std::vector<rec_annotations::RTSAnnotation>* rts_annotations = nullptr) const;
    void addAnnotationData(TargetReferences& target_references,
                           const reconstruction::KalmanEstimator& estimator, 
                           const std::string& name,
                           const rec_annotations::AnnotationStyle& style,
                           const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                           const std::vector<kalman::KalmanUpdate>& updates,
                           const std::vector<unsigned int>& used_mms,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs,
                           const std::vector<QPointF>* fail_pos = nullptr,
                           const std::vector<QPointF>* skip_pos = nullptr,
                           std::vector<kalman::RTSDebugInfo>* rts_debug_infos = nullptr) const;
    void addAnnotationData(TargetReferences& target_references,
                           const std::string& name,
                           const rec_annotations::AnnotationStyle& style,
                           const boost::optional<rec_annotations::AnnotationStyle>& style_osg,
                           const std::vector<reconstruction::Measurement>& measurements,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs) const;
    void createAnnotations(ViewPointGenAnnotation* annotation, 
                           const TargetReferences& target_references) const;

    ReconstructorBase& reconstructor_;

    Settings settings_;

    int  slice_idx_     = -1;
    bool is_last_slice_ = false;

    std::map<unsigned int, TargetReferences>              references_;
    std::map<unsigned int, reconstruction::InterpOptions> interp_options_;
};
