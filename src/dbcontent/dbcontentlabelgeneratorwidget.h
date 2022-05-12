#ifndef DBCONTENTLABELGENERATORWIDGET_H
#define DBCONTENTLABELGENERATORWIDGET_H

#include <QWidget>

class DBContentLabelGenerator;

class QCheckBox;
class QLineEdit;

class DBContentLabelGeneratorWidget : public QWidget
{
    Q_OBJECT

public slots:
    void editSettingsSlot();

    void autoLabelChangedSlot(bool checked);
    void lodChangedSlot(const QString& text);

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

public:
    DBContentLabelGeneratorWidget(DBContentLabelGenerator& label_generator);
    virtual ~DBContentLabelGeneratorWidget();

protected:
    DBContentLabelGenerator& label_generator_;
};

#endif // DBCONTENTLABELGENERATORWIDGET_H
