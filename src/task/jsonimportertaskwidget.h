#ifndef JSONIMPORTERTASKWIDGET_H
#define JSONIMPORTERTASKWIDGET_H

#include <QWidget>

class JSONImporterTask;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidget;

class QPushButton;

class JSONImporterTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void dbObjectChangedSlot();
    void testImportSlot ();
    void importSlot ();
    void testImportDoneSlot ();
    void importDoneSlot ();

public:
    JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~JSONImporterTaskWidget();

    void update ();

    void testImport();
    void import();

protected:
    JSONImporterTask& task_;

    DBObjectComboBox* object_box_ {nullptr};

    QPushButton* test_button_ {nullptr};
    QPushButton* import_button_ {nullptr};

    void setDBOBject (const std::string& object_name);
};

#endif // JSONIMPORTERTASKWIDGET_H
