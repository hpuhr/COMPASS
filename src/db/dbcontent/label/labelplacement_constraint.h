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

#include "labelplacement_defs.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include <QRectF>
#include <QLineF>

namespace label_placement
{

//minimal displacement overlap removal with stable separation constraints (VPSC, solved exactly per axis):
//labels are placed as close as possible to their reference (home) positions subject to non-overlap
//constraints between labels, with anchors and with the region of interest. Each overlapping pair gets one
//separation constraint (left/right/above/below); its direction is taken over from the previous placement
//(the labels' initial positions) unless that is much more expensive than the cheapest direction. Optionally
//the distance from the previous placement is weighted in as well (cs_weight_prev).
//=> decluttered (hard constraints), local (minimal displacement, the previous placement only as a weighted
//term, no drift), smooth (the solution is continuous in the inputs as long as the constraint set does not
//change) and regular (pairs keep their sides between runs).
namespace constraint
{
    enum Dir
    {
        None = -1,
        Left,  //first left of second
        Right, //second left of first
        Up,    //first above second
        Down   //second above first
    };

    /**
     * Separation constraint on one axis: v[ right ] - v[ left ] >= gap.
     */
    struct Separation
    {
        int    left  = -1;
        int    right = -1;
        double gap   = 0.0;
    };

    /**
     * Exact solver of one axis (VPSC, Dwyer, Marriott, Stuckey: "Fast node overlap removal", 2006): minimizes
     * sum weight * (v - desired)^2 subject to separation constraints without cycles.
     * - variables are merged into blocks connected by active (tight) constraints, a block sits at the weighted
     *   mean of its variables' desired positions (minus their offsets in the block)
     * - satisfy(): variables in topological order, each block is merged with its most violated incoming
     *   constraint until none is violated => feasible
     * - refine(): a block is split at an active constraint with a negative Lagrange multiplier (the two parts
     *   want to move apart), the parts are merged again where violated, until no such constraint is left => optimal
     */
    class AxisSolver
    {
    public:
        AxisSolver(const std::vector<double>& desired,
                   const std::vector<double>& weights,
                   const std::vector<Separation>& constraints)
        {
            const size_t n = desired.size();

            vars_.resize(n);
            for (size_t i = 0; i < n; ++i)
            {
                vars_[ i ].desired = desired[ i ];
                vars_[ i ].weight  = weights[ i ];
            }

            cons_.reserve(constraints.size());
            for (const auto& c : constraints)
            {
                vars_[ c.left  ].out.push_back((int)cons_.size());
                vars_[ c.right ].in.push_back ((int)cons_.size());

                Con con;
                con.left  = c.left;
                con.right = c.right;
                con.gap   = c.gap;

                cons_.push_back(con);
            }

            blocks_.reserve(2 * n);
            for (size_t i = 0; i < n; ++i)
                addToBlock(newBlock(), (int)i);

            mark_.assign(n, 0);
        }

        /**
         * Returns false if the solution is not optimal and feasible (cyclic constraints, split limit).
         */
        bool solve(std::vector<double>& v)
        {
            bool ok = satisfy() && refine();

            v.resize(vars_.size());
            for (size_t i = 0; i < vars_.size(); ++i)
                v[ i ] = position((int)i);

            for (size_t c = 0; c < cons_.size(); ++c)
                if (violation((int)c) > FeasibilityTolerance)
                    ok = false;

            return ok;
        }

        static constexpr double FeasibilityTolerance = 1e-6;  // max violation of a satisfied constraint
        static constexpr double ViolationTolerance   = 1e-9;  // violations below are not merged
        static constexpr double LagrangeTolerance    = -1e-6; // multipliers below split a block

    private:
        struct Var
        {
            double           desired = 0.0;
            double           weight  = 1.0;
            double           offset  = 0.0; // position relative to the block position
            int              block   = -1;
            std::vector<int> in;            // constraints with this variable on the right
            std::vector<int> out;           // constraints with this variable on the left
        };

        struct Con
        {
            int    left   = -1;
            int    right  = -1;
            double gap    = 0.0;
            bool   active = false; // tight, connects two variables of a block
            double lm     = 0.0;   // Lagrange multiplier (active constraints only)
        };

