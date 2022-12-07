
#pragma once

#include <QWidget>

class ViewConfigWidget : public QWidget
{
public:
    ViewConfigWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0) : QWidget(parent, f) {}
    virtual ~ViewConfigWidget() = default;

    virtual void setStatus(const QString& text, bool visible, const QColor& color = Qt::black) = 0;

    void onDisplayChange()
    {
        onDisplayChange_impl();
    }

protected:
    virtual void onDisplayChange_impl() {}
};
