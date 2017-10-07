/*
 * OSGViewConfigWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEWCONFIGWIDGET_H_
#define OSGVIEWCONFIGWIDGET_H_

#include <QWidget>
#include "dbovariable.h"

class OSGView;
class QLineEdit;
class QSlider;
class QRadioButton;
class QCheckBox;

/**
 * @brief Widget with configuration elements for a OSGView
 *
 */
class OSGViewConfigWidget : public QWidget
{
    Q_OBJECT

public slots:
    void mapSelectedSlot (bool selected);
    void mapOpacityChangedSlot ();
    void dataOpacityChangedSlot ();

    void useHeightSlot (bool checked);
    void useHeightScaleSlot (bool checked);
    void heightScaleFactorChangedSlot ();
    void heightClampChangedSlot(bool checked);

public:
    /// @brief Constructor
    OSGViewConfigWidget (OSGView* view, QWidget* parent=nullptr);
    /// @brief Destructor
    virtual ~OSGViewConfigWidget();

protected:
    /// Base view
    OSGView* view_;

    QSlider* map_opacity_slider_ {nullptr};
    QSlider* data_opacity_slider_ {nullptr};

    QCheckBox* use_height_check_ {nullptr};
    QCheckBox* height_scale_check_ {nullptr};
    QSlider* height_scale_slider_ {nullptr};
    QCheckBox* height_clamp_check_ {nullptr};

    std::map <QRadioButton*, std::string> map_names_;
};

#endif /* OSGVIEWCONFIGWIDGET_H_ */
