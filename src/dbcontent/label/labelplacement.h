
#pragma once

#include "labelplacement_defs.h"

#include <string>
#include <vector>
#include <map>

#include <QRectF>

#include <boost/optional.hpp>

class QImage;

/**
 */
class LabelPlacementEngine
{
public:
    typedef label_placement::Method    Method;
    typedef label_placement::ForceType ForceType;
    typedef label_placement::Label     Label;

    struct Settings
    {
        typedef std::vector<QRectF> Objects;

        Method method = Method::ForceBased;

        QRectF roi; //TODO 

        //settings for methods 'ForceBasedXXX'
        ForceType fb_force_type      = ForceType::Simple;
        int       fb_max_iter        = 500;   //maximum number of iterations
        double    fb_precision       = 0.01;  //precision = convergence limit
        double    fb_expand_x        = 1.2;   //bounding box expansion in x direction (TODO: per avoidance type)
        double    fb_expand_y        = 1.1;   //bounding box expansion in y direction (TODO: per avoidance type)
        bool      fb_avoid_labels    = true;  //avoid other labels (should be on)
        bool      fb_avoid_anchors   = false; //avoid anchor locations (= positions the labels are attached to)
        bool      fb_avoid_objects   = false; //avoid objects added as 'fb_additional_objects'
        double    fb_weight_labels   = 0.25;  //force weight for label avoidance (TODO: separate x and y part)
        double    fb_weight_anchors  = 0.5;   //force weight for anchor avoidance (TODO: separate x and y part)
        double    fb_weight_objects  = 0.25;  //force weight for object avoidance (TODO: separate x and y part)
        double    fb_anchor_radius   = 0.0;   //repel radius around anchors if 'fb_avoid_anchors' is on
        Objects   fb_additional_objects;      //additional objects to avoid, must be enabled by 'fb_avoid_objects'
    };

    struct TestConfig
    { 
        int width  = 1024;
        int height = 768;

        int num_objects = 100;
        int interval_ms = 1000;
        int speed_px    = 10;

        int radius_px      = 5;
        int label_w_px     = 40;
        int label_h_px     = 15;
        int label_offs_px  = 15;

        bool flip_y = false;
    };

    struct TestLabel
    {
        std::string txt;

        boost::optional<double> x_init;
        boost::optional<double> y_init;

        double x;
        double y;
        double w;
        double h;
        double x_anchor;
        double y_anchor;
        double dirx;
        double diry;
        double speed;
    };

    LabelPlacementEngine();
    virtual ~LabelPlacementEngine() = default;

    void addLabel(const std::string& id, 
                  double x_anchor, 
                  double y_anchor,
                  double w,
                  double h,
                  bool active = true, 
                  double* x_init = nullptr, 
                  double* y_init = nullptr);
    const Label& getLabel(size_t idx) const;
    const Label* getLabel(const std::string& id) const;
    void removeLabel(size_t idx);
    void removeLabel(const std::string& id);
    void clearLabels();

    bool placeLabels();

    Settings& settings() { return settings_; } 

    void showData(const TestConfig& test_config) const;
    void runTest(const TestConfig& test_config) const;

private:
    void revertPlacements();
    bool placeLabelsForceBased();
    bool placeLabelsSpringBased();

    void convertToScreen(const TestConfig& test_config, 
                         std::vector<TestLabel>& test_labels) const;
    void runTest(const std::vector<TestLabel>& test_labels,
                 const TestConfig& test_config) const;
    void renderTestFrame(QImage& img, 
                         const std::vector<TestLabel>& labels,
                         const TestConfig& test_config) const;

    std::vector<Label>            labels_;
    std::map<std::string, size_t> label_map_;

    Settings                      settings_;
};
