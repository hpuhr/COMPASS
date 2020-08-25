#ifndef MANAGESECTORSTASKWIDGET_H
#define MANAGESECTORSTASKWIDGET_H

#include <taskwidget.h>

class ManageSectorsTask;

class QVBoxLayout;
class QPushButton;
class QListWidget;
class QTableWidget;
class QCheckBox;
class QTabWidget;
class QTextEdit;
class QTableWidgetItem;

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

  void sectorItemChangedSlot(QTableWidgetItem* item);

  void changeSectorColorSlot();
  void deleteSectorSlot();

  void exportSectorsSlot ();
  void clearSectorsSlot ();
  void importSectorsSlot ();

public:
    ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent = nullptr);

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

    void updateParseMessage ();

    void importSectorsJSON (const std::string& filename);

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

    QTableWidget* sector_table_{nullptr};
    QStringList table_columns_{"ID", "Sector Name",  "Layer Name", "Num Points", "Altitude Minimum",
                               "Altitude Maximum", "Color", "Delete"};

    void addImportTab();
    void addManageTab();

    void updateSectorTable();
};

#endif // MANAGESECTORSTASKWIDGET_H
