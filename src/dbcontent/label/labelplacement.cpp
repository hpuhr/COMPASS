
#include "labelplacement.h"
#include "labelplacement_force.h"
#include "labelplacement_spring.h"

#include <limits>
#include <iostream>

#include <Eigen/Core>

#include <QPainter>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTimer>
#include <QTime>

/**
 */
LabelPlacementEngine::LabelPlacementEngine() = default;

/**
 * Add a new label.
 */
void LabelPlacementEngine::addLabel(const std::string& id, 
                                    double x_anchor, 
                                    double y_anchor, 
                                    double w,
                                    double h,
                                    bool active, 
                                    double* x_init, 
                                    double* y_init)
{
    Label l;
    l.id       = id;
    l.x_anchor = x_anchor;
    l.y_anchor = y_anchor;
    l.active   = active;
    l.x        = x_init ? *x_init : x_anchor;
    l.y        = y_init ? *y_init : y_anchor;
    l.x_last   = l.x;
    l.y_last   = l.y;
    l.w        = w;
    l.h        = h;

    auto it = label_map_.find(id);

    //label already registered -> overwrite
    if (it != label_map_.end())
    {
        labels_[ it->second ] = l;
        return;
    }

    //register new label
    size_t idx = labels_.size();

    labels_.push_back(l);
    label_map_[ id ] = idx;
}

/**
 * Obtain label by index.
 */
const LabelPlacementEngine::Label& LabelPlacementEngine::getLabel(size_t idx) const
{
    return labels_.at(idx);
}


/**
 * Obtain label by id.
 */
const LabelPlacementEngine::Label* LabelPlacementEngine::getLabel(const std::string& id) const
{
    auto it = label_map_.find(id);
    if (it == label_map_.end())
        return nullptr;

    return &labels_[ it->second ];
}

/**
 */
void LabelPlacementEngine::removeLabel(size_t idx)
{
    const auto& l = labels_.at(idx);

    label_map_.erase(l.id);
    labels_.erase(labels_.begin() + idx);
}

/**
 */
void LabelPlacementEngine::removeLabel(const std::string& id)
{
    auto it = label_map_.find(id);
    if (it == label_map_.end())
        return;
  
    labels_.erase(labels_.begin() + it->second);
    label_map_.erase(id);
}

/**
 */
void LabelPlacementEngine::clearLabels()
{
    labels_.clear();
    label_map_.clear();
}

/**
 * Reverts all label positions to initial positions.
 */
void LabelPlacementEngine::revertPlacements()
{
    for (auto& l : labels_)
    {
        l.revert();
    }    
}

/**
 * Main method for running automated label placement.
 */
bool LabelPlacementEngine::placeLabels()
{
    bool ok = false;
    if (settings_.method == Method::ForceBased)
        ok = placeLabelsForceBased();
    else if (settings_.method == Method::SpringBased)
        ok = placeLabelsSpringBased();
    else
        return false; //unknown method

    //if something went wrong revert positions to initial ones
    if (!ok)
    {
        revertPlacements();
        return false;
    }

    return true;
}

/**
 */
