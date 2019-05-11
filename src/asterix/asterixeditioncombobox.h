#ifndef ASTERIXEDITIONCOMBOBOX_H
#define ASTERIXEDITIONCOMBOBOX_H

#include "asteriximportertask.h"

#include <jasterix/jasterix.h>
#include <jasterix/category.h>

#include <QComboBox>

class ASTERIXEditionComboBox: public QComboBox
{
    Q_OBJECT

public slots:
    void changedEditionSlot(const QString &edition)
    {
        emit changedEdition(category_.number(), edition.toStdString());
    }

signals:
    /// @brief Emitted if edition was changed
    void changedEdition(const std::string& cat_str, const std::string& ed_str);

public:
    /// @brief Constructor
    ASTERIXEditionComboBox(ASTERIXImporterTask& task, const jASTERIX::Category& category, QWidget * parent = 0)
    : QComboBox(parent), task_(task), category_(category)
    {
        //const std::map<std::string, std::shared_ptr<Edition>>& editions() const;
        for (auto& ed_it : category_.editions())
        {
            addItem (ed_it.first.c_str());
        }

        setCurrentIndex (0);
        connect(this, SIGNAL(activated(const QString &)), this, SLOT(changedEditionSlot(const QString &)));

    }
    /// @brief Destructor
    virtual ~ASTERIXEditionComboBox() {}

    /// @brief Returns the currently selected framing
    std::string getEdition ()
    {
        return currentText().toStdString();
    }

    /// @brief Sets the currently selected edition
    void setEdition (const std::string& edition)
    {
        int index = findText(QString(edition.c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
    }

protected:
    ASTERIXImporterTask& task_;
    const jASTERIX::Category& category_;

};


#endif // ASTERIXEDITIONCOMBOBOX_H
