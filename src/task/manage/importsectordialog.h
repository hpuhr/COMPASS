#ifndef IMPORTSECTORDIALOG_H
#define IMPORTSECTORDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;

class ImportSectorDialog : public QDialog
{
    Q_OBJECT

public slots:
    void cancelSlot();
    void importSlot();

public:
    ImportSectorDialog(const std::string& layer_name, QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    std::string layerName();
    bool exclude ();

protected:
    QLineEdit* layer_name_edit_edit_ {nullptr};
    QCheckBox* exclude_check_ {nullptr};

    //bool import_ {false};
};

#endif // IMPORTSECTORDIALOG_H