        struct Block
        {
            std::vector<int> vars;
            double           wposn  = 0.0; // sum weight * (desired - offset)
            double           weight = 0.0;
            double           posn   = 0.0;
            bool             alive  = true;
        };

        double position(int v) const { return blocks_[ vars_[ v ].block ].posn + vars_[ v ].offset; }
        double violation(int c) const { return position(cons_[ c ].left) + cons_[ c ].gap - position(cons_[ c ].right); }
        double dfdv(int v) const { return 2.0 * vars_[ v ].weight * (position(v) - vars_[ v ].desired); }

        int newBlock()
        {
            blocks_.emplace_back();
            return (int)blocks_.size() - 1;
        }

        void addToBlock(int b, int v)
        {
            auto& block = blocks_[ b ];
            auto& var   = vars_[ v ];

            var.block = b;
            block.vars.push_back(v);

            block.weight += var.weight;
            block.wposn  += var.weight * (var.desired - var.offset);
            block.posn    = block.wposn / block.weight;
        }

        //block at its optimal position (weighted mean of the desired positions)
        void resetPosition(int b)
        {
            auto& block = blocks_[ b ];

            block.wposn = 0.0;
            for (int v : block.vars)
                block.wposn += vars_[ v ].weight * (vars_[ v ].desired - vars_[ v ].offset);

            block.posn = block.wposn / block.weight;
        }

        //merges the blocks of c (smaller into larger), c becomes tight and active
        int merge(int c)
        {
            auto& con = cons_[ c ];

            int bl = vars_[ con.left  ].block;
            int br = vars_[ con.right ].block;

            //offset of the right block's variables in the left block so that c is tight
            const double dist = vars_[ con.left ].offset + con.gap - vars_[ con.right ].offset;

            int target = bl;
            int source = br;
            double shift = dist;

            if (blocks_[ br ].vars.size() > blocks_[ bl ].vars.size())
            {
                target = br;
                source = bl;
                shift  = -dist;
            }

            const std::vector<int> moved = blocks_[ source ].vars;

            blocks_[ source ].alive = false;
            blocks_[ source ].vars.clear();

            for (int v : moved)
            {
                vars_[ v ].offset += shift;
                addToBlock(target, v);
            }

            con.active = true;

            return target;
        }

        //most violated constraint into (incoming = true) or out of the block, -1 if none is violated
        int mostViolated(int b, bool incoming) const
        {
            int    best     = -1;
            double best_vio = ViolationTolerance;

            for (int v : blocks_[ b ].vars)
            {
                for (int c : incoming ? vars_[ v ].in : vars_[ v ].out)
                {
                    const int other = incoming ? cons_[ c ].left : cons_[ c ].right;
                    if (vars_[ other ].block == b)
                        continue;

                    const double vio = violation(c);
                    if (vio > best_vio)
                    {
                        best     = c;
                        best_vio = vio;
                    }
                }
            }

            return best;
        }

        int mergeLeft(int b)
        {
            int c;
            while ((c = mostViolated(b, true)) >= 0)
                b = merge(c);

            return b;
        }

        int mergeRight(int b)
        {
            int c;
            while ((c = mostViolated(b, false)) >= 0)
                b = merge(c);

            return b;
        }

        bool satisfy()
        {
            //topological order of the variables (Kahn)
            const size_t n = vars_.size();

            std::vector<int> num_in(n, 0);
            for (const auto& c : cons_)
                ++num_in[ c.right ];

            std::vector<int> order;
            order.reserve(n);
            for (size_t i = 0; i < n; ++i)
                if (num_in[ i ] == 0)
                    order.push_back((int)i);

            for (size_t k = 0; k < order.size(); ++k)
                for (int c : vars_[ order[ k ] ].out)
                    if (--num_in[ cons_[ c ].right ] == 0)
                        order.push_back(cons_[ c ].right);

            if (order.size() != n)
                return false; // cycle

            for (int v : order)
                mergeLeft(vars_[ v ].block);

            return true;
        }

