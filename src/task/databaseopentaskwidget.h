#ifndef DATABASEOPENTASKWIDGET_H
#define DATABASEOPENTASKWIDGET_H

#include <QWidget>
#include <QComboBox>

class DBInterface;
class QStackedWidget;

class DatabaseOpenTask;

class DatabaseOpenTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void databaseTypeSelectSlot ();
    void databaseOpenedSlot ();

signals:
    void databaseOpenedSignal ();

public:
    DatabaseOpenTaskWidget(DatabaseOpenTask& task, DBInterface& db_interface, QWidget *parent=nullptr);
    virtual ~DatabaseOpenTaskWidget();

protected:
    DatabaseOpenTask& task_;

    DBInterface& db_interface_;

    QStackedWidget* connection_stack_ {nullptr};

    void useConnection (const std::string& connection_type);

    //QWidget* dbinterface_widget_ {nullptr};
};

#endif // DATABASEOPENTASKWIDGET_H
