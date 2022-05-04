#include "dbcontentlabelgeneratorwidget.h"
#include "dbcontentlabelgenerator.h"
#include "dbcontentlabeldswidget.h"
#include "logger.h"

#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>

using namespace std;

DBContentLabelGeneratorWidget::DBContentLabelGeneratorWidget(DBContentLabelGenerator& label_generator)
    : label_generator_(label_generator)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout1 = new QFormLayout;

    QCheckBox* auto_label_check = new QCheckBox();
    auto_label_check->setChecked(label_generator_.autoLabel());
    connect(auto_label_check, &QCheckBox::clicked,
            this, &DBContentLabelGeneratorWidget::autoLabelChangedSlot);
    form_layout1->addRow("Auto Label", auto_label_check);

    // lod
    QComboBox* lod_box = new QComboBox();
    lod_box->addItems({"Auto", "1", "2", "3"});

    if (label_generator_.autoLOD())
        lod_box->setCurrentText("Auto");
    else
        lod_box->setCurrentText(QString::number(label_generator_.currentLOD()));

    connect(lod_box, &QComboBox::currentTextChanged,
            this, &DBContentLabelGeneratorWidget::lodChangedSlot);
    form_layout1->addRow(tr("Level of Detail"), lod_box);

    main_layout->addLayout(form_layout1);

    DBContentLabelDSWidget* ds_widget = new DBContentLabelDSWidget(label_generator_);
    main_layout->addWidget(ds_widget);

    unsigned int row=0;

    // filters
    QGridLayout* filter_layout = new QGridLayout();

    QCheckBox* filter_mode3a_box = new QCheckBox("Mode 3/A Codes");
    filter_mode3a_box->setChecked(label_generator_.filterMode3aActive());
    connect(filter_mode3a_box, &QCheckBox::clicked,
            this, &DBContentLabelGeneratorWidget::filterMode3AActiveChangedSlot);
    filter_layout->addWidget(filter_mode3a_box, row, 0);

    QLineEdit* filter_mode3a_edit = new QLineEdit();
    filter_mode3a_edit->setText(label_generator_.filterMode3aValues().c_str());
    connect(filter_mode3a_edit, &QLineEdit::textEdited,
            this, &DBContentLabelGeneratorWidget::filterMode3ChangedSlot);
    filter_layout->addWidget(filter_mode3a_edit, row, 1);

    main_layout->addLayout(filter_layout);

    setLayout(main_layout);
}

DBContentLabelGeneratorWidget::~DBContentLabelGeneratorWidget()
{

}

void DBContentLabelGeneratorWidget::autoLabelChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: autoLabelChangedSlot: checked " << checked;

    label_generator_.autoLabel(checked);
}

void DBContentLabelGeneratorWidget::lodChangedSlot(const QString& text)
{
    string lod = text.toStdString();

    loginf << "DBContentLabelGeneratorWidget: lodChangedSlot: value " << lod;

    if (lod == "Auto")
        label_generator_.autoLOD(true);
    else
    {
        label_generator_.autoLOD(false);
        label_generator_.currentLOD(stoul(lod));
    }
}

void DBContentLabelGeneratorWidget::filterMode3AActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterMode3AActiveChangedSlot: checked " << checked;

    label_generator_.filterMode3aActive(checked);
}
void DBContentLabelGeneratorWidget::filterMode3ChangedSlot(const QString& text)
{
    string values = text.toStdString();

    loginf << "DBContentLabelGeneratorWidget: filterMode3ChangedSlot: value " << values;

    label_generator_.filterMode3aValues(values);
}
