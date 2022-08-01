
#pragma once

#include "labelplacement_defs.h"

#include <vector>

//general helpers for automated label placement
namespace label_placement
{
    /**
     * Collect the (scaled) bounding boxes for all given labels.
     */
    std::vector<QRectF> collectBoundingBoxes(const std::vector<Label>& labels, 
                                             double tx, 
                                             double ty)
    {
        if (labels.empty())
            return {};

        size_t n = labels.size();

        std::vector<QRectF> bboxes(n);
        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];
            bboxes[ i ] = l.boundingBox(tx, ty);
        }

        return bboxes;
    }
}
