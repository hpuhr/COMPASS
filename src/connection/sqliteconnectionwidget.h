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
    void databaseOpenedSignal ();

public slots:
    void addFileSlot ();
    void deleteFileSlot ();
    void openFileSlot ();

    void updateFileListSlot ();

public:
    explicit SQLiteConnectionWidget(SQLiteConnection &connection, QWidget *parent = 0);

protected:
    SQLiteConnection &connection_;

    QListWidget *file_list_;

    QPushButton *add_button_;
    QPushButton *delete_button_;

    QPushButton *open_button_;
};

#endif // SQLiteCONNECTIONWIDGET_H
