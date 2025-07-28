#pragma once

#include <QFrame>

class QCheckBox;
class QGridLayout;

class SelectDataSourcesWidget : public QFrame
{
    Q_OBJECT

signals:
    void selectionChangedSignal(std::map<std::string, bool> selection);

protected slots:
    void toggleDataSourceSlot();

public:
    SelectDataSourcesWidget(const std::string& title, const std::string& ds_type,
                            QWidget* parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    virtual ~SelectDataSourcesWidget();

    void updateSelected(std::map<std::string, bool> selection);

protected:
    std::string title_;
    std::string ds_type_;

    QGridLayout* data_source_layout_ {nullptr};
    std::map<unsigned int, QCheckBox*> data_sources_checkboxes_;

    void updateCheckboxesChecked();

};