bool LabelPlacementEngine::placeLabelsForceBased()
{
    /*
    """Iteratively adjusts the locations of texts.
    Call adjust_text the very last, after all plotting (especially
    anything that can change the axes limits) has been done. This is
    because to move texts the function needs to use the dimensions of
    the axes, and without knowing the final size of the plots the
    results will be completely nonsensical, or suboptimal.
    First moves all texts that are outside the axes limits
    inside. Then in each iteration moves all texts away from each
    other and from points. In the end hides texts and substitutes them
    with annotations to link them to the respective points.
    Parameters
    ----------
    texts : list
        A list of :obj:`matplotlib.text.Text` objects to adjust.
    Other Parameters
    ----------------
    x : array_like
        x-coordinates of points to repel from; if not provided only uses text
        coordinates.
    y : array_like
        y-coordinates of points to repel from; if not provided only uses text
        coordinates
    add_objects : list or PathCollection
        a list of additional matplotlib objects to avoid; they must have a
        `.get_window_extent()` method; alternatively, a PathCollection or a
        list of Bbox objects.
    ax : matplotlib axe, default is current axe (plt.gca())
        axe object with the plot
    expand_text : array_like, default (1.05, 1.2)
        a tuple/list/... with 2 multipliers (x, y) by which to expand the
        bounding box of texts when repelling them from each other.
    expand_points : array_like, default (1.05, 1.2)
        a tuple/list/... with 2 multipliers (x, y) by which to expand the
        bounding box of texts when repelling them from points.
    expand_objects : array_like, default (1.05, 1.2)
        a tuple/list/... with 2 multipliers (x, y) by which to expand the
        bounding box of texts when repelling them from other objects.
    expand_align : array_like, default (1.05, 1.2)
        a tuple/list/... with 2 multipliers (x, y) by which to expand the
        bounding box of texts when autoaligning texts.
    autoalign: str or boolean {'xy', 'x', 'y', True, False}, default 'xy'
        Direction in wich the best alignement will be determined
        - 'xy' or True, best alignment of all texts determined in all
          directions automatically before running the iterative adjustment
          (overriding va and ha),
        - 'x', will only align horizontally,
        - 'y', will only align vertically,
        - False, do nothing (i.e. preserve va and ha)
    va : str, default 'center'
        vertical alignment of texts
    ha : str, default 'center'
        horizontal alignment of texts,
    force_text : tuple, default (0.1, 0.25)
        the repel force from texts is multiplied by this value
    force_points : tuple, default (0.2, 0.5)
        the repel force from points is multiplied by this value
    force_objects : float, default (0.1, 0.25)
        same as other forces, but for repelling additional objects
    lim : int, default 500
        limit of number of iterations
    precision : float, default 0.01
        iterate until the sum of all overlaps along both x and y are less than
        this amount, as a fraction of the total widths and heights,
        respectively. May need to increase for complicated situations.
    only_move : dict, default {'points':'xy', 'text':'xy', 'objects':'xy'}
        a dict to restrict movement of texts to only certain axes for certain
        types of overlaps.
        Valid keys are 'points', 'text', and 'objects'.
        Valid values are '', 'x', 'y', and 'xy'.
        For example, only_move={'points':'y', 'text':'xy', 'objects':'xy'}
        forbids moving texts along the x axis due to overlaps with points.
    avoid_text : bool, default True
        whether to repel texts from each other.
    avoid_points : bool, default True
        whether to repel texts from points. Can be helpful to switch off in
        extremely crowded plots.
    avoid_self : bool, default True
        whether to repel texts from its original positions.
    save_steps : bool, default False
        whether to save intermediate steps as images.
    save_prefix : str, default ''
        if `save_steps` is True, a path and/or prefix to the saved steps.
    save_format : str, default 'png'
        if `save_steps` is True, a format to save the steps into.
    add_step_numbers : bool, default True
        if `save_steps` is True, whether to add step numbers as titles to the
        images of saving steps.
    args and kwargs :
        any arguments will be fed into obj:`ax.annotate` after all the
        optimization is done just for plotting the connecting arrows if
        required.
    Return
    ------
    int
        Number of iteration
    """
    plt.draw()
    ax = ax or plt.gca()
    r = get_renderer(ax.get_figure())
    transform = texts[0].get_transform()
    if (x is not None) & (y is not None):
        for ix, tupxy in enumerate(zip(x, y)):
            t_x, t_y = transform.transform(tupxy)
            x[ix] = t_x
            y[ix] = t_y
    orig_xy = [get_text_position(text, ax) for text in texts]
    orig_x = [xy[0] for xy in orig_xy]
    orig_y = [xy[1] for xy in orig_xy]
    force_objects = float_to_tuple(force_objects)
    force_text = float_to_tuple(force_text)
    force_points = float_to_tuple(force_points)

    #    xdiff = np.diff(ax.get_xlim())[0]
    #    ydiff = np.diff(ax.get_ylim())[0]

    bboxes = get_bboxes(texts, r, (1.0, 1.0), ax)
    sum_width = np.sum(list(map(lambda bbox: bbox.width, bboxes)))
    sum_height = np.sum(list(map(lambda bbox: bbox.height, bboxes)))
    if not any(list(map(lambda val: "x" in val, only_move.values()))):
        precision_x = np.inf
    else:
        precision_x = precision * sum_width
    #
    if not any(list(map(lambda val: "y" in val, only_move.values()))):
        precision_y = np.inf
    else:
        precision_y = precision * sum_height

    if x is None:
        if y is None:
            if avoid_self:
                x, y = orig_x, orig_y
            else:
                x, y = [], []
        else:
            raise ValueError("Please specify both x and y, or neither")
    if y is None:
        raise ValueError("Please specify both x and y, or neither")
    if add_objects is None:
        text_from_objects = False
        add_bboxes = []
    else:
        try:
            add_bboxes = get_bboxes(add_objects, r, (1, 1), ax)
        except:
            raise ValueError(
                "Can't get bounding boxes from add_objects - is'\
                             it a flat list of matplotlib objects?"
            )
            return
        text_from_objects = True
    for text in texts:
        text.set_va(va)
        text.set_ha(ha)
    if save_steps:
        if add_step_numbers:
            plt.title("Before")
        plt.savefig(
            "%s%s.%s" % (save_prefix, "000a", save_format), format=save_format, dpi=150
        )

    if autoalign:
        if autoalign is True:
            autoalign = "xy"
        for i in range(2):
            texts = optimally_align_text(
                x,
                y,
                texts,
                expand=expand_align,
                add_bboxes=add_bboxes,
                direction=autoalign,
                renderer=r,
                ax=ax,
            )

    if save_steps:
        if add_step_numbers:
            plt.title("Autoaligned")
        plt.savefig(
            "%s%s.%s" % (save_prefix, "000b", save_format), format=save_format, dpi=150
        )

    texts = repel_text_from_axes(texts, ax, renderer=r, expand=expand_points)
    history = [(np.inf, np.inf)] * 10
    for i in range(lim):
        #        q1, q2 = [np.inf, np.inf], [np.inf, np.inf]

        if avoid_text:
            d_x_text, d_y_text, q1 = repel_text(
                texts, renderer=r, ax=ax, expand=expand_text
            )
        else:
            d_x_text, d_y_text, q1 = [0] * len(texts), [0] * len(texts), (0, 0)

        if avoid_points:
            d_x_points, d_y_points, q2 = repel_text_from_points(
                x, y, texts, ax=ax, renderer=r, expand=expand_points
            )
        else:
            d_x_points, d_y_points, q2 = [0] * len(texts), [0] * len(texts), (0, 0)

        if text_from_objects:
            d_x_objects, d_y_objects, q3 = repel_text_from_bboxes(
                add_bboxes, texts, ax=ax, renderer=r, expand=expand_objects
            )
        else:
            d_x_objects, d_y_objects, q3 = [0] * len(texts), [0] * len(texts), (0, 0)

        if only_move:
            if "text" in only_move:
                if "x" not in only_move["text"]:
                    d_x_text = np.zeros_like(d_x_text)
                if "y" not in only_move["text"]:
                    d_y_text = np.zeros_like(d_y_text)
            if "points" in only_move:
                if "x" not in only_move["points"]:
                    d_x_points = np.zeros_like(d_x_points)
                if "y" not in only_move["points"]:
                    d_y_points = np.zeros_like(d_y_points)
            if "objects" in only_move:
                if "x" not in only_move["objects"]:
                    d_x_objects = np.zeros_like(d_x_objects)
                if "y" not in only_move["objects"]:
                    d_y_objects = np.zeros_like(d_y_objects)

        dx = (
            np.array(d_x_text) * force_text[0]
            + np.array(d_x_points) * force_points[0]
            + np.array(d_x_objects) * force_objects[0]
        )
        dy = (
            np.array(d_y_text) * force_text[1]
            + np.array(d_y_points) * force_points[1]
            + np.array(d_y_objects) * force_objects[1]
        )
        qx = np.sum([q[0] for q in [q1, q2, q3]])
        qy = np.sum([q[1] for q in [q1, q2, q3]])
        histm = np.max(np.array(history), axis=0)
        history.pop(0)
        history.append((qx, qy))
        move_texts(texts, dx, dy, bboxes=get_bboxes(texts, r, (1, 1), ax), ax=ax)
        if save_steps:
            if add_step_numbers:
                plt.title(i + 1)
            plt.savefig(
                "%s%s.%s" % (save_prefix, "{0:03}".format(i + 1), save_format),
                format=save_format,
                dpi=150,
            )
        # Stop if we've reached the precision threshold, or if the x and y displacement
        # are both greater than the max over the last 10 iterations (suggesting a
        # failure to converge)
        if (qx < precision_x and qy < precision_y) or np.all([qx, qy] >= histm):
            break
        # Now adding arrows from texts to their original locations if required
    if "arrowprops" in kwargs:
        bboxes = get_bboxes(texts, r, (1, 1), ax)
        kwap = kwargs.pop("arrowprops")
        for j, (bbox, text) in enumerate(zip(bboxes, texts)):
            ap = {"patchA": text}  # Ensure arrow is clipped by the text
            ap.update(kwap)  # Add arrowprops from kwargs
            ax.annotate(
                "",  # Add an arrow from the text to the point
                xy=get_orig_coords(transform, orig_x[j], orig_y[j]),
                xytext=transform.inverted().transform(get_midpoint(bbox)),
                arrowprops=ap,
                xycoords=transform,
                textcoords=transform,
                *args,
                **kwargs
            )

    if save_steps:
        if add_step_numbers:
            plt.title(i + 1)
            plt.savefig(
                "%s%s.%s" % (save_prefix, "{0:03}".format(i + 1), save_format),
                format=save_format,
                dpi=150,
            )

    return i + 1
    */

   size_t n = labels_.size();

    //compute convergence limits
    auto bboxes = collectBoundingBoxes(labels_, 1, 1);
    double sum_widths  = 0.0;
    double sum_heights = 0.0;
    for (const auto& bbox : bboxes)
    {
        if (bbox.isEmpty())
            continue;
        sum_widths  += bbox.width();
        sum_heights += bbox.height(); 
    }
    double tol_x = sum_widths  < std::numeric_limits<double>::epsilon() ? std::numeric_limits<double>::max() : sum_widths  * settings_.fb_precision;
    double tol_y = sum_heights < std::numeric_limits<double>::epsilon() ? std::numeric_limits<double>::max() : sum_heights * settings_.fb_precision;

    //collect anchor points
    std::vector<QPointF> anchors;
    std::vector<QRectF>  anchor_regions;
    anchors.reserve(n);
    for (const auto& l : labels_)
    {
        if (!l.active)
            continue;
        
        anchors.emplace_back(l.x_anchor, l.y_anchor);

        if (settings_.fb_anchor_radius > 0)
        {
            const double r = settings_.fb_anchor_radius;
            anchor_regions.emplace_back(l.x_anchor - r, l.y_anchor - r, 2 * r, 2 * r);
        }
    }

    std::vector<Eigen::Vector2d> displacements(n);

    std::vector<Eigen::Vector2d> displacements_labels(n);
    std::vector<Eigen::Vector2d> displacements_anchors(n);
    std::vector<Eigen::Vector2d> displacements_objects(n);
    std::vector<Eigen::Vector2d> displacements_roi(n);

    bool   converged = false;
    double last_dx   = 0.0;
    double last_dy   = 0.0;
    int    used_iter = -1;
    
    //iterate up to max iterations
    for (int i = 0; i < settings_.fb_max_iter; ++i)
    {
        displacements.assign(n, {0, 0});

        displacements_labels.assign(n, {0, 0});
        displacements_anchors.assign(n, {0, 0});
        displacements_objects.assign(n, {0, 0});
        displacements_roi.assign(n, {0, 0});
           
        Eigen::Vector2d total(0, 0);

        Eigen::Vector2d total_labels(0, 0);
        Eigen::Vector2d total_anchors(0, 0);
        Eigen::Vector2d total_objects(0, 0);
        Eigen::Vector2d total_roi(0, 0);

        //compute various displacements by repelling from certain types of obstacles
        if (settings_.fb_avoid_labels)
        {
            label_placement::force::repelFromLabels(displacements_labels, 
                                                    total_labels, 
                                                    labels_, 
                                                    settings_.fb_expand_x, 
                                                    settings_.fb_expand_y, 
                                                    settings_.fb_force_type);
        }        
        if (settings_.fb_avoid_anchors)
        {
            if (settings_.fb_anchor_radius > 0)
            {
                label_placement::force::repelFromObjects(displacements_anchors, 
                                                         total_anchors, 
                                                         anchor_regions, 
                                                         labels_, 
                                                         settings_.fb_expand_x, 
                                                         settings_.fb_expand_y, 
                                                         settings_.fb_force_type);
            }
            else
            {
                label_placement::force::repelFromPoints(displacements_anchors, 
                                                        total_anchors, 
                                                        anchors, 
                                                        labels_, 
                                                        settings_.fb_expand_x, 
                                                        settings_.fb_expand_y, 
                                                        settings_.fb_force_type);
            }
            
        }
        if (settings_.fb_avoid_objects && !settings_.additional_objects.empty())
        {
            label_placement::force::repelFromObjects(displacements_objects, 
                                                     total_objects, 
                                                     settings_.additional_objects, 
                                                     labels_, 
                                                     settings_.fb_expand_x, 
                                                     settings_.fb_expand_y, 
                                                     settings_.fb_force_type);
        }
        if (settings_.fb_avoid_roi && !settings_.roi.isEmpty())
        {
            label_placement::force::repelFromROI(displacements_roi,
                                                 total_roi,
                                                 settings_.roi,
                                                 labels_,
                                                 settings_.fb_expand_x, 
                                                 settings_.fb_expand_y, 
                                                 settings_.fb_force_type);
        }

        //sum up individual displacements using weights
        for (size_t j = 0; j < n; ++j)
        {
            displacements[ j ] += displacements_labels [ j ] * settings_.fb_weight_labels;
            displacements[ j ] += displacements_anchors[ j ] * settings_.fb_weight_anchors;
            displacements[ j ] += displacements_objects[ j ] * settings_.fb_weight_objects;
            displacements[ j ] += displacements_roi    [ j ] * settings_.fb_weight_roi;
        }

        //sum up total absolute movement
        total += total_labels;
        total += total_anchors;
        total += total_objects;
        total += total_roi;

        // if (settings_.verbose)
        // {
        //     std::cout << "   labels:  " << total_labels.x() << " / " << total_labels.y() << std::endl;
        //     std::cout << "   anchors: " << total_anchors.x() << " / " << total_anchors.y() << std::endl;
        //     std::cout << "   objects: " << total_objects.x() << " / " << total_objects.y() << std::endl;
        //     std::cout << "   roi:     " << total_roi.x() << " / " << total_roi.y() << std::endl;
        // }

        //move labels
        for (size_t j = 0; j < n; ++j)
        {
            labels_[ j ].x += displacements[ j ].x();
            labels_[ j ].y += displacements[ j ].y();
        }

        last_dx = total.x();
        last_dy = total.y();
        
        //convergence if total movement is below precomputed threshold
        if (total.x() <= tol_x && total.y() < tol_y)
        {
            converged = true;
            used_iter = i + 1;

            break;
        }
    }

    if (settings_.verbose)
        std::cout << (converged ? "CONVERGED" : "MAX ITER") << " dx: " << last_dx << "/" << tol_x << " dy: " << last_dy << "/" << tol_y << std::endl;

    //convergence or max iter reached
    return true;
}

