#pragma once

#include "dbcontent/label/labelgenerator.h"

#include <QWidget>
#include <QIcon>

//class QListWidget;
class QPushButton;
class QGridLayout;

namespace dbContent
{

class LabelGenerator;

class LabelDSWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateListSlot();
    void sourceClickedSlot();
    void changeLineSlot();
    void changeDirectionSlot();
    void selectDirectionSlot();
    void selectLineSlot();

public:
    LabelDSWidget(LabelGenerator& label_generator, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~LabelDSWidget();

    void forceUpdateList();

protected:
    LabelGenerator& label_generator_;

    //QListWidget* list_widget_{nullptr};

    QIcon arrow_lu_;
    QIcon arrow_ru_;
    QIcon arrow_ld_;
    QIcon arrow_rd_;

    QGridLayout* ds_grid_{nullptr};

    std::map<unsigned int, QPushButton*> line_buttons_;
    std::map<unsigned int, QPushButton*> direction_buttons_;

    QIcon& iconForDirection(LabelDirection direction);

    std::map<std::string, std::string> old_sources_;
};

}