        //Lagrange multipliers of the active constraints of the subtree at v (entered via constraint from_c),
        //returns the derivative of the subtree, keeps the constraint with the smallest multiplier
        double computeLM(int v, int from_c, int& min_c)
        {
            double d = dfdv(v);

            for (int c : vars_[ v ].out)
            {
                auto& con = cons_[ c ];
                if (!con.active || c == from_c)
                    continue;

                con.lm = computeLM(con.right, c, min_c);
                d     += con.lm;

                if (min_c < 0 || con.lm < cons_[ min_c ].lm)
                    min_c = c;
            }
            for (int c : vars_[ v ].in)
            {
                auto& con = cons_[ c ];
                if (!con.active || c == from_c)
                    continue;

                con.lm = -computeLM(con.left, c, min_c);
                d     -= con.lm;

                if (min_c < 0 || con.lm < cons_[ min_c ].lm)
                    min_c = c;
            }

            return d;
        }

        //splits block b at its active constraint c into the part left of c and the part right of c
        void split(int b, int c)
        {
            auto& con = cons_[ c ];
            con.active = false;

            //left part: variables reachable from the left variable via active constraints (marked)
            ++mark_gen_;
            auto inLeft = [ & ] (int v) { return mark_[ v ] == mark_gen_; };

            std::vector<int> stack { con.left };

            mark_[ con.left ] = mark_gen_;

            while (!stack.empty())
            {
                const int v = stack.back();
                stack.pop_back();

                auto visit = [ & ] (int c2, int other)
                {
                    if (cons_[ c2 ].active && !inLeft(other))
                    {
                        mark_[ other ] = mark_gen_;
                        stack.push_back(other);
                    }
                };

                for (int c2 : vars_[ v ].out)
                    visit(c2, cons_[ c2 ].right);
                for (int c2 : vars_[ v ].in)
                    visit(c2, cons_[ c2 ].left);
            }

            const double           old_posn = blocks_[ b ].posn;
            const std::vector<int> old_vars = blocks_[ b ].vars;

            blocks_[ b ].alive = false;
            blocks_[ b ].vars.clear();

            const int bl = newBlock();
            const int br = newBlock();

            for (int v : old_vars)
                addToBlock(inLeft(v) ? bl : br, v);

            //the right part stays in place while the left part moves to its optimum and is merged leftwards
            blocks_[ br ].posn  = old_posn;
            blocks_[ br ].wposn = old_posn * blocks_[ br ].weight;

            mergeLeft(bl);

            //the right part (possibly merged meanwhile) moves to its optimum and is merged rightwards
            const int r = vars_[ con.right ].block;
            resetPosition(r);
            mergeRight(r);
        }

        bool refine()
        {
            const size_t max_splits = 10 * (vars_.size() + cons_.size()) + 100;

            size_t splits = 0;

            //blocks to check; a split only changes the blocks it ends up in (the multipliers of a block depend on
            //its own variables only), so only these are checked again
            std::vector<int> todo;
            todo.reserve(blocks_.size());
            for (size_t b = 0; b < blocks_.size(); ++b)
                todo.push_back((int)b);

            while (!todo.empty())
            {
                const int b = todo.back();
                todo.pop_back();

                if (!blocks_[ b ].alive || blocks_[ b ].vars.size() < 2)
                    continue;

                int min_c = -1;
                computeLM(blocks_[ b ].vars.front(), -1, min_c);

                if (min_c < 0 || cons_[ min_c ].lm >= LagrangeTolerance)
                    continue;

                if (++splits > max_splits)
                    return false;

                split(b, min_c);

                todo.push_back(vars_[ cons_[ min_c ].left  ].block);
                todo.push_back(vars_[ cons_[ min_c ].right ].block);
            }

            return true;
        }

        std::vector<Var>   vars_;
        std::vector<Con>   cons_;
        std::vector<Block> blocks_;

        std::vector<int> mark_;         // variable marks of split(), current mark = mark_gen_
        int              mark_gen_ = 0;
    };

    /**
     * Variables and constraints of one axis. Hard constraints between labels, soft bounds (violable at the cost
     * weight * violation^2) for obstacles and the region of interest, so the problem stays feasible (ordering
     * constraints without cycles plus soft bounds). A soft bound is a dummy variable at the bound with the soft
     * weight, separated from the label by a zero gap (same cost as a slack variable).
     */
    struct Axis
    {
        Axis(const std::vector<double>& targets, double weight)
        :   desired(targets)
        ,   weights(targets.size(), weight)
        {
        }

        //v[ right ] - v[ left ] >= gap
        void addHard(int left, int right, double gap)
        {
            constraints.push_back({ left, right, gap });
        }

