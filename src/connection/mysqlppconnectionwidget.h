#ifndef MYSQLPPCONNECTIONWIDGET_H
#define MYSQLPPCONNECTIONWIDGET_H

#include <QWidget>

class MySQLppConnection;
class QComboBox;
class QVBoxLayout;
class QPushButton;
class QStackedWidget;

class MySQLppConnectionWidget : public QWidget
{
    Q_OBJECT

signals:
    void databaseOpenedSignal ();

public slots:
    void addServerSlot ();
    void deleteServerSlot ();
    void serverSelectedSlot (const QString &value);
    void serverConnectedSlot ();
    void databaseOpenedSlot ();

public:
    explicit MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent = 0);

protected:
    MySQLppConnection &connection_;

    QComboBox *server_select_;
    QPushButton *add_button_;
    QPushButton *delete_button_;

    QStackedWidget *server_widgets_;

    void updateServers();
};

#endif // MYSQLPPCONNECTIONWIDGET_H
