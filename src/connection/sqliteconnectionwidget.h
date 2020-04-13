#ifndef SQLiteCONNECTIONWIDGET_H
#define SQLiteCONNECTIONWIDGET_H

#include <QWidget>

class SQLiteConnection;
class QComboBox;
class QVBoxLayout;
class QPushButton;
class QListWidget;

class SQLiteConnectionWidget : public QWidget
{
    Q_OBJECT

  signals:
    void databaseOpenedSignal();

  public slots:
    void newFileSlot();
    void addFileSlot();
    void deleteFileSlot();
    void openFileSlot();

    void updateFileListSlot();

  public:
    explicit SQLiteConnectionWidget(SQLiteConnection& connection, QWidget* parent = 0);
    virtual ~SQLiteConnectionWidget();

    void addFile(const std::string& filename);

  protected:
    SQLiteConnection& connection_;

    QListWidget* file_list_{nullptr};

    QPushButton* new_button_{nullptr};
    QPushButton* add_button_{nullptr};
    QPushButton* delete_button_{nullptr};

    QPushButton* open_button_{nullptr};
};

#endif  // SQLiteCONNECTIONWIDGET_H