        //v[ a ] >= bound, soft
        void addSoftLower(int a, double bound, double weight)
        {
            constraints.push_back({ addDummy(bound, weight), a, 0.0 });
        }

        //v[ a ] <= bound, soft
        void addSoftUpper(int a, double bound, double weight)
        {
            constraints.push_back({ a, addDummy(bound, weight), 0.0 });
        }

        bool solve(std::vector<double>& v) const
        {
            std::vector<double> all;

            AxisSolver solver(desired, weights, constraints);
            bool ok = solver.solve(all);

            v.assign(all.begin(), all.begin() + v.size());
            return ok;
        }

        bool empty() const { return constraints.empty(); }

        std::vector<double>     desired;
        std::vector<double>     weights;
        std::vector<Separation> constraints;

    private:
        int addDummy(double position, double weight)
        {
            desired.push_back(position);
            weights.push_back(weight);

            return (int)desired.size() - 1;
        }
    };

    /**
     * Directed graph of the ordering constraints of one axis, rejects edges closing a cycle (infeasible).
     */
    class OrderGraph
    {
    public:
        explicit OrderGraph(size_t n) : edges_(n), visited_(n, 0) {}

        bool reaches(int from, int to) const
        {
            ++visit_gen_;

            stack_.clear();
            stack_.push_back(from);

            while (!stack_.empty())
            {
                int v = stack_.back();
                stack_.pop_back();

                if (v == to)
                    return true;
                if (visited_[ v ] == visit_gen_)
                    continue;

                visited_[ v ] = visit_gen_;

                for (int w : edges_[ v ])
                    stack_.push_back(w);
            }

            return false;
        }

        //adds 'from' before 'to', fails if 'to' already reaches 'from'
        bool add(int from, int to)
        {
            if (reaches(to, from))
                return false;

            edges_[ from ].push_back(to);
            return true;
        }

    private:
        std::vector<std::vector<int>> edges_;

        mutable std::vector<int> visited_;       // visit marks of reaches(), current mark = visit_gen_
        mutable std::vector<int> stack_;
        mutable int              visit_gen_ = 0;
    };

    /**
     * Needed separation per direction for two boxes (x, y = top left), gaps included.
     */
    inline void separationCosts(double xi, double yi, double wi, double hi,
                                double xj, double yj, double wj, double hj,
                                double gap_x,
                                double gap_y,
                                double* cost)
    {
        cost[ Left  ] = xi + wi + gap_x - xj;
        cost[ Right ] = xj + wj + gap_x - xi;
        cost[ Up    ] = yi + hi + gap_y - yj;
        cost[ Down  ] = yj + hj + gap_y - yi;
    }

    /**
     * Candidate directions in order of preference: the previous one first if separated before and not too
     * expensive now, then by needed separation (ties in a fixed order => deterministic). The needed separation is
     * weighted by the size of the pair along the axis (cs_balance), so wide labels are not always stacked vertically.
     */
    inline std::vector<int> directionCandidates(const double* cost,
                                                const double* prev_cost,
                                                double min_w,
                                                double min_h,
                                                double size_x,
                                                double size_y,
                                                const Settings& settings)
    {
        const double norm_x = std::pow(std::max(size_x, 1.0), settings.cs_balance);
        const double norm_y = std::pow(std::max(size_y, 1.0), settings.cs_balance);

        auto norm  = [ & ] (int d) { return d == Left || d == Right ? norm_x : norm_y; };
        auto score = [ & ] (int d) { return cost[ d ] / norm(d); };

        std::vector<int> dirs { Left, Right, Up, Down };

        std::stable_sort(dirs.begin(), dirs.end(), [ & ] (int a, int b) { return score(a) < score(b); });

        //previous separation: the satisfied direction with the largest slack, within a tolerance relative to the
        //label size (minimal displacement solutions are tight, labels move relative to each other between runs)
        int    prev_dir   = None;
        double prev_slack = 0.0;

        for (int d = Left; d <= Down; ++d)
        {
            const double tolerance = settings.cs_prev_tolerance * (d == Left || d == Right ? min_w : min_h);

            double slack = -prev_cost[ d ];
            if (slack >= -tolerance && (prev_dir == None || slack > prev_slack))
            {
                prev_dir   = d;
                prev_slack = slack;
            }
        }

        if (prev_dir != None &&
            score(prev_dir) <= settings.cs_keep_factor * score(dirs.front()) + settings.cs_keep_abs / norm(prev_dir))
        {
            dirs.erase(std::find(dirs.begin(), dirs.end(), prev_dir));
            dirs.insert(dirs.begin(), prev_dir);
        }

        return dirs;
    }

