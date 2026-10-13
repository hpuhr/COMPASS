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

#include "catch.hpp"

#include "labelplacement.h"
#include "labelplacement_helpers.h"
#include "labelplacement_spring.h"
#include "labelplacement_constraint.h"

#include <numeric>
#include <random>

#include <QLineF>

TEST_CASE("LabelPlacementEngine coincident labels", "[labelplacement]")
{
    LabelPlacementEngine engine;

    SECTION("simple force method")
    {
        engine.settings().method = label_placement::Method::ForceBasedSimple;
    }
    SECTION("extended force method")
    {
        engine.settings().method = label_placement::Method::ForceBasedExtended;
    }
    SECTION("relaxed force method")
    {
        engine.settings().method = label_placement::Method::ForceBasedRelaxed;
    }

    //two labels at the same anchor and the same position (e.g. the same target from two sources)
    double x = 110.0;
    double y = 80.0;

    engine.addLabel("a", 100.0, 100.0, 40.0, 10.0, &x, &y);
    engine.addLabel("b", 100.0, 100.0, 40.0, 10.0, &x, &y);

    REQUIRE(engine.placeLabels());

    const auto* a = engine.getLabel("a");
    const auto* b = engine.getLabel("b");
    REQUIRE(a);
    REQUIRE(b);

    REQUIRE(!a->boundingBox().intersects(b->boundingBox()));

    //separated vertically, in label order
    REQUIRE(a->x == Approx(x));
    REQUIRE(b->x == Approx(x));
    REQUIRE(a->y < b->y);
}

TEST_CASE("LabelPlacementEngine fixed objects", "[labelplacement]")
{
    const auto method = GENERATE(label_placement::Method::ForceBasedSimple,
                                 label_placement::Method::ForceBasedExtended,
                                 label_placement::Method::ForceBasedExact,
                                 label_placement::Method::ForceBasedRelaxed,
                                 label_placement::Method::ConstraintBased);
    CAPTURE((int)method);

    LabelPlacementEngine engine;

    auto& settings = engine.settings();
    settings.method = method;
    settings.roi    = QRectF(0, 0, 400, 300);

    //fixed object (e.g. a manually placed label) covering the home position of label a
    const QRectF object(100.0, 80.0, 60.0, 20.0);

    settings.avoid_objects      = true;
    settings.additional_objects = { object };

    double ax = 95.0, ay = 85.0;
    double bx = 300.0, by = 200.0;

    engine.addLabel("a",  90.0, 100.0, 40.0, 10.0, &ax, &ay, &ax, &ay);
    engine.addLabel("b", 290.0, 215.0, 40.0, 10.0, &bx, &by, &bx, &by);

    REQUIRE(engine.placeLabels());

    const auto* a = engine.getLabel("a");
    const auto* b = engine.getLabel("b");
    REQUIRE(a);
    REQUIRE(b);

    //the label avoids the object, the object does not move anything else
    REQUIRE(!a->boundingBox().intersects(object));
    REQUIRE(!b->boundingBox().intersects(object));
    REQUIRE(QLineF(QPointF(b->x, b->y), QPointF(bx, by)).length() <= 1.0);
}

