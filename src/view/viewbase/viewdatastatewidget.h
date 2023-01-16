
#include "viewloadstate.h"

#include <QWidget>

class QPushButton;
class QLabel;

/**
 */
class ViewDataStateWidget : public QWidget
{
public:
    ViewDataStateWidget(QWidget* parent = nullptr);
    virtual ~ViewDataStateWidget();

    void setState(const view::LoadState& state);

protected:
    virtual void reloadRequested();

private:
    void setStatus(const QString& text, bool visible, const QColor& color = Qt::black);

    QPushButton* reload_button_ = nullptr;
    QLabel*      status_label_  = nullptr;
};
