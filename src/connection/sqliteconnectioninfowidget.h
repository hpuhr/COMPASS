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
    explicit SQLiteConnectionInfoWidget(SQLiteConnection &connection, QWidget *parent = 0);

protected:
    SQLiteConnection &connection_;

    //QLabel *server_;
    QLabel *database_;
    QLabel *status_;
};

#endif // SQLiteCONNECTIONINFOWIDGET_H
