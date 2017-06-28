
#ifndef VIEWCONTAINERCONFIGWIDGET_H_
#define VIEWCONTAINERCONFIGWIDGET_H_

#include <QWidget>

class ViewContainerWidget;
class View;
class QLabel;
class QVBoxLayout;

class ViewControlWidget : public QWidget
{
    Q_OBJECT
public:
    ViewControlWidget( View* view, QWidget* parent=NULL );
    ~ViewControlWidget();

private slots:
    void loadingStartedSlot();
    void loadingFinishedSlot();
    void loadingTimeSlot( double s );
    void removeViewSlot();

signals:
    void viewDeleted();

private:
    void createWidget();

    View* view_;

    QLabel* load_;
    QString time_;
};

class ViewContainerConfigWidget : public QWidget
{
    Q_OBJECT
public:
    ViewContainerConfigWidget( ViewContainerWidget* view_container, QWidget* parent=NULL );
    virtual ~ViewContainerConfigWidget();

    const QString& name() { return name_; }

    void addGeographicView();
    void addScatterPlotView();
    void addHistogramView();
    void addListBoxView();
    void addMosaicView();
    void addTemplateView (std::string template_name);

private slots:
    void updateSlot();
    void closeSlot();

private:
    void createWidget();

    ViewContainerWidget* view_container_;
    std::vector<ViewControlWidget*> view_widgets_;
    QVBoxLayout* layout_;

    QString name_;
};

#endif // VIEWCONTAINERCONFIGWIDGET_H_