TEST_CASE("LabelPlacementEngine regularisation", "[labelplacement]")
{
    LabelPlacementEngine engine;

    auto& settings = engine.settings();
    settings.method          = label_placement::Method::ForceBasedExtended;
    settings.fb_weight_ref   = 0.05;
    settings.fb_weight_init  = 0.2;
    settings.fb_max_ref_dist = 60.0;

    SECTION("a label without conflict moves towards its reference position, repeated runs reach it")
    {
        double x_ref = 100.0;
        double y_ref = 100.0;
        double x     = 130.0;
        double y     = 100.0;

        engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y, &x_ref, &y_ref);
        REQUIRE(engine.placeLabels());

        //equilibrium of both pulls: (w_ref * ref + w_init * init) / (w_ref + w_init)
        REQUIRE(engine.getLabel("a")->x == Approx((0.05 * 100.0 + 0.2 * 130.0) / 0.25).margin(0.5));
        REQUIRE(engine.getLabel("a")->y == Approx(100.0).margin(0.5));

        //next runs start from the last placement
        for (int run = 0; run < 40; ++run)
        {
            x = engine.getLabel("a")->x;
            y = engine.getLabel("a")->y;

            engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y, &x_ref, &y_ref);
            REQUIRE(engine.placeLabels());
        }

        REQUIRE(engine.getLabel("a")->x == Approx(100.0).margin(1.0));
    }

    SECTION("labels never leave the max distance from their reference position")
    {
        settings.fb_max_ref_dist = 3.0;

        //coincident labels, separating them would need more than the max distance
        double x = 110.0;
        double y = 80.0;

        engine.addLabel("a", 100.0, 100.0, 40.0, 10.0, &x, &y, &x, &y);
        engine.addLabel("b", 100.0, 100.0, 40.0, 10.0, &x, &y, &x, &y);
        REQUIRE(engine.placeLabels());

        for (const char* id : { "a", "b" })
        {
            const auto* l = engine.getLabel(id);
            REQUIRE(QLineF(l->x, l->y, x, y).length() <= 3.0 + 1e-6);
        }
    }

    SECTION("coincident labels are still separated")
    {
        SECTION("simple force method")
        {
            settings.method = label_placement::Method::ForceBasedSimple;
        }
        SECTION("extended force method")
        {
            settings.method = label_placement::Method::ForceBasedExtended;
        }

        double x = 110.0;
        double y = 80.0;

        engine.addLabel("a", 100.0, 100.0, 40.0, 10.0, &x, &y, &x, &y);
        engine.addLabel("b", 100.0, 100.0, 40.0, 10.0, &x, &y, &x, &y);
        REQUIRE(engine.placeLabels());

        const auto* a = engine.getLabel("a");
        const auto* b = engine.getLabel("b");

        REQUIRE(!a->boundingBox().intersects(b->boundingBox()));
        REQUIRE(QLineF(a->x, a->y, x, y).length() <= 60.0 + 1e-6);
        REQUIRE(QLineF(b->x, b->y, x, y).length() <= 60.0 + 1e-6);
    }
}

TEST_CASE("LabelPlacementEngine relaxed force method", "[labelplacement]")
{
    LabelPlacementEngine engine;

    auto& settings = engine.settings();
    settings.method         = label_placement::Method::ForceBasedRelaxed;
    settings.fb_start_relax = 0.2;

    double x_ref = 100.0;
    double y_ref = 100.0;
    double x     = 130.0;
    double y     = 100.0;

    SECTION("a label without conflict starts the given fraction closer to its reference position")
    {
        engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y, &x_ref, &y_ref);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(124.0));
        REQUIRE(engine.getLabel("a")->y == Approx(100.0));

        //next runs start from the last placement => reference position is approached
        for (int run = 0; run < 40; ++run)
        {
            x = engine.getLabel("a")->x;
            y = engine.getLabel("a")->y;

            engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y, &x_ref, &y_ref);
            REQUIRE(engine.placeLabels());
        }

        REQUIRE(engine.getLabel("a")->x == Approx(100.0).margin(0.01));
    }

    SECTION("no relaxation => the label stays where it is")
    {
        settings.fb_start_relax = 0.0;

        engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y, &x_ref, &y_ref);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(130.0));
    }

    SECTION("a label without reference position stays where it is")
    {
        engine.addLabel("a", 90.0, 120.0, 40.0, 10.0, &x, &y);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(130.0));
    }
}

