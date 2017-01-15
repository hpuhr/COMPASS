#ifndef MYSQLPPCONNECTIONWIDGET_H
#define MYSQLPPCONNECTIONWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <cassert>

class MySQLppConnection;
class QTextEdit;
class QLineEdit;
class QPushButton;

class DatabaseNameComboBox : public QComboBox
{
    Q_OBJECT
public:
    /// @brief Constructor.
    DatabaseNameComboBox (QWidget *parent = 0)
    : QComboBox (parent)
    {
    }

    /// @brief Destructor.
    virtual ~DatabaseNameComboBox () { }

    void loadDatabaseNames ()
    {
//        std::vector<std::string> names = ATSDB::getInstance().getDatabaseNames();
//        std::vector<std::string>::iterator it;

//        for (it = names.begin(); it != names.end(); it++)
//        {
//            addItem((*it).c_str());
//        }
    }

    /// @brief Returns the currently selected data source
    std::string getDatabaseName ()
    {
        return currentText().toStdString();
    }

    bool hasDatabaseName (std::string name)
    {
        int index = findText(name.c_str());
        return index >= 0;
    }

    /// @brief Sets the current data source
    void setDatabaseName (std::string name)
    {
        int index = findText(name.c_str());
        assert (index >= 0);
        setCurrentIndex(index);
    }
};

class MySQLppConnectionWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void updateMySQLConnectInfo ();
    void updateMySQLDatabaseInfo ();

public:
    explicit MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent = 0);

    /// @brief Sets the database system
    void setDBType (std::string value);
    /// @brief Sets the filename for SQLite3 databases
    //void setDBFilename (std::string value);
    /// @brief Sets the server address for MySQL databases
    void setDBServer (std::string value);
    /// @brief Sets the database name for MySQL databases
    void setDBName (std::string value);
    /// @brief Sets the port for MySQL databases
    void setDBPort (std::string value);
    /// @brief Sets the database user for MySQL databases
    void setDBUser (std::string value);
    /// @brief Sets the database user password for MySQL databases
    void setDBPassword (std::string value);
    /// @brief Sets no password for MySQL databases
    void setDBNoPassword ();

    void connectDB ();
    void openDB ();

    /// @brief Returns if a database system was set
    bool hasDefinedDatabase ();

protected:
    MySQLppConnection &connection_;

    /// MySQL ip address edit field
    QLineEdit *mysql_db_ip_edit_;
    /// MySQL port edit field
    QLineEdit *mysql_db_port_edit_;
    /// MySQL username edit field
    QLineEdit *mysql_db_username_edit_;
    /// MySQL password edit field
    QLineEdit *mysql_db_password_edit_;

    /// Open connection button
    QPushButton *connect_button_;

    /// MySQL database name edit field
    //QLineEdit *mysql_db_name_edit_;
    DatabaseNameComboBox *mysql_db_name_box_;

    QPushButton *open_button_;

    /// MySQL database name
    std::string mysql_db_name_;
    /// MySQL ip address
    std::string mysql_db_ip_;
    /// MySQL port
    std::string mysql_db_port_;
    /// MySQL username
    std::string mysql_db_username_;
    /// MySQL password
    std::string mysql_db_password_;

    /// @brief Creates GUI elements
    void createElements ();

};

#endif // MYSQLPPCONNECTIONWIDGET_H
