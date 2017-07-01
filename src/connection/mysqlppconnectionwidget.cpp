#include "mysqlppconnectionwidget.h"
#include "mysqlserver.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QStackedWidget>

MySQLppConnectionWidget::MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *servers_label = new QLabel ("Server Selection");
    servers_label->setFont(font_bold);
    layout->addWidget(servers_label);

    server_select_ = new QComboBox ();
    connect (server_select_, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(serverSelectedSlot(const QString &)));
    layout->addWidget (server_select_);
    layout->addStretch();

    QHBoxLayout *button_layout = new QHBoxLayout ();

    add_button_ = new QPushButton ("Add");
    connect (add_button_, SIGNAL(clicked()), this, SLOT(addServerSlot()));
    button_layout->addWidget(add_button_);

    delete_button_ = new QPushButton ("Delete");
    connect (delete_button_, SIGNAL(clicked()), this, SLOT(deleteServerSlot()));
    button_layout->addWidget(delete_button_);
    layout->addLayout(button_layout);
    layout->addStretch();

    server_widgets_ = new QStackedWidget ();
    layout->addWidget(server_widgets_);
    layout->addStretch();

    updateServers ();

    setLayout (layout);
}

void MySQLppConnectionWidget::addServerSlot ()
{
    logdbg << "MySQLppConnectionWidget: addServerSlot";

    bool ok;
    QString text = QInputDialog::getText(this, tr("Server Name"),
                                         tr("Specify a (unique) server name:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty())
    {
        connection_.addServer(text.toStdString());
        updateServers ();
    }
}

void MySQLppConnectionWidget::deleteServerSlot ()
{
    logdbg << "MySQLppConnectionWidget: deleteServerSlot";

    connection_.deleteUsedServer();

    updateServers ();
}

void MySQLppConnectionWidget::serverSelectedSlot (const QString &value)
{
    logdbg << "MySQLppConnectionWidget: serverSelectedSlot: '" << value.toStdString() << "'";

    assert (server_widgets_);
    while (server_widgets_->count() > 0)
        server_widgets_->removeWidget(server_widgets_->widget(0));

    if (value.size() > 0)
    {
        connection_.setServer (value.toStdString());

        QWidget *widget = connection_.usedServer().widget();
        QObject::connect(widget, SIGNAL(serverConnectedSignal()), this, SLOT(serverConnectedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
        QObject::connect(widget, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));

        server_widgets_->addWidget(widget);
        delete_button_->setDisabled(false);
    }
    else
        delete_button_->setDisabled(true);
}

void MySQLppConnectionWidget::serverConnectedSlot ()
{
    logdbg << "MySQLppConnectionWidget: serverConnectedSlot";
    server_select_->setDisabled(true);
    add_button_->setDisabled(true);
    delete_button_->setDisabled(true);
}

void MySQLppConnectionWidget::databaseOpenedSlot()
{
    logdbg << "MySQLppConnectionWidget: databaseOpenedSlot";
    emit databaseOpenedSignal ();
}

void MySQLppConnectionWidget::updateServers()
{
    logdbg << "MySQLppConnectionWidget: updateServers";
    const std::map <std::string, MySQLServer*> &servers = connection_.servers();
    std::string used_server = connection_.usedServerString();

    server_select_->clear();

    for (auto it : servers)
    {
        server_select_->addItem(it.first.c_str());
    }

    int index = server_select_->findText(used_server.c_str());
    if (index != -1) // -1 for not found
    {
       server_select_->setCurrentIndex(index);

    }
}
