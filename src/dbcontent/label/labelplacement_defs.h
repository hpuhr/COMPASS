
#pragma once

#include <string>

#include <QRectF>
#include <QPointF>


namespace label_placement
{
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
        double      x        = 0; //current x pos of the label (top left)
        double      y        = 0; //current y pos of the label (top left)
        double      x_last   = 0; //initial x pos of the label (top left)
        double      y_last   = 0; //initial y pos of the label (top left)
        double      w        = 0; //label width
        double      h        = 0; //Label height

        bool        active  = true;  //label is active (=will be included in the optimization)
        bool        outside = false; //label out of bounds TODO
        bool        hidden  = false; //label hidden by the generator during placement TODO 
    };

    enum class Method
    {
        ForceBased = 0, //simple force based approach
        SpringBased
    };

    enum class ForceType
    {
        Simple = 0, //just get me out of nah, not physically correct but cheap
        Exact       //repel in a physically accurate way
    };
}
