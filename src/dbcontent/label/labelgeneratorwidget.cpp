#include "dbcontent/label/labelgeneratorwidget.h"
#include "dbcontent/label/labelgenerator.h"
#include "dbcontent/label/labeldswidget.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "logger.h"
#include "files.h"

#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QMenu>

using namespace std;
using namespace Utils;

namespace dbContent
{

LabelGeneratorWidget::LabelGeneratorWidget(LabelGenerator& label_generator)
    : label_generator_(label_generator)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* edit_layout = new QHBoxLayout();
    edit_layout->addStretch();

    QPushButton* ed_edit = new QPushButton();
    ed_edit->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    ed_edit->setFixedSize(UI_ICON_SIZE);
    ed_edit->setFlat(UI_ICON_BUTTON_FLAT);
    connect(ed_edit, &QPushButton::clicked,
            this, &LabelGeneratorWidget::editSettingsSlot);
    ed_edit->setToolTip("Edit Label Contents");
    edit_layout->addWidget(ed_edit);

    main_layout->addLayout(edit_layout);

    QFormLayout* form_layout1 = new QFormLayout;

    QCheckBox* auto_label_check = new QCheckBox();
    auto_label_check->setChecked(label_generator_.autoLabel());
    connect(auto_label_check, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::autoLabelChangedSlot);
    form_layout1->addRow("Auto Label", auto_label_check);

    // lod
    QComboBox* lod_box = new QComboBox();
    lod_box->addItems({"Auto", "1", "2", "3"});

    if (label_generator_.autoLOD())
        lod_box->setCurrentText("Auto");
    else
        lod_box->setCurrentText(QString::number(label_generator_.currentLOD()));

    connect(lod_box, &QComboBox::currentTextChanged,
            this, &LabelGeneratorWidget::lodChangedSlot);
    form_layout1->addRow(tr("Level of Detail"), lod_box);

    main_layout->addLayout(form_layout1);

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    LabelDSWidget* ds_widget = new LabelDSWidget(label_generator_);
    scroll_area->setWidget(ds_widget);
    scroll_area->setMinimumHeight(300);
    scroll_area->setMaximumHeight(400);

    main_layout->addWidget(scroll_area);

    unsigned int row=0;

    // filters
    QGridLayout* filter_layout = new QGridLayout();

    // m3a
    QCheckBox* filter_mode3a_box = new QCheckBox("Mode 3/A Codes");
    filter_mode3a_box->setChecked(label_generator_.filterMode3aActive());
    connect(filter_mode3a_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterMode3AActiveChangedSlot);
    filter_layout->addWidget(filter_mode3a_box, row, 0);

    QLineEdit* filter_mode3a_edit = new QLineEdit();
    filter_mode3a_edit->setText(label_generator_.filterMode3aValues().c_str());
    connect(filter_mode3a_edit, &QLineEdit::textEdited,
            this, &LabelGeneratorWidget::filterMode3AChangedSlot);
    filter_layout->addWidget(filter_mode3a_edit, row, 1);

    // mc
    ++row;
    QCheckBox* filter_modec_min_box = new QCheckBox("Mode C Min [FL]");
    filter_modec_min_box->setChecked(label_generator_.filterModecMinActive());
    connect(filter_modec_min_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterModeCMinActiveChangedSlot);
    filter_layout->addWidget(filter_modec_min_box, row, 0);

    QLineEdit* filter_modec_min_edit = new QLineEdit();
    filter_modec_min_edit->setText(QString::number(label_generator_.filterModecMinValue()));
    connect(filter_modec_min_edit, &QLineEdit::textEdited,
            this, &LabelGeneratorWidget::filterModeCMinChangedSlot);
    filter_layout->addWidget(filter_modec_min_edit, row, 1);

    ++row;
    QCheckBox* filter_modec_max_box = new QCheckBox("Mode C Max [FL]");
    filter_modec_max_box->setChecked(label_generator_.filterModecMaxActive());
    connect(filter_modec_max_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterModeCMaxActiveChangedSlot);
    filter_layout->addWidget(filter_modec_max_box, row, 0);

    QLineEdit* filter_modec_max_edit = new QLineEdit();
    filter_modec_max_edit->setText(QString::number(label_generator_.filterModecMaxValue()));
    connect(filter_modec_max_edit, &QLineEdit::textEdited,
            this, &LabelGeneratorWidget::filterModeCMaxChangedSlot);
    filter_layout->addWidget(filter_modec_max_edit, row, 1);

    ++row;
    QCheckBox* filter_modec_null_box = new QCheckBox("Mode C NULL");
    filter_modec_null_box->setChecked(label_generator_.filterModecNullWanted());
    connect(filter_modec_null_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterModeCNullWantedChangedSlot);
    filter_layout->addWidget(filter_modec_null_box, row, 0);

    // ti
    ++row;
    QCheckBox* filter_ti_box = new QCheckBox("Aircraft Identification");
    filter_ti_box->setChecked(label_generator_.filterTIActive());
    connect(filter_ti_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterTIActiveChangedSlot);
    filter_layout->addWidget(filter_ti_box, row, 0);

