/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "viewcontainerconfigwidget.h"

#include <QLabel>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "files.h"
#include "global.h"
#include "view.h"
#include "viewcontainer.h"
#include "viewcontainerwidget.h"

using namespace Utils;

ViewContainerConfigWidget::ViewContainerConfigWidget(ViewContainer* view_container, QWidget* parent)
    : QWidget(parent), view_container_(view_container)
{
    //setAutoFillBackground(true);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* layout = new QVBoxLayout();
    // layout->setMargin( 3 );
    setLayout(layout);

    QFrame* frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(FRAME_SIZE);
    layout->addWidget(frame);

    QVBoxLayout* blayout = new QVBoxLayout();
    frame->setLayout(blayout);
    //    blayout->setMargin( 3 );
    //    blayout->setSpacing( 3 );

    QHBoxLayout* namelayout = new QHBoxLayout();
    namelayout->setMargin(0);
    blayout->addLayout(namelayout);

    name_ = QString(view_container_->getWindowName()
                        .c_str());  //"Window " + QString::number( view_container_->getKey() );
    QLabel* head = new QLabel(name_);
    head->setFont(font_bold);
    namelayout->addWidget(head);

    if (view_container_->getWindowName().compare("MainWindow") != 0)
    {
        QToolButton* vdel = new QToolButton();
        vdel->setIcon(QIcon(Files::getIconFilepath("delete.png").c_str()));
        namelayout->addWidget(vdel);
        connect(vdel, SIGNAL(clicked()), this, SLOT(closeSlot()));
    }

    layout_ = new QVBoxLayout();
    blayout->addLayout(layout_);

    updateSlot();
}

ViewContainerConfigWidget::~ViewContainerConfigWidget() {}

void ViewContainerConfigWidget::addView(const std::string& class_name)
{
    view_container_->addView(class_name);
    updateSlot();
}

// void ViewContainerConfigWidget::addTemplateView (std::string template_name)
//{
//    if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//        return;

//    view_container_->addTemplateView(template_name);
//    updateSlot();

//}

void ViewContainerConfigWidget::closeSlot()
{
    // view_container_->close();
    // TODO why r u not delete view_container?
}

/*
 */
void ViewContainerConfigWidget::updateSlot()
{
    int i, n = view_widgets_.size();
    for (i = 0; i < n; ++i)
        delete view_widgets_[i];
    view_widgets_.clear();

    const std::vector<View*>& views = view_container_->getViews();
    n = views.size();
    for (i = 0; i < n; ++i)
    {
        ViewControlWidget* w = new ViewControlWidget(views[i], this);

        connect(w, SIGNAL(viewDeleted()), this, SLOT(updateSlot()));

        layout_->addWidget(w);
        view_widgets_.push_back(w);
    }
}

/****************************************************************************
ViewControlWidget
*****************************************************************************/

/*
 */
ViewControlWidget::ViewControlWidget(View* view, QWidget* parent) : QWidget(parent), view_(view)
{
    QFont font_underline;
    font_underline.setItalic(true);

    QVBoxLayout* vlayout = new QVBoxLayout();
    setLayout(vlayout);

    vlayout->setMargin(0);
    vlayout->setSpacing(1);

    QFrame* frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    frame->setLineWidth(1);

    vlayout->addWidget(frame);
    QVBoxLayout* flayout = new QVBoxLayout();
    frame->setLayout(flayout);
    flayout->setMargin(1);
    flayout->setSpacing(1);

    QHBoxLayout* namelayout = new QHBoxLayout;
    namelayout->setMargin(0);
    flayout->addLayout(namelayout);

    QString text = QString::fromStdString(view_->getName());
    QLabel* label = new QLabel(text);
    label->setFont(font_underline);
    namelayout->addWidget(label);

    QToolButton* vdel = new QToolButton();
    vdel->setIcon(QIcon(Files::getIconFilepath("delete.png").c_str()));
    connect(vdel, SIGNAL(clicked()), this, SLOT(removeViewSlot()));
    namelayout->addWidget(vdel);
    // namelayout->addStretch( 1 );

    load_ = new QLabel();
    load_->setText("Idle");
    flayout->addWidget(load_);
    connect(view, SIGNAL(loadingStarted()), this, SLOT(loadingStartedSlot()));
    connect(view, SIGNAL(loadingFinished()), this, SLOT(loadingFinishedSlot()));
    connect(view, SIGNAL(loadingTime(double)), this, SLOT(loadingTimeSlot(double)));
}

/*
 */
ViewControlWidget::~ViewControlWidget() {}

/*
 */
void ViewControlWidget::loadingStartedSlot()
{
    load_->setText("Loading In Progress (" + time_ + ")");
}

/*
 */
void ViewControlWidget::loadingFinishedSlot() { load_->setText("Loading Done (" + time_ + ")"); }

/*
 */
void ViewControlWidget::loadingTimeSlot(double s)
{
    time_ = QString::number(s) + " s";
    QString txt = "Loading Done ";
    if (s > 0.0)
        txt = "Loading In Progress (";
    txt += time_ + ")";
    load_->setText(txt);
}

/*
 */
void ViewControlWidget::removeViewSlot()
{
    delete view_;
    emit viewDeleted();
}
