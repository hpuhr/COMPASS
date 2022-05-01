#include "dbcontentlabelgeneratorwidget.h"
#include "dbcontentlabelgenerator.h"
#include "logger.h"

#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>

using namespace std;

DBContentLabelGeneratorWidget::DBContentLabelGeneratorWidget(DBContentLabelGenerator& label_generator)
    : label_generator_(label_generator)
{
    QFormLayout* main_layout = new QFormLayout;

    QCheckBox* auto_label_check = new QCheckBox();
    auto_label_check->setChecked(label_generator_.autoLabel());
    connect(auto_label_check, &QCheckBox::clicked,
            this, &DBContentLabelGeneratorWidget::autoLabelChangedSlot);
    main_layout->addRow("Auto Label", auto_label_check);

    // lod
    QComboBox* lod_box = new QComboBox();
    lod_box->addItems({"Auto", "1", "2", "3"});

    if (label_generator_.autoLOD())
        lod_box->setCurrentText("Auto");
    else
        lod_box->setCurrentText(QString::number(label_generator_.currentLOD()));

    connect(lod_box, &QComboBox::currentTextChanged,
            this, &DBContentLabelGeneratorWidget::lodChangedSlot);
    main_layout->addRow(tr("Level of Detail"), lod_box);

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