/**
 */
bool LabelPlacementEngine::placeLabelsSpringBased()
{
    return false;
}

/**
 */
void LabelPlacementEngine::showData(const TestConfig& test_config) const
{
    size_t n = labels_.size();

    if (n < 2)
        return;

    std::vector<TestLabel> test_labels(n);
    for (size_t i = 0; i < n; ++i)
    {
        const auto& l = labels_[ i ];

        auto& tl = test_labels[ i ];
        tl.x        = l.x;
        tl.y        = l.y;
        tl.x_anchor = l.x_anchor;
        tl.y_anchor = l.y_anchor;
        tl.x_init   = l.x_last;
        tl.y_init   = l.y_last;
        tl.w        = l.w;
        tl.h        = l.h;
        tl.txt      = l.id;
    }

    auto config = test_config;
    
    if (settings_.fb_avoid_anchors && settings_.fb_anchor_radius > 0)
    {
        config.radius        = settings_.fb_anchor_radius;
        config.avoid_anchors = true;
    }
    if (settings_.fb_avoid_roi && !settings_.roi.isEmpty())
    {
        config.roi       = settings_.roi;
        config.avoid_roi = true;
    }

    //convert coords to test window size in pixels
    computeScreenTransform(config, test_labels);

    //create test dialog
    QDialog dlg;

    QVBoxLayout* layoutv = new QVBoxLayout;
    dlg.setLayout(layoutv);

    QLabel* label = new QLabel;
    label->setFixedSize(config.width, config.height);

    QPushButton* closeButton = new QPushButton("Close");

    layoutv->addWidget(label);
    layoutv->addWidget(closeButton);

    QObject::connect(closeButton, &QPushButton::pressed, &dlg, &QDialog::accept);

    QImage img(config.width, config.height, QImage::Format_RGB32);
    img.fill(Qt::white);

    renderTestFrame(img, test_labels, config);
        
    label->setPixmap(QPixmap::fromImage(img));

    dlg.setWindowModality(Qt::WindowModality::ApplicationModal);
    dlg.exec();
}