    /**
     * Overlap area of two boxes.
     */
    inline double overlapArea(const QRectF& r0, const QRectF& r1)
    {
        const double ox = std::min(r0.right(),  r1.right())  - std::max(r0.left(), r1.left());
        const double oy = std::min(r0.bottom(), r1.bottom()) - std::max(r0.top(),  r1.top());

        return ox > 0 && oy > 0 ? ox * oy : 0.0;
    }

    /**
     * Distance between a point and a box (0 if inside).
     */
    inline double pointBoxDistance(double x, double y, const QRectF& r)
    {
        const double dx = std::max({ r.left() - x, 0.0, x - r.right()  });
        const double dy = std::max({ r.top()  - y, 0.0, y - r.bottom() });

        return std::hypot(dx, dy);
    }

    /**
     * Candidate box of a label in direction (sx, sy) from the anchor (each -1, 0, 1; screen coordinates, y down): the
     * box is placed so that its nearest point to the anchor lies at the given distance in that direction.
     */
    inline QRectF candidateBox(const Label& l, int sx, int sy, double dist)
    {
        const double norm = std::hypot((double)sx, (double)sy);
        const double ox   = dist * sx / norm;
        const double oy   = dist * sy / norm;

        const double x = l.x_anchor + ox + (sx > 0 ? 0.0 : (sx < 0 ? -l.w : -l.w / 2));
        const double y = l.y_anchor + oy + (sy > 0 ? 0.0 : (sy < 0 ? -l.h : -l.h / 2));

        return QRectF(x, y, l.w, l.h);
    }

    /**
     * Chooses a candidate position per label among positions around its anchor (4: diagonal, 8: diagonal and
     * axis-aligned), at the anchor distance of its reference position, independent of the reference direction.
     * Minimizes the overlap with the other labels, the obstacles and the area outside the region of interest;
     * leaving the candidate of the previous placement costs cs_cand_switch, leaving the reference position
     * cs_cand_penalty (fractions of the label area). Labels without a previous placement choose freely (ties =>
     * reference position). Deterministic sweeps over the labels until no label changes. Replaces hx/hy by the chosen
     * positions, flags labels which keep the candidate of their previous placement.
     */
    inline void chooseCandidates(const std::vector<Label>& labels,
                                 const std::vector<QRectF>& obstacles,
                                 const Settings& settings,
                                 std::vector<double>& hx,
                                 std::vector<double>& hy,
                                 std::vector<char>& keeps_prev,
                                 int* side_changes = nullptr)
    {
        const size_t n = labels.size();

        keeps_prev.assign(n, 0);
        for (size_t i = 0; i < n; ++i)
            keeps_prev[ i ] = labels[ i ].has_last ? 1 : 0;

        if (side_changes)
            *side_changes = 0;

        if (settings.cs_candidates < 4)
            return;

        //directions: diagonals first, then axis-aligned
        static const int Dirs[ 8 ][ 2 ] = { {  1, -1 }, { -1, -1 }, {  1,  1 }, { -1,  1 },
                                            {  1,  0 }, { -1,  0 }, {  0, -1 }, {  0,  1 } };

        const int nc = settings.cs_candidates >= 8 ? 8 : 4;

        std::vector<QRectF> cands(n * nc);
        std::vector<int>    ref(n, 0), prev(n, -1), cur(n, 0);
        std::vector<QRectF> reach(n); // union of all candidate boxes

        auto nearest = [ & ] (size_t i, const QRectF& box)
        {
            int    best   = 0;
            double best_d = std::numeric_limits<double>::max();

            for (int k = 0; k < nc; ++k)
            {
                const double d = QLineF(cands[ i * nc + k ].center(), box.center()).length();
                if (d < best_d)
                {
                    best   = k;
                    best_d = d;
                }
            }
            return best;
        };

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];

            const QRectF ref_box(hx[ i ], hy[ i ], l.w, l.h);
            const double dist = pointBoxDistance(l.x_anchor, l.y_anchor, ref_box);

