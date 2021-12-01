#ifndef METADBOVARIABLEDETAILWIDGET_H
#define METADBOVARIABLEDETAILWIDGET_H

#include <QWidget>

class DBObjectManager;
class MetaDBOVariable;

class MetaDBOVariableDetailWidget : public QWidget
{
    Q_OBJECT

public slots:

public:
    MetaDBOVariableDetailWidget(DBObjectManager& dbo_man, QWidget* parent = nullptr);

    void show (MetaDBOVariable& meta_var);

private:
    DBObjectManager& dbo_man_;

    bool has_current_entry_ {false};
    MetaDBOVariable* meta_var_ {nullptr};
};

#endif // METADBOVARIABLEDETAILWIDGET_H
