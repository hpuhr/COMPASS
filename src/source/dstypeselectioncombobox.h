#ifndef DSTYPESELECTIONCOMBOBOX_H
#define DSTYPESELECTIONCOMBOBOX_H

#include "datasourcemanager.h"
//#include "global.h"
#include "logger.h"

#include <QComboBox>

#include <algorithm>

class DSTypeSelectionComboBox : public QComboBox
{
    Q_OBJECT

signals:
    void changedTypeSignal(const QString& type);

public slots:
    void changedSlot()
    {
        if (doing_update_)
            return;

        loginf << "DSTypeSelectionComboBox: changed " << currentText().toStdString();

        ds_type_ = currentText().toStdString();

        emit changedTypeSignal(ds_type_.c_str());
    }

public:
    DSTypeSelectionComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
    {
        addItem(""); // to show none

        for (auto it : DataSourceManager::data_source_types_)
            addItem(it.c_str());

        updateCurrentText();

        connect(this, SIGNAL(currentTextChanged(const QString&)), this, SLOT(changedSlot()));
    }

    virtual ~DSTypeSelectionComboBox() {}

    std::string type()
    {
        if (ds_type_.size())
            assert (std::find(DataSourceManager::data_source_types_.begin(),
                              DataSourceManager::data_source_types_.end(), ds_type_)
                    != DataSourceManager::data_source_types_.end());

        return ds_type_;
    }

    void setType(const std::string& type)
    {
        ds_type_ = type;

        if (ds_type_.size())
            assert (std::find(DataSourceManager::data_source_types_.begin(),
                              DataSourceManager::data_source_types_.end(), ds_type_)
                    != DataSourceManager::data_source_types_.end());

        updateCurrentText();
    }

protected:
    std::string ds_type_;
    bool doing_update_ {false};

    void updateCurrentText()
    {
        doing_update_ = true;

        if (ds_type_.size())
            assert (std::find(DataSourceManager::data_source_types_.begin(),
                              DataSourceManager::data_source_types_.end(), ds_type_)
                    != DataSourceManager::data_source_types_.end());

        setCurrentText(ds_type_.c_str());

        doing_update_ = false;
    }

};

#endif // DSTYPESELECTIONCOMBOBOX_H
