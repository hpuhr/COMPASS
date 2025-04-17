#ifndef IMPORTSECTORDIALOG_H
#define IMPORTSECTORDIALOG_H

#include <QDialog>
#include <QColor>

class QLineEdit;
class QCheckBox;
class QPushButton;

class ImportSectorDialog : public QDialog
{
    Q_OBJECT

public slots:
    void colorSlot();

    void cancelSlot();
    void importSlot();

public:
    ImportSectorDialog(const std::string& layer_name, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    std::string layerName();
    bool exclude ();
    QColor color ();

protected:
    QLineEdit* layer_name_edit_edit_ {nullptr};
    QCheckBox* exclude_check_ {nullptr};
    QPushButton* color_button_ {nullptr};

    QColor color_;
};

#endif // IMPORTSECTORDIALOG_H
