
#pragma once

#include "labelplacement_defs.h"
#include "labelplacement_helpers.h"

#include <vector>
#include <iostream>

#include <Eigen/Core>

namespace label_placement
{

//helpers for the force-based approach.
//see: https://github.com/Phlya/adjustText
namespace force
{
    /**
     */
    int sign(double v)
    {
        if (v < 0)
            return -1;
        if (v > 0)
            return 1;
        return 0;
    }

    /**
     */
    QPointF repelFromPosition(const QRectF& bbox, 
                              double x, 
                              double y, 
                              ForceType force)
    {
        if (!bbox.contains(x, y))
            return QPointF(0, 0);

        if (force == ForceType::Simple)
        {
            QPointF c = bbox.center();

            int dir_x = sign(c.x() - x);
            int dir_y = sign(c.y() - y);

            double dx = dir_x == 0 ? 0 : (dir_x == -1 ? x - bbox.right()  : x - bbox.left());
            double dy = dir_y == 0 ? 0 : (dir_y == -1 ? y - bbox.bottom() : y - bbox.top() );

            return {dx, dy};
        }
        else if (force == ForceType::Exact)
        {
            QPointF c = bbox.center();

            double dx = c.x() - x;
            double dy = c.y() - y;
            double d_sqr = std::max(dx * dx + dy * dy, 1e-06);
            double d = std::sqrt(d_sqr);
            dx /= d;
            dy /= d;

            const double f = std::max(bbox.width(), bbox.height());

            return {dx * f, dy * f};
        }

        //unknown mode
        return {0, 0};
    }

    /**
     */
    QPointF repelFromBox(const QRectF& bbox, 
                         const QRectF& bbox2, 
                         ForceType force)
    {
        if (!bbox.intersects(bbox2))
        {
            return {0, 0};
        }

        if (force == ForceType::Simple)
        {
            auto r_isec = bbox & bbox2;
            if (r_isec.isEmpty())
                return {0, 0};

            const double dx = r_isec.width();
            const double dy = r_isec.height();
            const int    sx = sign(bbox.left() - bbox2.left());
            const int    sy = sign(bbox.top()  - bbox2.top() );

            return {dx * sx, dy * sy};
        }
        else if (force == ForceType::Exact)
        {
            QPointF c0 = bbox.center();
            QPointF c1 = bbox2.center();

            double dx = c0.x() - c1.x();
            double dy = c0.y() - c1.y();
            double d_sqr = std::max(dx * dx + dy * dy, 1e-06);
            double d = std::sqrt(d_sqr);
            dx /= d;
            dy /= d;

            const double f = std::max(bbox.width(), bbox.height());

            return {dx * f, dy * f};
        }
        
        //unknown mode
        return {0, 0};
    }

    /**
     */
    void repelFromLabels(std::vector<Eigen::Vector2d>& movements,
                         Eigen::Vector2d& total,
                         const std::vector<Label>& labels, 
                         double tx, 
                         double ty,
                         ForceType force)
    {
        /*"""
        Repel texts from each other while expanding their bounding boxes by expand
        (x, y), e.g. (1.2, 1.2) would multiply width and height by 1.2.
        Requires a renderer to get the actual sizes of the text, and to that end
        either one needs to be directly provided, or the axes have to be specified,
        and the renderer is then got from the axes object.
        """
        ax = ax or plt.gca()
        r = renderer or get_renderer(ax.get_figure())
        bboxes = get_bboxes(texts, r, expand, ax=ax)
        xmins = [bbox.xmin for bbox in bboxes]
        xmaxs = [bbox.xmax for bbox in bboxes]
        ymaxs = [bbox.ymax for bbox in bboxes]
        ymins = [bbox.ymin for bbox in bboxes]

        overlaps_x = np.zeros((len(bboxes), len(bboxes)))
        overlaps_y = np.zeros_like(overlaps_x)
        overlap_directions_x = np.zeros_like(overlaps_x)
        overlap_directions_y = np.zeros_like(overlaps_y)
        for i, bbox1 in enumerate(bboxes):
            overlaps = get_points_inside_bbox(
                xmins * 2 + xmaxs * 2, (ymins + ymaxs) * 2, bbox1
            ) % len(bboxes)
            overlaps = np.unique(overlaps)
            for j in overlaps:
                bbox2 = bboxes[j]
                x, y = bbox1.intersection(bbox1, bbox2).size
                overlaps_x[i, j] = x
                overlaps_y[i, j] = y
                direction = np.sign(bbox1.extents - bbox2.extents)[:2]
                overlap_directions_x[i, j] = direction[0]
                overlap_directions_y[i, j] = direction[1]

        move_x = overlaps_x * overlap_directions_x
        move_y = overlaps_y * overlap_directions_y

        delta_x = move_x.sum(axis=1)
        delta_y = move_y.sum(axis=1)

        q = np.sum(overlaps_x), np.sum(overlaps_y)
        if move:
            move_texts(texts, delta_x, delta_y, bboxes, ax=ax)
        return delta_x, delta_y, q*/

        size_t n = labels.size();

        movements.assign(n, Eigen::Vector2d(0, 0));
        total = Eigen::Vector2d(0, 0);

        if (n <= 1)
            return;

        std::vector<QRectF> bboxes = collectBoundingBoxes(labels, tx, ty);

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];
            if (!l.active)
                continue;

