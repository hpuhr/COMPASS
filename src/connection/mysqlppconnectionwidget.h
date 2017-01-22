#ifndef MYSQLPPCONNECTIONWIDGET_H
#define MYSQLPPCONNECTIONWIDGET_H

#include <QWidget>

class MySQLppConnection;
class QComboBox;
class QVBoxLayout;
class QPushButton;

class MySQLppConnectionWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void addServer ();
    void deleteServer ();
    void serverSelectedSlot (const QString &value);

public:
    explicit MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent = 0);

protected:
    MySQLppConnection &connection_;

    QComboBox *server_select_;
    QPushButton *delete_button_;

    QVBoxLayout *server_widget_layout_;

    void updateServers();
};

#endif // MYSQLPPCONNECTIONWIDGET_H