TEST_CASE("LabelPlacementEngine constraint based method", "[labelplacement]")
{
    LabelPlacementEngine engine;
    engine.settings().method = label_placement::Method::ConstraintBased;

    //label with home (reference) and previous placement (initial position), anchor below the home position
    auto add = [ & ] (const std::string& id, double hx, double hy, double px, double py, double w = 40.0, double h = 10.0)
    {
        engine.addLabel(id, hx, hy + 30.0, w, h, &px, &py, &hx, &hy);
    };

    auto rect = [ & ] (const char* id)
    {
        const auto* l = engine.getLabel(id);
        return QRectF(l->x, l->y, l->w, l->h);
    };

    SECTION("a label without conflict is placed at home, also if placed elsewhere before")
    {
        add("a", 100.0, 100.0, 130.0, 80.0);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(100.0));
        REQUIRE(engine.getLabel("a")->y == Approx(100.0));
    }

    SECTION("with a weight on the previous placement a label without conflict returns home gradually")
    {
        engine.settings().cs_weight_prev = 1.0;

        add("a", 100.0, 100.0, 130.0, 80.0);
        REQUIRE(engine.placeLabels());

        //equal weights => halfway
        REQUIRE(engine.getLabel("a")->x == Approx(115.0));
        REQUIRE(engine.getLabel("a")->y == Approx(90.0));
    }

    SECTION("the separation direction can be weighted by the label size")
    {
        //wide labels, mostly side by side: vertical needs 14 px, horizontal 22 px (previously coincident => no
        //previous side)
        add("a", 100.0, 100.0, 100.0, 100.0, 60.0, 16.0);
        add("b", 140.0, 104.0, 100.0, 100.0, 60.0, 16.0);

        SECTION("absolute => vertical")
        {
            engine.settings().cs_balance = 0.0;
            REQUIRE(engine.placeLabels());

            REQUIRE(engine.getLabel("a")->x == Approx(100.0));
            REQUIRE(engine.getLabel("b")->x == Approx(140.0));
        }
        SECTION("relative to the size => horizontal (22 / 60 < 14 / 16)")
        {
            engine.settings().cs_balance = 1.0;
            REQUIRE(engine.placeLabels());

            REQUIRE(engine.getLabel("a")->y == Approx(100.0));
            REQUIRE(engine.getLabel("b")->y == Approx(104.0));
        }

        REQUIRE(!rect("a").intersects(rect("b")));
    }

    SECTION("candidate positions: a conflicting label moves to the other side of its anchor")
    {
        engine.settings().cs_candidates   = 4;
        engine.settings().cs_cand_penalty = 0.0;
        engine.settings().cs_cand_switch  = 0.5;

        //anchors 2 px apart vertically (same x), homes right up of them (anchor line 20 px) => overlapping by 8 px,
        //more than the cost of changing sides (0.5 * label area)
        double ahx = 114.0, ahy = 106.0;
        double bhx = 114.0, bhy = 108.0;
        engine.addLabel("a", 100.0, 130.0, 40.0, 10.0, &ahx, &ahy, &ahx, &ahy);
        engine.addLabel("b", 100.0, 132.0, 40.0, 10.0, &bhx, &bhy, &bhx, &bhy);
        REQUIRE(engine.placeLabels());

        REQUIRE(!rect("a").intersects(rect("b")));

        //a is mirrored to the left of its anchor (2 * 100 - 114 - 40 = 46), b stays home, no displacement needed
        REQUIRE(engine.getLabel("a")->x == Approx(46.0));
        REQUIRE(engine.getLabel("a")->y == Approx(106.0));
        REQUIRE(engine.getLabel("b")->x == Approx(114.0));
        REQUIRE(engine.getLabel("b")->y == Approx(108.0));
    }

    SECTION("candidate positions: the previous side is kept")
    {
        engine.settings().cs_candidates = 8;

        //no conflict, home right up of the anchor (anchor line 20 px), previously left up
        double hx = 114.0, hy = 106.0;
        double px = 46.0,  py = 106.0;
        engine.addLabel("a", 100.0, 130.0, 40.0, 10.0, &px, &py, &hx, &hy);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(46.0));
        REQUIRE(engine.getLabel("a")->y == Approx(106.0));
    }

    SECTION("candidate positions: a label without history chooses its side freely")
    {
        engine.settings().cs_candidates   = 8;
        engine.settings().cs_cand_penalty = 0.0;
        engine.settings().cs_cand_switch  = 0.5;

        //anchors 6 px apart vertically => homes overlap by 4 px, less than the cost of changing sides
        double ahx = 114.0, ahy = 106.0;
        double bhx = 114.0, bhy = 112.0;
        engine.addLabel("b", 100.0, 136.0, 40.0, 10.0, &bhx, &bhy, &bhx, &bhy);

        SECTION("with history => both stay on their side, separated by displacement")
        {
            engine.addLabel("a", 100.0, 130.0, 40.0, 10.0, &ahx, &ahy, &ahx, &ahy);
            REQUIRE(engine.placeLabels());

            REQUIRE(engine.getLabel("a")->x == Approx(114.0));
            REQUIRE(engine.getLabel("b")->x == Approx(114.0));
        }
        SECTION("without history => a moves to a free side, no displacement needed")
        {
            engine.addLabel("a", 100.0, 130.0, 40.0, 10.0, &ahx, &ahy, &ahx, &ahy, false);
            REQUIRE(engine.placeLabels());

            const auto* a = engine.getLabel("a");
            REQUIRE(a->x != Approx(114.0));
            REQUIRE(engine.getLabel("b")->x == Approx(114.0));
            REQUIRE(engine.getLabel("b")->y == Approx(112.0));
        }

        REQUIRE(!rect("a").intersects(rect("b")));
    }

    SECTION("without candidate positions labels stay on their side")
    {
        add("a", 114.0, 100.0, 114.0, 100.0);
        add("b", 114.0, 106.0, 114.0, 106.0);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x + 20.0 > engine.getLabel("a")->x_anchor);
        REQUIRE(engine.getLabel("b")->x + 20.0 > engine.getLabel("b")->x_anchor);
    }

    SECTION("gaps per axis")
    {
        engine.settings().cs_gap_y = 4.0;

        add("a", 100.0, 100.0, 100.0, 100.0);
        add("b", 100.0, 100.0, 100.0, 100.0);
        REQUIRE(engine.placeLabels());

        //height + vertical gap = 14, split evenly
        REQUIRE(engine.getLabel("a")->y == Approx(93.0).margin(0.01));
        REQUIRE(engine.getLabel("b")->y == Approx(107.0).margin(0.01));
    }

    SECTION("coincident labels are separated with minimal displacement")
    {
        add("a", 100.0, 100.0, 100.0, 100.0);
        add("b", 100.0, 100.0, 100.0, 100.0);
        REQUIRE(engine.placeLabels());

        //vertical is cheaper (height + gap = 12), split evenly, first label above
        REQUIRE(engine.getLabel("a")->x == Approx(100.0));
        REQUIRE(engine.getLabel("b")->x == Approx(100.0));
        REQUIRE(engine.getLabel("a")->y == Approx(94.0).margin(0.01));
        REQUIRE(engine.getLabel("b")->y == Approx(106.0).margin(0.01));
        REQUIRE(!rect("a").intersects(rect("b")));
    }

    SECTION("the previous side of a pair is kept")
    {
        //b was above a before
        add("a", 100.0, 100.0, 100.0, 100.0);
        add("b", 100.0, 100.0, 100.0, 88.0);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("b")->y == Approx(94.0).margin(0.01));
        REQUIRE(engine.getLabel("a")->y == Approx(106.0).margin(0.01));
    }

    SECTION("the previous axis is kept if not much more expensive")
    {
        //side by side before, narrow labels: horizontal needs 22, vertical 12 => kept (22 <= 2 * 12 + 10)
        add("a", 100.0, 100.0, 100.0, 100.0, 20.0, 10.0);
        add("b", 100.0, 100.0, 125.0, 100.0, 20.0, 10.0);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(89.0).margin(0.01));
        REQUIRE(engine.getLabel("b")->x == Approx(111.0).margin(0.01));
        REQUIRE(engine.getLabel("a")->y == Approx(100.0));
        REQUIRE(engine.getLabel("b")->y == Approx(100.0));
    }

    SECTION("a much more expensive previous axis is not kept")
    {
        //side by side before, wide labels: horizontal needs 42, vertical 12 => switched (42 > 2 * 12 + 10)
        add("a", 100.0, 100.0, 100.0, 100.0);
        add("b", 100.0, 100.0, 145.0, 100.0);
        REQUIRE(engine.placeLabels());

        REQUIRE(engine.getLabel("a")->x == Approx(100.0));
        REQUIRE(engine.getLabel("b")->x == Approx(100.0));
        REQUIRE(!rect("a").intersects(rect("b")));
    }

    SECTION("a label passing another one moves smoothly, without overlap")
    {
        //b's home moves past a in steps of 1 px, each run starts from the last placement
        double ax = 100.0, ay = 100.0, bx = 60.0, by = 100.0;

        int large_changes = 0;

        for (int step = 0; step <= 80; ++step)
        {
            const double bhx = 60.0 + step;

            engine.clearLabels();
            add("a", 100.0, 100.0, ax, ay);
            add("b", bhx, 100.0, bx, by);
            REQUIRE(engine.placeLabels());

            REQUIRE(!rect("a").intersects(rect("b")));

            const auto* a = engine.getLabel("a");
            const auto* b = engine.getLabel("b");

            //change relative to the home positions (b's home moved by 1)
            const double change = std::max(std::hypot(a->x - ax, a->y - ay),
                                           std::hypot((b->x - bhx) - (bx - (bhx - 1.0)), b->y - by));
            if (step > 0 && change > 2.0)
                ++large_changes;

            ax = a->x; ay = a->y; bx = b->x; by = b->y;
        }

        //continuous except where the pair has to change its axis (needed separation gets too large)
        REQUIRE(large_changes <= 2);
    }

    SECTION("anchors are avoided")
    {
        engine.settings().cs_anchor_radius = 10.0;

        double hx = 100.0, hy = 100.0;

        //b's anchor lies within a's home position
        engine.addLabel("a", 100.0, 160.0, 40.0, 10.0, &hx, &hy, &hx, &hy);

        double bhx = 300.0, bhy = 300.0;
        engine.addLabel("b", 120.0, 105.0, 40.0, 10.0, &bhx, &bhy, &bhx, &bhy);

        REQUIRE(engine.placeLabels());

        REQUIRE(!rect("a").intersects(QRectF(110.0, 95.0, 20.0, 20.0)));
    }

    SECTION("a dense stack is decluttered")
    {
        for (int i = 0; i < 20; ++i)
            add("l" + std::to_string(i), 500.0, 500.0, 500.0, 500.0);

        REQUIRE(engine.placeLabels());

        for (int i = 0; i < 20; ++i)
            for (int j = i + 1; j < 20; ++j)
                REQUIRE(!rect(("l" + std::to_string(i)).c_str()).intersects(rect(("l" + std::to_string(j)).c_str())));
    }

    SECTION("labels are kept within the region of interest")
    {
        engine.settings().cs_keep_in_roi = true;
        engine.settings().roi            = QRectF(0.0, 0.0, 400.0, 300.0);

        add("a", 380.0, -5.0, 380.0, -5.0);
        REQUIRE(engine.placeLabels());

        //soft bounds: violation / overshoot = 1 / cs_soft_weight (100) => (380 + 100 * 360) / 101
        REQUIRE(engine.getLabel("a")->x == Approx((380.0 + 100.0 * 360.0) / 101.0).margin(0.01));
        REQUIRE(engine.getLabel("a")->y == Approx((-5.0 + 100.0 * 0.0) / 101.0).margin(0.01));
    }

    SECTION("same input, same placement")
    {
        auto run = [ & ] ()
        {
            engine.clearLabels();
            for (int i = 0; i < 10; ++i)
                add("l" + std::to_string(i), 100.0 + i * 7.0, 100.0 + (i % 3) * 4.0, 100.0 + i * 7.0, 100.0);

            REQUIRE(engine.placeLabels());

            std::vector<QPointF> pos;
            for (int i = 0; i < 10; ++i)
                pos.emplace_back(engine.getLabel(i).x, engine.getLabel(i).y);
            return pos;
        };

        REQUIRE(run() == run());
    }
}

