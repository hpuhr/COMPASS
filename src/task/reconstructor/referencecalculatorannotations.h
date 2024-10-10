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

#include <vector>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <Eigen/Core>

#include <QColor>
#include <QPointF>

class ViewPointGenAnnotation;
class ReconstructorBase;

namespace reconstruction
{
    struct Measurement;
    struct Reference;
    struct KalmanProjectionHandler;
    class  KalmanEstimator;
}

namespace kalman
{
    struct KalmanUpdate;
    struct RTSDebugInfo;
}

namespace refcalc_annotations
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
    boost::optional<Eigen::Vector2d> pos_mm_uncorr;
    boost::optional<double>          speed;
    boost::optional<double>          accel;
    boost::optional<double>          Q_var;
    boost::optional<std::string>     info;
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
 * Annotation for a single IMM submodel step.
*/
struct IMMStepAnnotation
{
    TRAnnotation state;
    QColor       color;
};

/**
 * Annotation for an IMM step.
*/
struct IMMAnnotation
{
    std::vector<IMMStepAnnotation> imm_step_models;
    IMMStepAnnotation              imm_step;
    std::string                    extra_info;
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
    std::vector<IMMAnnotation>   imm_annotations;
    std::vector<Eigen::Vector2d> interp_input_positions;
};

/**
*/
class ReferenceCalculatorAnnotations
{
public:
    ReferenceCalculatorAnnotations();
    virtual ~ReferenceCalculatorAnnotations();

    bool hasAnnotations() const;

    void setReconstructor(const ReconstructorBase* reconstructor);

    void addAnnotationData(const std::string& name,
                           const AnnotationStyle& style,
                           const boost::optional<AnnotationStyle>& style_osg,
                           const std::vector<TRAnnotation>& annotations,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs,
                           const std::vector<QPointF>* fail_pos = nullptr,
                           const std::vector<QPointF>* skip_pos = nullptr,
                           const std::vector<Eigen::Vector2d>* interp_input_positions = nullptr,
                           const std::vector<IMMAnnotation>* imm_annotations = nullptr,
                           const std::vector<RTSAnnotation>* rts_annotations = nullptr);
    void addAnnotationData(const reconstruction::KalmanEstimator& estimator, 
                           const std::string& name,
                           const AnnotationStyle& style,
                           const boost::optional<AnnotationStyle>& style_osg,
                           const std::vector<kalman::KalmanUpdate>& updates,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs,
                           bool debug_imm,
                           const std::vector<reconstruction::Measurement>* mms = nullptr,
                           const std::vector<unsigned int>* used_mms = nullptr,            
                           const std::vector<QPointF>* fail_pos = nullptr,
                           const std::vector<QPointF>* skip_pos = nullptr,
                           const std::vector<Eigen::Vector2d>* interp_input_positions = nullptr,
                           const std::vector<kalman::RTSDebugInfo>* rts_debug_infos = nullptr);
    void addAnnotationData(const std::string& name,
                           const AnnotationStyle& style,
                           const boost::optional<AnnotationStyle>& style_osg,
                           const std::vector<reconstruction::Measurement>& measurements,
                           const boost::optional<boost::posix_time::ptime>& t0,
                           const boost::optional<boost::posix_time::ptime>& t1,
                           size_t offs);
    void createAnnotations(ViewPointGenAnnotation* annotation) const;

private:
    TRAnnotation createTRAnnotation(const reconstruction::Measurement& mm,
                                    const boost::optional<Eigen::Vector2d>& speed_pos,
                                    const boost::optional<Eigen::Vector2d>& accel_pos,
                                    const boost::optional<Eigen::Vector2d>& mm_pos,
                                    const boost::optional<std::string>& info_str) const;
    TRAnnotation createTRAnnotation(const reconstruction::Reference& ref,
                                    const boost::optional<Eigen::Vector2d>& speed_pos,
                                    const boost::optional<Eigen::Vector2d>& accel_pos,
                                    const boost::optional<Eigen::Vector2d>& mm_pos) const;
    TRAnnotation createTRAnnotation(const kalman::KalmanUpdate& update,
                                    const reconstruction::KalmanEstimator& estimator,
                                    reconstruction::KalmanProjectionHandler& phandler,
                                    const Eigen::Vector2d& proj_center,
                                    int submodel_idx,
                                    bool debug,
                                    const std::string& name) const;
    RTSAnnotation createRTSAnnotation(const kalman::RTSDebugInfo& rts_debug_info,
                                      const reconstruction::KalmanEstimator& estimator,
                                      reconstruction::KalmanProjectionHandler& phandler) const;
    IMMAnnotation createIMMAnnotation(const kalman::KalmanUpdate& imm_update,
                                      const reconstruction::KalmanEstimator& estimator,
                                      reconstruction::KalmanProjectionHandler& phandler) const;

    const ReconstructorBase* reconstructor_ = nullptr;

    std::map<std::string, AnnotationData> annotation_data_;
};

} //namespace refcalc_annotations
