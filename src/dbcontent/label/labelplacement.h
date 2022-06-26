
#include <string>
#include <vector>
#include <map>

#include <QRectF>

/**
 */
class LabelPlacementEngine
{
public:
    enum class Method
    {
        ForceBased = 0 //simple force based approach
    };

    struct Settings
    {
        typedef std::vector<QRectF> Objects;

        Method method = Method::ForceBased;

        QRectF roi; //TODO 

        //settings for method 'ForceBased'
        int     fb_max_iter        = 500;   //maximum number of iterations
        double  fb_precision       = 0.01;  //precision = convergence limit
        double  fb_expand_x        = 1.1;   //bounding box expansion in x direction (TODO: per avoidance type)
        double  fb_expand_y        = 1.1;   //bounding box expansion in y direction (TODO: per avoidance type)
        bool    fb_avoid_labels    = true;  //avoid other labels (should be on)
        bool    fb_avoid_anchors   = false; //avoid anchor locations (= positions the labels are attached to)
        bool    fb_avoid_objects   = false; //avoid objects added as 'fb_additional_objects'
        double  fb_weight_labels   = 0.5;  //force weight for label avoidance (TODO: separate x and y part)
        double  fb_weight_anchors  = 0.5;   //force weight for anchor avoidance (TODO: separate x and y part)
        double  fb_weight_objects  = 0.25;  //force weight for object avoidance (TODO: separate x and y part)
        Objects fb_additional_objects;      //additional objects to avoid, must be enabled by 'fb_avoid_objects'
    };

    struct Label
    {
        /**
         * Revert current to initial position.
         */
        void revert()
        {
            x = x_last;
            y = y_last;
        }

        bool isInit() const 
        {
            return (w > 0 && h > 0 && !id.empty());
        }

        bool isShown() const 
        {
            return (active && !outside && !hidden);
        }

        /**
         * Compute (scaled) bounding box.
         */
        QRectF boundingBox(double tx = 1.0, double ty = 1.0) const
        {
            if (w < 0 || h < 0)
                return QRectF();

            QRectF bbox(x, y, w, h);
            if (tx != 1 || ty != 1)
            {
                double wnew = w * tx;
                double hnew = h * ty;
                double dx = wnew - w;
                double dy = hnew - h;
                dx /= 2;
                dy /= 2;
                bbox = QRectF(x - dx, y - dy, wnew, hnew);
            }

            return bbox;
        }

        QPointF midPoint() const
        {
            return QPointF(x + w / 2, y + h / 2);
        }

        std::string id;

        double      x_anchor = 0; //x pos of the position the label is attached to
        double      y_anchor = 0; //y pos of the position the label is attached to
        double      x        = 0; //current x pos of the label 
        double      y        = 0; //current y pos of the label 
        double      x_last   = 0; //initial x pos of the label 
        double      y_last   = 0; //initial y pos of the label
        double      w        = 0; //label width
        double      h        = 0; //Label height

        bool        active  = true;  //label is active (=will be included in the optimization)
        bool        outside = false; //label out of bounds TODO
        bool        hidden  = false; //label hidden by the generator during placement TODO 
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

private:
    void revertPlacements();
    bool placeLabelsForceBased();

    std::vector<Label>            labels_;
    std::map<std::string, size_t> label_map_;

    Settings                      settings_;
};
