#ifndef MYSQLSERVERWIDGET_H
#define MYSQLSERVERWIDGET_H

#include <qwidget.h>

class MySQLServer;
class MySQLppConnection;
class QLineEdit;
class QComboBox;
class QPushButton;

//class DatabaseNameComboBox : public QComboBox
//{
//    Q_OBJECT
//public:
//    /// @brief Constructor.
//    DatabaseNameComboBox (QWidget *parent = 0)
//    : QComboBox (parent)
//    {
//    }

//    /// @brief Destructor.
//    virtual ~DatabaseNameComboBox () { }

//    void loadDatabaseNames ()
//    {
////        std::vector<std::string> names = ATSDB::getInstance().getDatabaseNames();
////        std::vector<std::string>::iterator it;

////        for (it = names.begin(); it != names.end(); it++)
////        {
////            addItem((*it).c_str());
////        }
//    }

//    /// @brief Returns the currently selected data source
//    std::string getDatabaseName ()
//    {
//        return currentText().toStdString();
//    }

//    bool hasDatabaseName (std::string name)
//    {
//        int index = findText(name.c_str());
//        return index >= 0;
//    }

//    /// @brief Sets the current data source
//    void setDatabaseName (std::string name)
//    {
//        int index = findText(name.c_str());
//        assert (index >= 0);
//        setCurrentIndex(index);
//    }
//};

class MySQLServerWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateHostSlot (const QString &value);
    void updateUserSlot (const QString &value);
    void updatePasswortSlot (const QString &value);
    void updatePortSlot (const QString &value);
    void connectSlot ();

    void updateDatabaseSlot (const QString &value);
    void openDatabaseSlot ();

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
};


#endif // MYSQLSERVERWIDGET_H

