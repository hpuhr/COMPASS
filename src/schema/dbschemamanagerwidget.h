#ifndef DBSCHEMAMANAGERWIDGET_H
#define DBSCHEMAMANAGERWIDGET_H

#include <QFrame>

class DBSchemaManager;

class QLineEdit;
class QComboBox;
class QPushButton;
class QStackedWidget;

class DBSchemaManagerWidget : public QFrame
{
    Q_OBJECT

public:
    explicit DBSchemaManagerWidget(DBSchemaManager &manager, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~DBSchemaManagerWidget();

signals:

public slots:
    void databaseOpenedSlot ();

    void addSchemaSlot ();
    void deleteSchemaSlot ();
    /// @brief Sets the schema
    void schemaSelectedSlot (const QString &value);

protected:
    DBSchemaManager &manager_;

    /// Current schema selection field
    QComboBox *schema_select_;

    QPushButton *add_button_;
    QPushButton *delete_button_;

    QStackedWidget *schema_widgets_;

    void updateSchemas();

};

#endif // DBSCHEMAMANAGERWIDGET_H
