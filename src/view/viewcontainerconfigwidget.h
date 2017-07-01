
#ifndef VIEWCONTAINERCONFIGWIDGET_H_
#define VIEWCONTAINERCONFIGWIDGET_H_

#include <QWidget>

class ViewContainer;
class View;
class QLabel;
class QVBoxLayout;

class ViewControlWidget : public QWidget
{
    Q_OBJECT
public:
    ViewControlWidget(View* view, QWidget *parent=nullptr);
    ~ViewControlWidget();

private slots:
    void loadingStartedSlot();
    void loadingFinishedSlot();
    void loadingTimeSlot( double s );
    void removeViewSlot();

signals:
    void viewDeleted();

private:
    View* view_;

    QLabel* load_;
    QString time_;
};

class ViewContainerConfigWidget : public QWidget
{
    Q_OBJECT
public:
    ViewContainerConfigWidget (ViewContainer* view_container, QWidget *parent=nullptr);
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
    ViewContainer* view_container_;
    std::vector<ViewControlWidget*> view_widgets_;
    QVBoxLayout* layout_;

    QString name_;
};

#endif // VIEWCONTAINERCONFIGWIDGET_H_