            for (int k = 0; k < nc; ++k)
                cands[ i * nc + k ] = candidateBox(l, Dirs[ k ][ 0 ], Dirs[ k ][ 1 ], dist);

            ref [ i ] = nearest(i, ref_box);
            prev[ i ] = l.has_last ? nearest(i, QRectF(l.x_last, l.y_last, l.w, l.h)) : -1;
            cur [ i ] = prev[ i ] >= 0 ? prev[ i ] : ref[ i ];

            reach[ i ] = cands[ i * nc ];
            for (int k = 1; k < nc; ++k)
                reach[ i ] |= cands[ i * nc + k ];
        }

        //neighbours which can interact
        std::vector<std::vector<int>> neighbours(n), near_obstacles(n);
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                if (reach[ i ].intersects(reach[ j ]))
                {
                    neighbours[ i ].push_back((int)j);
                    neighbours[ j ].push_back((int)i);
                }
            }
            for (size_t o = 0; o < obstacles.size(); ++o)
                if (reach[ i ].intersects(obstacles[ o ]))
                    near_obstacles[ i ].push_back((int)o);
        }

        const bool   use_roi = settings.cs_keep_in_roi && !settings.roi.isEmpty();
        const double eps     = 1e-9;

        auto cost = [ & ] (size_t i, int k)
        {
            const QRectF& box  = cands[ i * nc + k ];
            const double  area = box.width() * box.height();

            double c = 0.0;
            if (k != ref[ i ])
                c += area * settings.cs_cand_penalty;
            if (prev[ i ] >= 0 && k != prev[ i ])
                c += area * settings.cs_cand_switch;

            for (int j : neighbours[ i ])
                c += overlapArea(box, cands[ j * nc + cur[ j ] ]);
            for (int o : near_obstacles[ i ])
                c += overlapArea(box, obstacles[ o ]);
            if (use_roi)
                c += area - overlapArea(box, settings.roi);

            return c;
        };

        for (int sweep = 0; sweep < settings.cs_cand_sweeps; ++sweep)
        {
            bool changed = false;

            for (size_t i = 0; i < n; ++i)
            {
                int    best      = cur[ i ];
                double best_cost = cost(i, cur[ i ]);

                for (int k = 0; k < nc; ++k)
                {
                    if (k == cur[ i ])
                        continue;

                    const double c = cost(i, k);
                    if (c < best_cost - eps)
                    {
                        best      = k;
                        best_cost = c;
                    }
                }

                if (best != cur[ i ])
                {
                    cur[ i ] = best;
                    changed  = true;
                }
            }

            if (!changed)
                break;
        }

        for (size_t i = 0; i < n; ++i)
        {
            const QRectF& box = cands[ i * nc + cur[ i ] ];

            hx[ i ] = box.left();
            hy[ i ] = box.top();

            keeps_prev[ i ] = prev[ i ] >= 0 && cur[ i ] == prev[ i ] ? 1 : 0;

            if (side_changes && prev[ i ] >= 0 && cur[ i ] != prev[ i ])
                ++(*side_changes);
        }
    }

    /**
     * Diagnostics of a placement run.
     */
    struct Stats
    {
        int  rounds           = 0;     // rounds of constraint generation
        bool rounds_exhausted = false; // new overlaps were still found in the last round
        bool solver_converged = true;  // all axis solves optimal and feasible
        int  unresolvable     = 0;     // overlapping pairs without an acyclic separation direction
        int  overlaps         = 0;     // overlapping label pairs left (without gap)
        int  side_changes     = 0;     // labels placed on another side of their anchor than before (candidates)
    };

    /**
     * Places the labels, see namespace description. Reference position = target (falls back to the initial
     * position), initial position = previous placement (only used for the separation directions).
     * Notice: Returns true unless an error occurred, also if overlaps remain (no feasible direction, too dense
     * for the region of interest, solver limits), see stats.
     */
    inline bool placeLabels(std::vector<Label>& labels,
                            const Settings& settings,
                            Stats* stats = nullptr)
    {
        Stats tmp_stats;
        Stats& st = stats ? *stats : tmp_stats;
        st = Stats();

        const size_t n = labels.size();
        if (n == 0)
            return true;

        const double gap_x = settings.cs_gap_x;
        const double gap_y = settings.cs_gap_y;

        std::vector<double> hx(n), hy(n), px(n), py(n), w(n), h(n);

        for (size_t i = 0; i < n; ++i)
        {
            const auto& l = labels[ i ];

            hx[ i ] = l.x_ref.has_value() ? l.x_ref.value() : l.x_last;
            hy[ i ] = l.y_ref.has_value() ? l.y_ref.value() : l.y_last;
            px[ i ] = l.x_last;
            py[ i ] = l.y_last;
            w [ i ] = l.w;
            h [ i ] = l.h;
        }

        //fixed obstacles: anchors
        std::vector<QRectF> obstacles;
        if (settings.cs_avoid_anchors && settings.cs_anchor_radius > 0)
        {
            const double r = settings.cs_anchor_radius;

            obstacles.reserve(n);
            for (const auto& l : labels)
                obstacles.emplace_back(l.x_anchor - r, l.y_anchor - r, 2 * r, 2 * r);
        }

        const size_t no = obstacles.size();

        const double soft = settings.cs_soft_weight;

        //candidate positions: home on the side of the anchor with the least conflicts
        std::vector<char> keeps_prev;
        chooseCandidates(labels, obstacles, settings, hx, hy, keeps_prev, &st.side_changes);

        //labels without a valid previous placement (no history, changed sides): no previous separation sides, no
        //smoothness
        for (size_t i = 0; i < n; ++i)
        {
            if (keeps_prev[ i ])
                continue;

            px[ i ] = hx[ i ];
            py[ i ] = hy[ i ];
        }

        //targets: home, optionally blended with the previous placement (smoothness between runs, a label without
        //conflict returns home gradually):
        //  w_home * (v - home)^2 + w_prev * (v - prev)^2 = (w_home + w_prev) * (v - target)^2 + const
        const double w_prev = std::max(0.0, settings.cs_weight_prev);

        std::vector<double> tx(n), ty(n);
        for (size_t i = 0; i < n; ++i)
        {
            const double wp = keeps_prev[ i ] ? w_prev : 0.0;

            tx[ i ] = (hx[ i ] + wp * px[ i ]) / (1.0 + wp);
            ty[ i ] = (hy[ i ] + wp * py[ i ]) / (1.0 + wp);
        }

        Axis ax(tx, 1.0 + w_prev), ay(ty, 1.0 + w_prev);

        //region of interest as (soft) bounds
        if (settings.cs_keep_in_roi && !settings.roi.isEmpty())
        {
            const auto& roi = settings.roi;

            for (size_t i = 0; i < n; ++i)
            {
                ax.addSoftLower((int)i, roi.left(),            soft);
                ax.addSoftUpper((int)i, roi.right()  - w[ i ], soft);
                ay.addSoftLower((int)i, roi.top(),             soft);
                ay.addSoftUpper((int)i, roi.bottom() - h[ i ], soft);
            }
        }

        std::vector<signed char> pair_dir    (n * n , None);
        std::vector<signed char> obstacle_dir(n * no, None);

        OrderGraph graph_x(n), graph_y(n);

        std::vector<double> x = tx, y = ty;

        auto overlaps = [ & ] (double x0, double y0, double w0, double h0,
                               double x1, double y1, double w1, double h1)
        {
            return x0 < x1 + w1 + gap_x && x1 < x0 + w0 + gap_x &&
                   y0 < y1 + h1 + gap_y && y1 < y0 + h0 + gap_y;
        };

        bool solved = false;

        for (int round = 0; round < settings.cs_max_rounds; ++round)
        {
            bool added = false;

            st.rounds = round + 1;

            //label pairs
            for (size_t i = 0; i < n; ++i)
            {
                for (size_t j = i + 1; j < n; ++j)
                {
                    auto& dir = pair_dir[ i * n + j ];
                    if (dir != None)
                        continue;

                    if (!overlaps(x[ i ], y[ i ], w[ i ], h[ i ], x[ j ], y[ j ], w[ j ], h[ j ]))
                        continue;

                    double cost[ 4 ], prev_cost[ 4 ];
                    separationCosts(x [ i ], y [ i ], w[ i ], h[ i ], x [ j ], y [ j ], w[ j ], h[ j ], gap_x, gap_y, cost);
                    separationCosts(px[ i ], py[ i ], w[ i ], h[ i ], px[ j ], py[ j ], w[ j ], h[ j ], gap_x, gap_y, prev_cost);

                    for (int d : directionCandidates(cost, prev_cost,
                                                     std::min(w[ i ], w[ j ]), std::min(h[ i ], h[ j ]),
                                                     (w[ i ] + w[ j ]) / 2, (h[ i ] + h[ j ]) / 2,
                                                     settings))
                    {
                        const int a = (int)i;
                        const int b = (int)j;

                        bool ok = false;

                        if (d == Left)
                            ok = graph_x.add(a, b);
                        else if (d == Right)
                            ok = graph_x.add(b, a);
                        else if (d == Up)
                            ok = graph_y.add(a, b);
                        else
                            ok = graph_y.add(b, a);

                        if (!ok)
                            continue;

                        //second - first >= size of the first + gap (hard)
                        if (d == Left)
                            ax.addHard(a, b, w[ i ] + gap_x);
                        else if (d == Right)
                            ax.addHard(b, a, w[ j ] + gap_x);
                        else if (d == Up)
                            ay.addHard(a, b, h[ i ] + gap_y);
                        else
                            ay.addHard(b, a, h[ j ] + gap_y);

                        dir   = (signed char)d;
                        added = true;
                        break;
                    }
                }
            }

            //labels vs obstacles
            for (size_t i = 0; i < n; ++i)
            {
                for (size_t o = 0; o < no; ++o)
                {
                    auto& dir = obstacle_dir[ i * no + o ];
                    if (dir != None)
                        continue;

                    const auto& ob = obstacles[ o ];

                    if (!overlaps(x[ i ], y[ i ], w[ i ], h[ i ], ob.left(), ob.top(), ob.width(), ob.height()))
                        continue;

                    double cost[ 4 ], prev_cost[ 4 ];
                    separationCosts(x [ i ], y [ i ], w[ i ], h[ i ], ob.left(), ob.top(), ob.width(), ob.height(), gap_x, gap_y, cost);
                    separationCosts(px[ i ], py[ i ], w[ i ], h[ i ], ob.left(), ob.top(), ob.width(), ob.height(), gap_x, gap_y, prev_cost);

                    const int d = directionCandidates(cost, prev_cost,
                                                      std::min(w[ i ], ob.width()), std::min(h[ i ], ob.height()),
                                                      (w[ i ] + ob.width()) / 2, (h[ i ] + ob.height()) / 2,
                                                      settings).front();
                    const int a = (int)i;

                    //soft: a label squeezed between obstacles may cover one rather than another label
                    if (d == Left)
                        ax.addSoftUpper(a, ob.left() - w[ i ] - gap_x,      soft);
                    else if (d == Right)
                        ax.addSoftLower(a, ob.left() + ob.width() + gap_x,  soft);
                    else if (d == Up)
                        ay.addSoftUpper(a, ob.top() - h[ i ] - gap_y,       soft);
                    else
                        ay.addSoftLower(a, ob.top() + ob.height() + gap_y,  soft);

                    dir   = (signed char)d;
                    added = true;
                }
            }

            //no new overlaps => done (but solve at least once if there are constraints, e.g. the region of interest)
            if (!added && (solved || (ax.empty() && ay.empty())))
                break;

            st.rounds_exhausted = added && round + 1 == settings.cs_max_rounds;

            //minimal displacement from the reference positions per axis
            if (!ax.solve(x))
                st.solver_converged = false;
            if (!ay.solve(y))
                st.solver_converged = false;

            solved = true;
        }

        for (size_t i = 0; i < n; ++i)
        {
            labels[ i ].x = x[ i ];
            labels[ i ].y = y[ i ];
        }

        //diagnostics: pairs left overlapping (without gap), pairs without a feasible direction
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                if (x[ i ] < x[ j ] + w[ j ] && x[ j ] < x[ i ] + w[ i ] &&
                    y[ i ] < y[ j ] + h[ j ] && y[ j ] < y[ i ] + h[ i ])
                {
                    ++st.overlaps;

                    if (pair_dir[ i * n + j ] == None)
                        ++st.unresolvable;
                }
            }
        }

        return true;
    }

} //namespace constraint
} //namespace label_placement
