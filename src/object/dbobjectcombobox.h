#ifndef DBOOBJECTCOMBOBOX_H
#define DBOOBJECTCOMBOBOX_H

#include <QComboBox>

#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "global.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class DBObjectComboBox: public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if type was changed
    void changedObject();

public:
    /// @brief Constructor
    DBObjectComboBox(bool allow_meta, QWidget * parent = 0)
    : QComboBox(parent), allow_meta_ (allow_meta)
    {
        assert (ATSDB::instance().objectManager().objects().size());
        if (allow_meta_)
            addItem (META_OBJECT_NAME.c_str());

        for (auto obj_it : ATSDB::instance().objectManager().objects())
        {
            addItem (obj_it.first.c_str());
        }

        setCurrentIndex (0);
        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedObject() ));

    }
    /// @brief Destructor
    virtual ~DBObjectComboBox() {}

    /// @brief Returns the currently selected data type
    std::string getObjectName ()
    {
        return currentText().toStdString();
    }

    /// @brief Sets the currently selected data type
    void setObjectName (const std::string &object_name)
    {
        int index = findText(QString(object_name.c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
    }

protected:
    bool allow_meta_;
};

#endif // DBOOBJECTCOMBOBOX_H
