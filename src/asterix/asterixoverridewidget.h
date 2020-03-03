#ifndef ASTERIXOVERRIDEWIDGET_H
#define ASTERIXOVERRIDEWIDGET_H

#include <QWidget>

class ASTERIXImportTask;

class QCheckBox;
class QLineEdit;

class ASTERIXOverrideWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateSlot();

    void activeCheckedSlot ();

    void sacOrgEditedSlot (const QString& value);
    void sicOrgEditedSlot (const QString& value);
    void sacNewEditedSlot (const QString& value);
    void sicNewEditedSlot (const QString& value);
    void todOffsetEditedSlot (const QString& value);

public:
    ASTERIXOverrideWidget(ASTERIXImportTask& task, QWidget* parent=nullptr);
    virtual ~ASTERIXOverrideWidget();

protected:
    ASTERIXImportTask& task_;

    QCheckBox* active_check_ {nullptr};

    QLineEdit* sac_org_edit_ {nullptr};
    QLineEdit* sic_org_edit_ {nullptr};

    QLineEdit* sac_new_edit_ {nullptr};
    QLineEdit* sic_new_edit_ {nullptr};

    QLineEdit* tod_offset_edit_ {nullptr};
};

#endif // ASTERIXOVERRIDEWIDGET_H
