#ifndef DBCONTENTLABELGENERATORWIDGET_H
#define DBCONTENTLABELGENERATORWIDGET_H

#include <QWidget>

class QCheckBox;
class QLineEdit;

namespace dbContent
{

class LabelGenerator;

// is not owned by LabelGenerator, can be deleted
// TODO multiple instances do not update each other
class LabelGeneratorWidget : public QWidget
{
    Q_OBJECT

public slots:
    void editSettingsSlot();
    void editDBContentSlot();

    void autoLabelChangedSlot(bool checked);
    void lodChangedSlot(const QString& text);

    void declutterLabelsChangedSlot(bool checked);

    void filterMode3AActiveChangedSlot(bool checked);
    void filterMode3AChangedSlot(const QString& text);

    void filterModeCMinActiveChangedSlot(bool checked);
    void filterModeCMinChangedSlot(const QString& text);
    void filterModeCMaxActiveChangedSlot(bool checked);
    void filterModeCMaxChangedSlot(const QString& text);
    void filterModeCNullWantedChangedSlot(bool checked);

    void filterTIActiveChangedSlot(bool checked);
    void filterTIChangedSlot(const QString& text);

    void filterTAActiveChangedSlot(bool checked);
    void filterTAChangedSlot(const QString& text);

    void filterPSROnlyActiveChangedSlot(bool checked);

public:
    LabelGeneratorWidget(LabelGenerator& label_generator);
    virtual ~LabelGeneratorWidget();

protected:
    LabelGenerator& label_generator_;
};

}

#endif // DBCONTENTLABELGENERATORWIDGET_H