/**
 */
void LabelPlacementEngine::runTest(const TestConfig& test_config) const
{
    std::vector<Label> labels;

    int n = test_config.num_objects;

    std::vector<TestLabel> test_labels(n);

    //init random labels/positions
    for (int i = 0; i < n; ++i)
    {
        auto& l = test_labels[ i ];

        Eigen::Vector2d pos, dir;
        pos.setRandom();
        dir.setRandom();
        dir.normalize();

        pos += Eigen::Vector2d(1, 1);
        pos *= 0.5;

        Eigen::Vector2d v;
        v.setRandom();

        double speed = 0.9 + 0.1 * v.x();

        l.x        = pos.x();
        l.y        = pos.y();
        l.x_anchor = l.x;
        l.y_anchor = l.y;
        l.x_init   = l.x;
        l.y_init   = l.y;
        l.w        = test_config.label_w;
        l.h        = test_config.label_h;
        l.dirx     = dir.x();
        l.diry     = dir.y();
        l.speed    = speed;
        l.txt      = "Test" + std::to_string(i); //unique id is important
    }

    test_config.roi = QRectF(0, 0, 1, 1);

    //convert coords to test window size in pixels
    computeScreenTransform(test_config, test_labels);

    //show test data in dialog
    runTest(test_labels, test_config);
}

