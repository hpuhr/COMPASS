#ifndef ASTERIXMAPPINGCOMBOBOX_H
#define ASTERIXMAPPINGCOMBOBOX_H

#include <QComboBox>

#include "asteriximporttask.h"

class ASTERIXMappingComboBox : public QComboBox
{
    Q_OBJECT

public slots:
    void changedMappingSlot()
    {
        emit changedMappingSignal(cat_, currentText().toStdString());
    }

signals:
    /// @brief Emitted if type was changed
    void changedMappingSignal(unsigned int cat, const std::string& mapping_str);

public:
    /// @brief Constructor
    ASTERIXMappingComboBox(ASTERIXImportTask& task, unsigned int cat, QWidget* parent = 0)
        : QComboBox(parent), task_(task), cat_(cat)
    {
        updateMappings();
        connect(this, SIGNAL(activated(const QString&)), this, SLOT(changedMappingSlot()));
    }
    /// @brief Destructor
    virtual ~ASTERIXMappingComboBox() {}

    void updateMappings()
    {
        clear();

        for (auto map_it : task_.getPossibleMappings(cat_))
        {
            addItem(map_it.c_str());
        }

        setCurrentIndex(0);
    }

    unsigned int cat() { return cat_; }

    /// @brief Returns the currently selected framing
    std::string getMapping() { return currentText().toStdString(); }

    /// @brief Sets the currently selected data type
    void setMapping(const std::string& mapping)
    {
        int index = findText(QString(mapping.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

protected:
    ASTERIXImportTask& task_;
    unsigned int cat_ {0};
};

#endif // ASTERIXMAPPINGCOMBOBOX_H
