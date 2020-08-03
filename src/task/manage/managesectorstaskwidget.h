#ifndef MANAGESECTORSTASKWIDGET_H
#define MANAGESECTORSTASKWIDGET_H

#include <taskwidget.h>

class ManageSectorsTask;

class QHBoxLayout;
class QPushButton;
class QListWidget;
class QComboBox;
class QStackedWidget;
class QCheckBox;
class QTabWidget;

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

public:
    ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent = nullptr);

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

protected:
    ManageSectorsTask& task_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    void addMainTab();
};

#endif // MANAGESECTORSTASKWIDGET_H