/**
 */
void LabelPlacementEngine::runTest(const std::vector<TestLabel>& test_labels,
                                   const TestConfig& test_config) const
{
    auto config = test_config;

    std::vector<TestLabel> labels = test_labels;

    //create test dialog
    QDialog dlg;

    QHBoxLayout* layouth = new QHBoxLayout;
    dlg.setLayout(layouth);

    QLabel* label = new QLabel;
    label->setFixedSize(config.width, config.height);

    bool runs = false;

    QVBoxLayout* layoutv = new QVBoxLayout;

    layouth->addWidget(label);
    layouth->addLayout(layoutv);

    QPushButton* runButton = new QPushButton("Run");
    QPushButton* closeButton = new QPushButton("Close");

    QCheckBox* avoidAnchorsBox = new QCheckBox("Avoid anchors");
    QCheckBox* avoidROIBox = new QCheckBox("Avoid ROI");

    QDoubleSpinBox* radiusBox = new QDoubleSpinBox;
    radiusBox->setDecimals(6);
    radiusBox->setMinimum(0);
    radiusBox->setMaximum(1000);
    radiusBox->setValue(config.radius);

    QComboBox* autoCombo = new QComboBox;
    autoCombo->addItem("None", QVariant(-1));
    autoCombo->addItem("Force", QVariant((int)LabelPlacementEngine::Method::ForceBased));
    autoCombo->addItem("Spring", QVariant((int)LabelPlacementEngine::Method::SpringBased));

    QComboBox* forceCombo = new QComboBox;
    forceCombo->addItem("Simple", QVariant((int)LabelPlacementEngine::ForceType::Simple));
    forceCombo->addItem("Exact", QVariant((int)LabelPlacementEngine::ForceType::Exact));

    layoutv->addWidget(autoCombo);
    layoutv->addWidget(forceCombo);
    layoutv->addWidget(avoidAnchorsBox);
    layoutv->addWidget(avoidROIBox);
    layoutv->addWidget(radiusBox);

    layoutv->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding));

    layoutv->addWidget(runButton);
    layoutv->addWidget(closeButton);

    QObject::connect(closeButton, &QPushButton::pressed, &dlg, &QDialog::accept);

    //callback for the run button
    auto runCB = [&] () 
    {
        if (runs)
            runButton->setText("Run");
        else
            runButton->setText("Pause");

        runs = !runs;
    };

    QObject::connect(runButton, &QPushButton::pressed, runCB);

    int n = (int)test_labels.size();

    //updates the display every time the timer elapses
    auto updateCB = [ & ] () 
    {
        if (!runs || n < 1)
            return;

        config.radius = radiusBox->value();
        config.avoid_roi = avoidROIBox->isChecked();
        config.avoid_anchors = avoidAnchorsBox->isChecked();

        const int w = test_config.width;
        const int h = test_config.height;

        //init display image
        QImage img(w, h, QImage::Format_RGB32);
        img.fill(Qt::white);

        //update position and init labels
        for (int i = 0; i < n; ++i)
        {
            auto& l = labels[ i ];

            //this is in label space
            l.x_anchor += l.dirx * config.speed * l.speed;
            l.y_anchor += l.diry * config.speed * l.speed;

            if (config.avoid_roi && !config.roi.isEmpty())
            {
                if (l.x_anchor < config.roi.left() || l.y_anchor < config.roi.top() || l.x_anchor > config.roi.right() || l.y_anchor > config.roi.bottom())
                {
                    l.dirx *= -1;
                    l.diry *= -1;
                }
            }

            l.x = l.x_anchor + config.label_offs;
            l.y = l.y_anchor - config.label_offs;
            l.x_init = l.x;
            l.y_init = l.y;
        }

        int mode = autoCombo->currentData().toInt();

        //place labels automatically if desired
        if (mode >= 0)
        {
            QTime t;
            t.restart();

            LabelPlacementEngine::Method    method = (LabelPlacementEngine::Method)mode;
            LabelPlacementEngine::ForceType ftype  = (LabelPlacementEngine::ForceType)forceCombo->currentData().toInt();

            LabelPlacementEngine labelPlacement;
            auto& settings = labelPlacement.settings();
            settings.method           = method;
            settings.fb_avoid_anchors = config.avoid_anchors;
            settings.fb_force_type    = ftype;
            settings.fb_anchor_radius = config.radius;
            settings.fb_avoid_roi     = config.avoid_roi;
            settings.roi              = config.roi;

            //collect labels to place
            for (int i = 0; i < n; ++i)
            {
                auto& l = labels[ i ];
                labelPlacement.addLabel(l.txt, l.x_anchor, l.y_anchor, l.w, l.h, true, &l.x, &l.y);
            }

            //run placement
            labelPlacement.placeLabels();

            //retrieve optimized position
            for (int i = 0; i < n; ++i)
            {
                auto& l = labels[ i ];
                const auto& l_opt = labelPlacement.getLabel(i);
                l.x = l_opt.x;
                l.y = l_opt.y;
            }

            std::cout << "Auto placement of labels in " << t.restart() << std::endl;
        }
        
        renderTestFrame(img, labels, config);
        
        //show new canva
        label->setPixmap(QPixmap::fromImage(img));

        std::cout << "UPDATE! " << label->width() << "x" << label->height() << std::endl;
    };

    //run update every 1s
    QTimer t;
    t.setInterval(config.interval_ms);
    t.setSingleShot(false);
    QObject::connect(&t, &QTimer::timeout, updateCB);
    t.start();

    dlg.exec();
}

