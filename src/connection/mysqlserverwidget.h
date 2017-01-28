#ifndef MYSQLSERVERWIDGET_H
#define MYSQLSERVERWIDGET_H

#include <qwidget.h>

class MySQLServer;
class MySQLppConnection;
class QLineEdit;
class QComboBox;
class QPushButton;

class MySQLServerWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateHostSlot (const QString &value);
    void updateUserSlot (const QString &value);
    void updatePasswordSlot (const QString &value);
    void updatePortSlot (const QString &value);
    void connectSlot ();

    void updateDatabaseSlot (const QString &value);
    void openDatabaseSlot ();

signals:
    void serverConnectedSignal ();
    void databaseOpenedSignal ();

public:
    explicit MySQLServerWidget(MySQLppConnection &connection, MySQLServer &server, QWidget *parent = 0);
    virtual ~MySQLServerWidget ();

protected:
    MySQLppConnection &connection_;
    MySQLServer &server_;

    /// MySQL ip address edit field
    QLineEdit *host_edit_;
    /// MySQL username edit field
    QLineEdit *user_edit_;
    /// MySQL password edit field
    QLineEdit *password_edit_;
    /// MySQL port edit field
    QLineEdit *port_edit_;

    /// Open connection button
    QPushButton *connect_button_;

    /// MySQL database name edit field
    //QLineEdit *mysql_db_name_edit_;
    QComboBox *db_name_box_;

    QPushButton *open_button_;

    void updateDatabases ();
};


#endif // MYSQLSERVERWIDGET_H