namespace
{
    /**
     * Reference solver for the axis problem (Hildreth's method, plain projected dual coordinate ascent).
     */
    std::vector<double> referenceSolve(const std::vector<double>& desired,
                                       const std::vector<double>& weights,
                                       const std::vector<label_placement::constraint::Separation>& constraints)
    {
        std::vector<double> v = desired;
        std::vector<double> lambda(constraints.size(), 0.0);

        for (int sweep = 0; sweep < 1000000; ++sweep)
        {
            double max_change = 0.0;

            for (size_t k = 0; k < constraints.size(); ++k)
            {
                const auto& c = constraints[ k ];

                const double norm       = 1.0 / weights[ c.left ] + 1.0 / weights[ c.right ];
                const double lambda_new = std::max(0.0, lambda[ k ] + (c.gap - (v[ c.right ] - v[ c.left ])) / norm);
                const double dl         = lambda_new - lambda[ k ];

                v[ c.right ] += dl / weights[ c.right ];
                v[ c.left  ] -= dl / weights[ c.left  ];

                lambda[ k ] = lambda_new;
                max_change  = std::max(max_change, std::fabs(dl));
            }

            if (max_change < 1e-14)
                break;
        }

        return v;
    }
}

TEST_CASE("Constraint based placement axis solver", "[labelplacement]")
{
    using label_placement::constraint::AxisSolver;
    using label_placement::constraint::Separation;

    std::vector<double> v;

    SECTION("a violated constraint is split by the weights")
    {
        //min (a - 0)^2 + 3 (b - 0)^2 with b - a >= 10 => a = -7.5, b = 2.5
        AxisSolver solver({ 0.0, 0.0 }, { 1.0, 3.0 }, { { 0, 1, 10.0 } });
        REQUIRE(solver.solve(v));

        REQUIRE(v[ 0 ] == Approx(-7.5));
        REQUIRE(v[ 1 ] == Approx(2.5));
    }

    SECTION("a satisfied constraint does not move anything")
    {
        AxisSolver solver({ 0.0, 20.0 }, { 1.0, 1.0 }, { { 0, 1, 10.0 } });
        REQUIRE(solver.solve(v));

        REQUIRE(v[ 0 ] == Approx(0.0));
        REQUIRE(v[ 1 ] == Approx(20.0));
    }

    SECTION("a block merged too eagerly is split again")
    {
        //a and b are merged first, then the heavy c pulls the block far left => b's multiplier is negative, b is
        //split off and stays at its desired position; a and c: min a^2 + 10 (c + 100)^2 with a <= c => -1000 / 11
        AxisSolver solver({ 0.0, 0.0, -100.0 }, { 1.0, 1.0, 10.0 }, { { 0, 1, 10.0 }, { 0, 2, 0.0 } });
        REQUIRE(solver.solve(v));

        REQUIRE(v[ 0 ] == Approx(-1000.0 / 11.0));
        REQUIRE(v[ 1 ] == Approx(0.0).margin(1e-9));
        REQUIRE(v[ 2 ] == Approx(-1000.0 / 11.0));
    }

    SECTION("cyclic constraints are reported")
    {
        AxisSolver solver({ 0.0, 0.0 }, { 1.0, 1.0 }, { { 0, 1, 10.0 }, { 1, 0, 10.0 } });
        REQUIRE(!solver.solve(v));
    }

    SECTION("random problems: same optimum as the reference solver")
    {
        std::mt19937 rng(42);

        std::uniform_real_distribution<double> u_pos(0.0, 100.0);
        std::uniform_real_distribution<double> u_gap(0.0, 20.0);
        std::uniform_int_distribution<int>     u_n  (2, 12);
        std::bernoulli_distribution            heavy(0.2);

        for (int problem = 0; problem < 100; ++problem)
        {
            const int n = u_n(rng);

            std::vector<double> desired(n), weights(n);
            for (int i = 0; i < n; ++i)
            {
                desired[ i ] = u_pos(rng);
                weights[ i ] = heavy(rng) ? 100.0 : 1.0;
            }

            //random order => acyclic constraints
            std::vector<int> order(n);
            std::iota(order.begin(), order.end(), 0);
            std::shuffle(order.begin(), order.end(), rng);

            std::vector<Separation> constraints;
            std::uniform_int_distribution<int> u_idx(0, n - 1);

            for (int k = 0; k < 2 * n; ++k)
            {
                int i = u_idx(rng);
                int j = u_idx(rng);
                if (i == j)
                    continue;
                if (i > j)
                    std::swap(i, j);

                constraints.push_back({ order[ i ], order[ j ], u_gap(rng) });
            }

            CAPTURE(problem);

            AxisSolver solver(desired, weights, constraints);
            REQUIRE(solver.solve(v));

            const auto ref = referenceSolve(desired, weights, constraints);

            for (const auto& c : constraints)
                REQUIRE(v[ c.right ] - v[ c.left ] >= c.gap - 1e-6);

            for (int i = 0; i < n; ++i)
                REQUIRE(v[ i ] == Approx(ref[ i ]).margin(1e-4));
        }
    }
}

