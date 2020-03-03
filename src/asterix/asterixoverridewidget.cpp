#include "asterixoverridewidget.h"
#include "asteriximporttask.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QIntValidator>
#include <QDoubleValidator>

#include "textfielddoublevalidator.h"

ASTERIXOverrideWidget::ASTERIXOverrideWidget(ASTERIXImportTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout ();

    active_check_ = new QCheckBox ("Override Tracker Data Active");
    connect(active_check_, &QCheckBox::clicked, this, &ASTERIXOverrideWidget::activeCheckedSlot);
    main_layout->addWidget(active_check_);

    QGridLayout* grid = new QGridLayout ();

    unsigned int row = 0;

    // org sac
    grid->addWidget (new QLabel("Original SAC"), row, 0);

    sac_org_edit_ = new QLineEdit ();
    sac_org_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sac_org_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::sacOrgEditedSlot);
    grid->addWidget (sac_org_edit_, row, 1);

    // org sic
    ++row;
    grid->addWidget (new QLabel("Original SIC"), row, 0);

    sic_org_edit_ = new QLineEdit ();
    sic_org_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sic_org_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::sicOrgEditedSlot);
    grid->addWidget (sic_org_edit_, row, 1);

    // new sic
    ++row;
    grid->addWidget (new QLabel("New SAC"), row, 0);

    sac_new_edit_ = new QLineEdit ();
    sac_new_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sac_new_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::sacNewEditedSlot);
    grid->addWidget (sac_new_edit_, row, 1);

    // new sac
    ++row;
    grid->addWidget (new QLabel("New SIC"), row, 0);

    sic_new_edit_ = new QLineEdit ();
    sic_new_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sic_new_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::sicNewEditedSlot);
    grid->addWidget (sic_new_edit_, row, 1);

    // tod offset
    ++row;
    grid->addWidget (new QLabel("Time of Day Offset"), row, 0);

    tod_offset_edit_ = new QLineEdit ();
    tod_offset_edit_->setValidator(new TextFieldDoubleValidator(-24*3600, 24*3600, 3));
    connect(tod_offset_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::todOffsetEditedSlot);
    grid->addWidget (tod_offset_edit_, row, 1);

    main_layout->addLayout(grid);

    main_layout->addStretch();

    updateSlot();

    setLayout (main_layout);
}

ASTERIXOverrideWidget::~ASTERIXOverrideWidget()
{
}

void ASTERIXOverrideWidget::updateSlot()
{
    assert (active_check_);
    assert (sac_org_edit_);
    assert (sic_org_edit_);
    assert (sac_new_edit_);
    assert (sic_new_edit_);
    assert (tod_offset_edit_);

    active_check_->setChecked(task_.overrideActive());

    sac_org_edit_->setText(QString::number(task_.overrideSacOrg()));
    sic_org_edit_->setText(QString::number(task_.overrideSicOrg()));

    sac_new_edit_->setText(QString::number(task_.overrideSacNew()));
    sic_new_edit_->setText(QString::number(task_.overrideSicNew()));

    tod_offset_edit_->setText(QString::number(task_.overrideTodOffset()));
}

void ASTERIXOverrideWidget::activeCheckedSlot ()
{
    loginf << "ASTERIXOverrideWidget: activeCheckedSlot";
    assert (active_check_);

    task_.overrideActive (active_check_->checkState() == Qt::Checked);
}

void ASTERIXOverrideWidget::sacOrgEditedSlot (const QString& value)
{
    loginf << "ASTERIXOverrideWidget: sacOrgEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor (sac_org_edit_);

    if (sac_org_edit_->hasAcceptableInput())
        task_.overrideSacOrg(sac_org_edit_->text().toUInt());
}

void ASTERIXOverrideWidget::sicOrgEditedSlot (const QString& value)
{
    loginf << "ASTERIXOverrideWidget: sicOrgEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor (sic_org_edit_);

    if (sic_org_edit_->hasAcceptableInput())
        task_.overrideSicOrg(sic_org_edit_->text().toUInt());
}

void ASTERIXOverrideWidget::sacNewEditedSlot (const QString& value)
{
    loginf << "ASTERIXOverrideWidget: sacNewEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor (sac_new_edit_);

    if (sac_new_edit_->hasAcceptableInput())
        task_.overrideSacNew(sac_new_edit_->text().toUInt());
}

void ASTERIXOverrideWidget::sicNewEditedSlot (const QString& value)
{
    loginf << "ASTERIXOverrideWidget: sicNewEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor (sic_new_edit_);

    if (sic_new_edit_->hasAcceptableInput())
        task_.overrideSicNew(sic_new_edit_->text().toUInt());
}

void ASTERIXOverrideWidget::todOffsetEditedSlot (const QString& value)
{
    loginf << "ASTERIXOverrideWidget: todOffsetEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor (tod_offset_edit_);

    if (tod_offset_edit_->hasAcceptableInput())
        task_.overrideTodOffset(tod_offset_edit_->text().toFloat());

}
