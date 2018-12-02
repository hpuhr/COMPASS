#ifndef JSONIMPORTERTASKWIDGET_H
#define JSONIMPORTERTASKWIDGET_H

#include <QWidget>

class JSONImporterTask;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidget;

class QPushButton;
class QListWidget;
class QCheckBox;
class QLineEdit;

class JSONImporterTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void testImportSlot ();
    void importSlot ();
    void importDoneSlot (bool test);

    void addFileSlot ();
    void deleteFileSlot ();
    void updateFileListSlot ();

public:
    JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~JSONImporterTaskWidget();

    void update ();

protected:
    JSONImporterTask& task_;

    QListWidget* file_list_ {nullptr};
    QPushButton* add_button_ {nullptr};
    QPushButton* delete_button_ {nullptr};

    QListWidget* object_parser_list_ {nullptr};

    QPushButton* test_button_ {nullptr};
    QPushButton* import_button_ {nullptr};

    void updateParserList ();
};

#endif // JSONIMPORTERTASKWIDGET_H