/**
 */
void LabelPlacementEngine::renderTestFrame(QImage& img, 
                                           const std::vector<TestLabel>& labels,
                                           const TestConfig& test_config) const
{
    QPainter p(&img);

    auto convert = [ & ] (double& x, double& y, bool scale_only) 
    {
        if (test_config.screen_transform)
            test_config.screen_transform(x, y, scale_only);
    };
    auto convertBBox = [ & ] (double& x, double& y, double w, double h) 
    {
        if (test_config.screen_transform_bbox)
            test_config.screen_transform_bbox(x, y, w, h);
    };

    if (test_config.avoid_roi && !test_config.roi.isEmpty())
    {
        double x0 = test_config.roi.left();
        double y0 = test_config.roi.top();
        double x1 = test_config.roi.right();
        double y1 = test_config.roi.bottom();

        convert(x0, y0, false);
        convert(x1, y1, false);

        double xmin = std::min(x0, x1);
        double ymin = std::min(y0, y1);
        double xmax = std::max(x0, x1);
        double ymax = std::max(y0, y1);

        QPen pen;
        pen.setColor(Qt::blue);
        pen.setStyle(Qt::PenStyle::DotLine);

        p.save();
        p.setPen(pen);
        p.drawRect(xmin, ymin, xmax - xmin, ymax - ymin);
        p.restore();
    }

    for (const auto& l : labels)
    {
        double x  = l.x;
        double y  = l.y;
        double xa = l.x_anchor;
        double ya = l.y_anchor;
        double xi = l.x_init.has_value() ? l.x_init.value() : 0.0;
        double yi = l.y_init.has_value() ? l.y_init.value() : 0.0;
        double w  = l.w;
        double h  = l.h;

        double rx = test_config.radius;
        double ry = test_config.radius;

        //convert from data space to screen space
        convertBBox(x, y, w, h);

        convert(xa, ya, false);
        convert(xi, yi, false);
        convert(rx, ry, true);

        //draw markers
        p.save();
        p.setBrush(QBrush(Qt::black, Qt::BrushStyle::SolidPattern));
        p.drawEllipse(QPointF(xa, ya), rx, ry);
        p.restore();

        //draw connection lines
        p.save();
        p.drawLine(QPointF(xa, ya), QPointF(x, y));
        p.restore();

        //draw labels
        p.save();
        p.drawRect(x, y, w, h);
        p.restore();

        //draw label paths
        if (l.x_init.has_value() && l.y_init.has_value())
        {
            p.save();
            p.setBrush(QBrush(Qt::red, Qt::BrushStyle::SolidPattern));
            p.drawEllipse(QPointF(xi, yi), 2, 2);
            p.restore();

            p.save();
            QPen pen;
            pen.setColor(Qt::red);
            pen.setStyle(Qt::PenStyle::DotLine);
            p.setPen(pen);
            p.drawLine(QPointF(xi, yi), QPointF(x, y));
            p.restore();
        }
    }
}

