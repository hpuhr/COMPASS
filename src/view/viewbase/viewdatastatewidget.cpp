
#include "viewdatastatewidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"

/**
 */
ViewDataStateWidget::ViewDataStateWidget(QWidget* parent)
:   QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    reload_button_ = new QPushButton("Reload");

    QFont font_status;
    font_status.setItalic(true);

    status_label_  = new QLabel;
    status_label_->setFont(font_status);

    layout->addWidget(status_label_);
    layout->addWidget(reload_button_);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    setState(view::LoadState());

    connect(reload_button_, &QPushButton::pressed, this, &ViewDataStateWidget::reloadRequested);
}

/**
 */
ViewDataStateWidget::~ViewDataStateWidget() = default;

/**
 */
void ViewDataStateWidget::setStatus(const QString& text, 
                                    bool visible, 
                                    const QColor& color)
{
    status_label_->setText(text);

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);
    status_label_->setPalette(palette);

    status_label_->setVisible(visible);
}

/**
 */
void ViewDataStateWidget::setState(const view::LoadState& state)
{
    QString msg   = state.message();
    QColor  color = state.color();
    bool    load  = state.isLoading();

    setStatus(msg, !msg.isEmpty(), color);

    reload_button_->setEnabled(!load);
}

/**
 */
void ViewDataStateWidget::reloadRequested()
{
    COMPASS::instance().dbContentManager().load();
}