    QLineEdit* filter_ti_edit = new QLineEdit();
    filter_ti_edit->setText(label_generator_.filterTIValues().c_str());
    connect(filter_ti_edit, &QLineEdit::textEdited,
            this, &LabelGeneratorWidget::filterTIChangedSlot);
    filter_layout->addWidget(filter_ti_edit, row, 1);

    // ta
    ++row;
    QCheckBox* filter_ta_box = new QCheckBox("Aircraft Address");
    filter_ta_box->setChecked(label_generator_.filterTAActive());
    connect(filter_ta_box, &QCheckBox::clicked,
            this, &LabelGeneratorWidget::filterTAActiveChangedSlot);
    filter_layout->addWidget(filter_ta_box, row, 0);

    QLineEdit* filter_ta_edit = new QLineEdit();
    filter_ta_edit->setText(label_generator_.filterTAValues().c_str());
    connect(filter_ta_edit, &QLineEdit::textEdited,
            this, &LabelGeneratorWidget::filterTAChangedSlot);
    filter_layout->addWidget(filter_ta_edit, row, 1);

    main_layout->addLayout(filter_layout);

    main_layout->addStretch();

    setLayout(main_layout);
}

LabelGeneratorWidget::~LabelGeneratorWidget()
{
}

void LabelGeneratorWidget::editSettingsSlot()
{
    QMenu menu;

    for (auto& db_cont_it : COMPASS::instance().dbContentManager())
    {
        QAction* action = new QAction(("Edit "+db_cont_it.first).c_str(), this);
        connect (action, &QAction::triggered, this, &LabelGeneratorWidget::editDBContentSlot);
        action->setProperty("dbcontent_name", db_cont_it.first.c_str());
        menu.addAction(action);
    }

    menu.exec(QCursor::pos());
}

void LabelGeneratorWidget::editDBContentSlot()
{
    QVariant name = sender()->property("dbcontent_name");

    string dbcontent_name = name.toString().toStdString();

    loginf << "DBContentLabelGeneratorWidget: editDBContentSlot: dbcontent " << dbcontent_name;

    label_generator_.editLabelContents(dbcontent_name);
}

void LabelGeneratorWidget::autoLabelChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: autoLabelChangedSlot: checked " << checked;

    label_generator_.autoLabel(checked);
}

void LabelGeneratorWidget::lodChangedSlot(const QString& text)
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

void LabelGeneratorWidget::filterMode3AActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterMode3AActiveChangedSlot: checked " << checked;

    label_generator_.filterMode3aActive(checked);
}
void LabelGeneratorWidget::filterMode3AChangedSlot(const QString& text)
{
    string values = text.toStdString();

    loginf << "DBContentLabelGeneratorWidget: filterMode3ChangedSlot: value " << values;

    label_generator_.filterMode3aValues(values);
}

void LabelGeneratorWidget::filterModeCMinActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterModeCMinActiveChangedSlot: checked " << checked;

    label_generator_.filterModecMinActive(checked);
}
void LabelGeneratorWidget::filterModeCMinChangedSlot(const QString& text)
{
    bool ok;

    float value = text.toFloat(&ok);

    if (ok)
        label_generator_.filterModecMinValue(value);
    else
        loginf << "DBContentLabelGeneratorWidget: filterModeCMinChangedSlot: impossible value '"
               << text.toStdString() << "'";

}
void LabelGeneratorWidget::filterModeCMaxActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterModeCMaxActiveChangedSlot: checked " << checked;

    label_generator_.filterModecMaxActive(checked);
}
void LabelGeneratorWidget::filterModeCMaxChangedSlot(const QString& text)
{
    bool ok;

    float value = text.toFloat(&ok);

    if (ok)
        label_generator_.filterModecMaxValue(value);
    else
        loginf << "DBContentLabelGeneratorWidget: filterModeCMaxChangedSlot: impossible value '"
               << text.toStdString() << "'";
}
void LabelGeneratorWidget::filterModeCNullWantedChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterModeCNullWantedChangedSlot: checked " << checked;
    label_generator_.filterModecNullWanted(checked);
}

void LabelGeneratorWidget::filterTIActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterTIActiveChangedSlot: checked " << checked;

    label_generator_.filterTIActive(checked);
}
void LabelGeneratorWidget::filterTIChangedSlot(const QString& text)
{
    string values = text.toStdString();

    loginf << "DBContentLabelGeneratorWidget: filterTIChangedSlot: value " << values;

    label_generator_.filterTIValues(values);
}

void LabelGeneratorWidget::filterTAActiveChangedSlot(bool checked)
{
    loginf << "DBContentLabelGeneratorWidget: filterTAActiveChangedSlot: checked " << checked;

    label_generator_.filterTAActive(checked);
}
void LabelGeneratorWidget::filterTAChangedSlot(const QString& text)
{
    string values = text.toStdString();

    loginf << "DBContentLabelGeneratorWidget: filterTAChangedSlot: value " << values;

    label_generator_.filterTAValues(values);
}

}