/**
 */
void LabelPlacementEngine::computeScreenTransform(const TestConfig& test_config, 
                                                  const std::vector<TestLabel>& test_labels) const                                
{
    test_config.screen_transform      = TestConfig::ScreenTransform();
    test_config.screen_transform_bbox = TestConfig::ScreenTransformBBox();

    int n = (int)test_labels.size();
    if (n < 2)
        return;

    //determine bounds
    double xmin = std::numeric_limits<double>::max();
    double ymin = std::numeric_limits<double>::max();
    double xmax = std::numeric_limits<double>::min();
    double ymax = std::numeric_limits<double>::min();

    auto checkPos = [ & ] (double x, double y) 
    {
        if (x < xmin)
            xmin = x;
        if (y < ymin)
            ymin = y;
        if (x > xmax)
            xmax = x;
        if (y > ymax)
            ymax = y;
    };
 
    double label_w_max = 0;
    double label_h_max = 0;
    for (int i = 0; i < n; ++i)
    {
        const auto& l = test_labels[ i ];

        checkPos(l.x, l.y);
        checkPos(l.x_anchor, l.y_anchor);

        if (l.x_init.has_value() && l.y_init.has_value())
            checkPos(*l.x_init, *l.y_init);

        if (l.w > label_w_max)
            label_w_max = l.w;
        if (l.h > label_h_max)
            label_h_max = l.h;
    }

    if (!test_config.roi.isEmpty())
    {
        xmin = test_config.roi.left();
        ymin = test_config.roi.top();
        xmax = test_config.roi.right();
        ymax = test_config.roi.bottom();
    }

    const double ws = test_config.width;
    const double hs = test_config.height;

    const double w = xmax - xmin;
    const double h = ymax - ymin;

    //fit data window into test window size
    double scale = 1.0;

    const double inner_aspect_ratio = w / h;
    const double outer_aspect_ratio = ws / hs;
    if (inner_aspect_ratio < outer_aspect_ratio) 
        scale = hs / h;
    else
        scale = ws / w;

    //center the fitted window
    double offsx = (ws - w * scale) * 0.5;
    double offsy = (hs - h * scale) * 0.5;

    const int h_win = test_config.height;

    bool flipy = test_config.flip_y;

    //store screen transform function to config
    auto screen_transform_func = [ = ] (double& x, double& y, bool scale_only) 
    {
        if (!scale_only)
        {
            x -= xmin;
            y -= ymin;
        }
        x *= scale;
        y *= scale;
        if (!scale_only)
        {
            x += offsx;
            y += offsy;

            if (flipy)
            {
                y = h_win - y;
            }
        }
    };
    auto screen_transform_bbox_func = [ = ] (double& x, double& y, double& w, double& h) 
    {
        x -= xmin;
        y -= ymin;   
        x *= scale;
        y *= scale;
        x += offsx;
        y += offsy;
        if (flipy)
        {
            y = h_win - y;
            y -= h;
        }
        w *= scale;
        h *= scale;
    };

    test_config.screen_transform      = screen_transform_func;
    test_config.screen_transform_bbox = screen_transform_bbox_func;


}
