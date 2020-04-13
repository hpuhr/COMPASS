#ifndef ASTERIXREFCOMBOBOX_H
#define ASTERIXREFCOMBOBOX_H

#include <jasterix/jasterix.h>
#include <jasterix/refedition.h>

#include <QComboBox>
#include <memory>

#include "asteriximporttask.h"

class ASTERIXREFEditionComboBox : public QComboBox
{
    Q_OBJECT

  public slots:
    void changedREFEditionSlot(const QString& edition)
    {
        emit changedREFSignal(category_->number(), edition.toStdString());
    }

  signals:
    /// @brief Emitted if REF was changed
    void changedREFSignal(const std::string& cat_str, const std::string& ref_ed_str);

  public:
    /// @brief Constructor
    ASTERIXREFEditionComboBox(ASTERIXImportTask& task,
                              const std::shared_ptr<jASTERIX::Category> category,
                              QWidget* parent = nullptr)
        : QComboBox(parent), task_(task), category_(category)
    {
        addItem("");

        if (category_->refEditions().size())
        {
            for (auto& ref_it : category_->refEditions())
            {
                addItem(ref_it.first.c_str());
            }

            setCurrentIndex(0);
            connect(this, SIGNAL(activated(const QString&)), this,
                    SLOT(changedREFEditionSlot(const QString&)));
        }
        else
            setDisabled(true);
    }
    /// @brief Destructor
    virtual ~ASTERIXREFEditionComboBox() {}

    /// @brief Returns the currently selected framing
    std::string getREFEdition() { return currentText().toStdString(); }

    /// @brief Sets the currently selected edition
    void setREFEdition(const std::string& ref_ed_str)
    {
        int index = findText(QString(ref_ed_str.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

  protected:
    ASTERIXImportTask& task_;
    const std::shared_ptr<jASTERIX::Category> category_;
};

#endif  // ASTERIXREFCOMBOBOX_H
