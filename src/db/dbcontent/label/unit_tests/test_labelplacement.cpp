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
