#ifndef FILTERMANAGERWIDGET_H
#define FILTERMANAGERWIDGET_H

#include <QFrame>

class FilterManager;
class FilterGeneratorWidget;

class QPushButton;
class QVBoxLayout;

class FilterManagerWidget : public QFrame
{
    Q_OBJECT

signals:

public slots:
    void addFilterSlot ();
    void updateFiltersSlot();
    void filterWidgetActionSlot (bool result);

    void databaseOpenedSlot ();
public:
    explicit FilterManagerWidget(FilterManager &manager, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~FilterManagerWidget();

protected:
    FilterManager &filter_manager_;

    FilterGeneratorWidget *filter_generator_widget_;

    QVBoxLayout *filter_layout_;

    QPushButton *add_button_;
};

#endif // FILTERMANAGERWIDGET_H
