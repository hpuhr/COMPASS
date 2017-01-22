#include "mysqlppconnectionwidget.h"
#include "mysqlserver.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>

MySQLppConnectionWidget::MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection)
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *servers_label = new QLabel ("Servers");
    servers_label->setFont(font_big);
    layout->addWidget(servers_label);

    server_select_ = new QComboBox ();
    updateServers ();
    connect (server_select_, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(serverSelectedSlot(const QString &)));
    layout->addWidget (server_select_);
    layout->addSpacing(10);

    QHBoxLayout *button_layout = new QHBoxLayout ();

    QPushButton *add_button = new QPushButton ("Add");
    connect (add_button, SIGNAL(clicked()), this, SLOT(addServer()));
    button_layout->addWidget(add_button);

    QPushButton *delete_button = new QPushButton ("Delete");
    connect (delete_button, SIGNAL(clicked()), this, SLOT(deleteServer()));
    button_layout->addWidget(delete_button);
    layout->addLayout(button_layout);

    server_widget_layout_ = new QVBoxLayout ();
    layout->addLayout(server_widget_layout_);

    setLayout (layout);
}

void MySQLppConnectionWidget::addServer ()
{
    logdbg << "MySQLppConnectionWidget: addServer";

    bool ok;
    QString text = QInputDialog::getText(this, tr("Server Name"),
                                         tr("Specify a (unique) server name:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty())
    {
        connection_.addServer(text.toStdString());
        updateServers ();
    }
    logdbg << "MySQLppConnectionWidget: addServer: done";
}

void MySQLppConnectionWidget::deleteServer ()
{
    logdbg << "MySQLppConnectionWidget: deleteServer";
}

void MySQLppConnectionWidget::serverSelectedSlot (const QString &value)
{
    logdbg << "MySQLppConnectionWidget: serverSelectedSlot: '" << value.toStdString() << "'";
//    ATSDB::getInstance().connect(getConnectionInfo());

//    assert (mysql_db_name_box_);
//    mysql_db_name_box_->loadDatabaseNames();

//    if (mysql_db_name_box_->hasDatabaseName(mysql_db_name_))
//        mysql_db_name_box_->setDatabaseName (mysql_db_name_);
//    else
//        mysql_db_name_=mysql_db_name_box_->getDatabaseName();
//    connect(mysql_db_name_box_, SIGNAL( currentIndexChanged (const QString &) ), this, SLOT( updateMySQLDatabaseInfo() ));

//    connect_button_->setDisabled(true);
//    open_button_->setDisabled(false);
}

void MySQLppConnectionWidget::updateServers()
{
    logdbg << "MySQLppConnectionWidget: updateServers";
//    const std::map <std::string, MySQLServer> &servers = connection_.servers();

//    server_select_->
//    server_select_->clear();

//    for (auto it : servers)
//    {
//        server_select_->addItem(it.first.c_str());
//    }
    logdbg << "MySQLppConnectionWidget: updateServers: done";
}
