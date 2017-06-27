#ifndef MYSQLPPCONNECTIONINFOWIDGET_H
#define MYSQLPPCONNECTIONINFOWIDGET_H

#include <QWidget>

class MySQLppConnection;
class QLabel;

class MySQLppConnectionInfoWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateSlot ();

public:
    explicit MySQLppConnectionInfoWidget(MySQLppConnection &connection, QWidget *parent = 0);

protected:
    MySQLppConnection &connection_;

    QLabel *server_;
    QLabel *database_;
    QLabel *status_;
};

#endif // MYSQLPPCONNECTIONINFOWIDGET_H
