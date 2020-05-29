#ifndef ASTERIXSPFEDITIONCOMBOBOX_H
#define ASTERIXSPFEDITIONCOMBOBOX_H

#include <jasterix/jasterix.h>
#include <jasterix/spfedition.h>

#include <QComboBox>
#include <memory>

#include "asteriximporttask.h"

class ASTERIXSPFEditionComboBox : public QComboBox
{
    Q_OBJECT

  public slots:
    void changedSPFEditionSlot(const QString& edition)
    {
        emit changedSPFSignal(category_->number(), edition.toStdString());
    }

  signals:
    /// @brief Emitted if REF was changed
    void changedSPFSignal(const std::string& cat_str, const std::string& ref_ed_str);

  public:
    /// @brief Constructor
    ASTERIXSPFEditionComboBox(ASTERIXImportTask& task,
                              const std::shared_ptr<jASTERIX::Category> category,
                              QWidget* parent = nullptr)
        : QComboBox(parent), task_(task), category_(category)
    {
        addItem("");

        if (category_->spfEditions().size())
        {
            for (auto& spf_it : category_->spfEditions())
            {
                addItem(spf_it.first.c_str());
            }

            setCurrentIndex(0);
            connect(this, SIGNAL(activated(const QString&)), this,
                    SLOT(changedSPFEditionSlot(const QString&)));
        }
        else
            setDisabled(true);
    }
    /// @brief Destructor
    virtual ~ASTERIXSPFEditionComboBox() {}

    /// @brief Returns the currently selected framing
    std::string getSPFEdition() { return currentText().toStdString(); }

    /// @brief Sets the currently selected edition
    void setSPFEdition(const std::string& spf_ed_str)
    {
        int index = findText(QString(spf_ed_str.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

  protected:
    ASTERIXImportTask& task_;
    const std::shared_ptr<jASTERIX::Category> category_;
};

#endif  // ASTERIXSPFEDITIONCOMBOBOX_H
