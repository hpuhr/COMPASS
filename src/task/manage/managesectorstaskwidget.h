#ifndef MANAGESECTORSTASKWIDGET_H
#define MANAGESECTORSTASKWIDGET_H

#include <taskwidget.h>

class ManageSectorsTask;

class QVBoxLayout;
class QPushButton;
class QListWidget;
//class QComboBox;
//class QStackedWidget;
class QCheckBox;
class QTabWidget;
class QTextEdit;

class ManageSectorsTaskWidget : public TaskWidget
{
    Q_OBJECT
public slots:
  void expertModeChangedSlot();

  void addFileSlot();
  void deleteFileSlot();
  void deleteAllFilesSlot();
  void selectedFileSlot();
  void updateFileListSlot();

  void importSlot();

public:
    ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent = nullptr);

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

    void updateParseMessage ();

protected:
    ManageSectorsTask& task_;

    QVBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    QTextEdit* parse_msg_edit_ {nullptr};

    QPushButton* import_button_ {nullptr};

    void addImportTab();
};

#endif // MANAGESECTORSTASKWIDGET_H
