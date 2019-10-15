#ifndef SQLiteCONNECTIONINFOWIDGET_H
#define SQLiteCONNECTIONINFOWIDGET_H

#include <QWidget>

class SQLiteConnection;
class QLabel;

class SQLiteConnectionInfoWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateSlot ();

public:
    explicit SQLiteConnectionInfoWidget(SQLiteConnection& connection, QWidget* parent=nullptr);

protected:
    SQLiteConnection& connection_;

    QLabel* database_label_ {nullptr};
    QLabel* status_label_ {nullptr};
};

#endif // SQLiteCONNECTIONINFOWIDGET_H