TEST_CASE("Constraint based placement of an overcrowded cluster", "[labelplacement]")
{
    //50 labels with homes within a small area (more space needed than available locally)
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> ux(650.0, 750.0), uy(400.0, 460.0);

    std::vector<label_placement::Label> labels(50);
    for (size_t i = 0; i < labels.size(); ++i)
    {
        auto& l = labels[ i ];

        l.id       = std::to_string(i);
        l.w        = 60.0;
        l.h        = 16.0;
        l.x_anchor = ux(rng);
        l.y_anchor = uy(rng);
        l.x        = l.x_anchor + 14.0;
        l.y        = l.y_anchor - 30.0;
        l.x_last   = l.x;
        l.y_last   = l.y;
        l.x_ref    = l.x;
        l.y_ref    = l.y;
    }

    const double weight_prev = GENERATE(0.0, 1.0);
    CAPTURE(weight_prev);

    label_placement::Settings settings;
    settings.method           = label_placement::Method::ConstraintBased;
    settings.cs_weight_prev   = weight_prev;
    settings.roi              = QRectF(0.0, 0.0, 1599.0, 899.0);
    settings.cs_avoid_anchors = true;
    settings.cs_anchor_radius = 10.0;
    settings.cs_keep_in_roi   = true;

    label_placement::constraint::Stats stats;
    REQUIRE(label_placement::constraint::placeLabels(labels, settings, &stats));

    REQUIRE(stats.solver_converged);
    REQUIRE(!stats.rounds_exhausted);
    REQUIRE(stats.overlaps == 0);
}