            const auto& bbox = bboxes[ i ];

            for (size_t j = 0; j < n; ++j)
            {
                if (i == j)
                    continue;

                const auto& l2 = labels[ j ];
                if (!l2.active)
                    continue;

                const auto& bbox2 = bboxes[ j ];

                auto offset = repelFromBox(bbox, bbox2, force);

                movements[ i ] += Eigen::Vector2d(offset.x(), offset.y());
                total          += Eigen::Vector2d(std::fabs(offset.x()), std::fabs(offset.y()));
            }
        }
    }

    /**
     */
    void repelFromObjects(std::vector<Eigen::Vector2d>& movements,
                          Eigen::Vector2d& total,
                          const std::vector<QRectF>& objects,
                          const std::vector<Label>& labels, 
                          double tx, 
                          double ty,
                          ForceType force)
    {
        /*"""
        Repel texts from other objects' bboxes while expanding their (texts')
        bounding boxes by expand (x, y), e.g. (1.2, 1.2) would multiply width and
        height by 1.2.
        Requires a renderer to get the actual sizes of the text, and to that end
        either one needs to be directly provided, or the axes have to be specified,
        and the renderer is then got from the axes object.
        """
        ax = ax or plt.gca()
        r = renderer or get_renderer(ax.get_figure())

        bboxes = get_bboxes(texts, r, expand, ax=ax)

        overlaps_x = np.zeros((len(bboxes), len(add_bboxes)))
        overlaps_y = np.zeros_like(overlaps_x)
        overlap_directions_x = np.zeros_like(overlaps_x)
        overlap_directions_y = np.zeros_like(overlaps_y)

        for i, bbox1 in enumerate(bboxes):
            for j, bbox2 in enumerate(add_bboxes):
                try:
                    x, y = bbox1.intersection(bbox1, bbox2).size
                    direction = np.sign(bbox1.extents - bbox2.extents)[:2]
                    overlaps_x[i, j] = x
                    overlaps_y[i, j] = y
                    overlap_directions_x[i, j] = direction[0]
                    overlap_directions_y[i, j] = direction[1]
                except AttributeError:
                    pass

        move_x = overlaps_x * overlap_directions_x
        move_y = overlaps_y * overlap_directions_y

        delta_x = move_x.sum(axis=1)
        delta_y = move_y.sum(axis=1)

        q = np.sum(overlaps_x), np.sum(overlaps_y)
        if move:
            move_texts(texts, delta_x, delta_y, bboxes, ax=ax)
        return delta_x, delta_y, q*/

        size_t n = labels.size();

        movements.assign(n, Eigen::Vector2d(0, 0));
        total = Eigen::Vector2d(0, 0);

        size_t no = objects.size();
        if (no < 1)
            return;

        std::vector<QRectF> bboxes = collectBoundingBoxes(labels, tx, ty);

        auto coutBBox = [ & ] (const QRectF& bbox) 
        {
            std::cout << "(" << bbox.x() << "," << bbox.y() << "," << bbox.width() << "x" << bbox.height() << ")"; 
        };

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];
            if (!l.active)
                continue;

            const auto& bbox = bboxes[ i ];

            for (size_t j = 0; j < no; ++j)
            {
                const auto& bbox2 = objects[ j ];
                if (bbox2.isEmpty())
                    continue;

                // std::cout << "      ";
                // coutBBox(bbox);
                // std::cout << " ";
                // coutBBox(bbox2);
                // std::cout << " => " << bbox.intersects(bbox2) << std::endl;

                auto offset = repelFromBox(bbox, bbox2, force);

                movements[ i ] += Eigen::Vector2d(offset.x(), offset.y());
                total          += Eigen::Vector2d(std::fabs(offset.x()), std::fabs(offset.y()));
            }
        }
    }

    /**
     */
    void repelFromPoints(std::vector<Eigen::Vector2d>& movements,
                         Eigen::Vector2d& total,
                         const std::vector<QPointF>& points,
                         const std::vector<Label>& labels, 
                         double tx, 
                         double ty,
                         ForceType force)
    {
        /*""
        Repel texts from all points specified by x and y while expanding their
        (texts'!) bounding boxes by expandby  (x, y), e.g. (1.2, 1.2)
        would multiply both width and height by 1.2.
        Requires a renderer to get the actual sizes of the text, and to that end
        either one needs to be directly provided, or the axes have to be specified,
        and the renderer is then got from the axes object.
        """
        assert len(x) == len(y)
        ax = ax or plt.gca()
        r = renderer or get_renderer(ax.get_figure())
        bboxes = get_bboxes(texts, r, expand, ax=ax)

        # move_x[i,j] is the x displacement of the i'th text caused by the j'th point
        move_x = np.zeros((len(bboxes), len(x)))
        move_y = np.zeros((len(bboxes), len(x)))
        for i, bbox in enumerate(bboxes):
            xy_in = get_points_inside_bbox(x, y, bbox)
            for j in xy_in:
                xp, yp = x[j], y[j]
                dx, dy = overlap_bbox_and_point(bbox, xp, yp)

                move_x[i, j] = dx
                move_y[i, j] = dy

        delta_x = move_x.sum(axis=1)
        delta_y = move_y.sum(axis=1)
        q = np.sum(np.abs(move_x)), np.sum(np.abs(move_y))
        if move:
            move_texts(texts, delta_x, delta_y, bboxes, ax=ax)
        return delta_x, delta_y, q*/

        size_t n = labels.size();

        movements.assign(n, Eigen::Vector2d(0, 0));
        total = Eigen::Vector2d(0, 0);

        size_t np = points.size();
        if (np < 1)
            return;

        std::vector<QRectF> bboxes = collectBoundingBoxes(labels, tx, ty);

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];
            if (!l.active)
                continue;

            const auto& bbox = bboxes[ i ];

            for (size_t j = 0; j < np; ++j)
            {
                const auto& point = points[ j ];
                
                auto offset = repelFromPosition(bbox, point.x(), point.y(), force);

                movements[ i ] += Eigen::Vector2d(offset.x(), offset.y());
                total          += Eigen::Vector2d(std::fabs(offset.x()), std::fabs(offset.y()));
            }
        }
    }

    /**
     */
    void repelFromROI(std::vector<Eigen::Vector2d>& movements,
                      Eigen::Vector2d& total,
                      const QRectF& roi,
                      const std::vector<Label>& labels, 
                      double tx, 
                      double ty,
                      ForceType force)
    {
        size_t n = labels.size();

        movements.assign(n, Eigen::Vector2d(0, 0));
        total = Eigen::Vector2d(0, 0);

        if (roi.isEmpty())
            return;

        std::vector<QRectF> bboxes = collectBoundingBoxes(labels, tx, ty);

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];
            if (!l.active)
                continue;

            const auto& bbox = bboxes[ i ];

            double dx = 0;
            double dy = 0;
            if (bbox.left() < roi.left())
                dx = roi.left() - bbox.left();
            if (bbox.right() > roi.right())
                dx = roi.right() - bbox.right();
            if (bbox.top() < roi.top())
                dy = roi.top() - bbox.top();
            if (bbox.bottom() > roi.bottom())
                dy = roi.bottom() - bbox.bottom();

            movements[ i ] += Eigen::Vector2d(dx, dy);
            total          += Eigen::Vector2d(std::fabs(dx), std::fabs(dy));
        }
    }

}
}