TEST_CASE("LabelPlacementEngine label lookup after removal", "[labelplacement]")
{
    LabelPlacementEngine engine;

    engine.addLabel("a", 0.0, 0.0, 10.0, 10.0);
    engine.addLabel("b", 1.0, 0.0, 10.0, 10.0);
    engine.addLabel("c", 2.0, 0.0, 10.0, 10.0);

    engine.removeLabel("a");

    REQUIRE(!engine.getLabel("a"));
    REQUIRE(engine.getLabel("b"));
    REQUIRE(engine.getLabel("b")->id == "b");
    REQUIRE(engine.getLabel("c"));
    REQUIRE(engine.getLabel("c")->id == "c");

    engine.removeLabel(size_t(0)); // b

    REQUIRE(!engine.getLabel("b"));
    REQUIRE(engine.getLabel("c"));
    REQUIRE(engine.getLabel("c")->id == "c");
    REQUIRE(engine.getLabel(0).id == "c");
}

TEST_CASE("Label placement helpers", "[labelplacement]")
{
    SECTION("data frame of negative coordinates")
    {
        label_placement::Label a;
        a.x        = -50.0;
        a.y        = -40.0;
        a.x_anchor = -60.0;
        a.y_anchor = -50.0;

        label_placement::Label b;
        b.x        = -20.0;
        b.y        = -10.0;
        b.x_anchor = -30.0;
        b.y_anchor = -20.0;

        QRectF frame = label_placement::dataFrame({ a, b });

        REQUIRE(frame.left()   == Approx(-60.0));
        REQUIRE(frame.top()    == Approx(-50.0));
        REQUIRE(frame.right()  == Approx(-20.0));
        REQUIRE(frame.bottom() == Approx(-10.0));
    }

    SECTION("boxes are put back within the roi at all edges")
    {
        const QRectF roi(0.0, 0.0, 100.0, 100.0);

        QRectF right = label_placement::force_exact::putWithinBounds(QRectF(90.0, 20.0, 20.0, 10.0), roi);
        REQUIRE(right.left()  == Approx(80.0));
        REQUIRE(right.width() == Approx(20.0));
        REQUIRE(right.top()   == Approx(20.0));

        QRectF left = label_placement::force_exact::putWithinBounds(QRectF(-5.0, 20.0, 20.0, 10.0), roi);
        REQUIRE(left.left() == Approx(0.0));

        QRectF bottom = label_placement::force_exact::putWithinBounds(QRectF(20.0, 95.0, 20.0, 10.0), roi);
        REQUIRE(bottom.top() == Approx(90.0));

        QRectF top = label_placement::force_exact::putWithinBounds(QRectF(20.0, -5.0, 20.0, 10.0), roi);
        REQUIRE(top.top() == Approx(0.0));
    }
}
