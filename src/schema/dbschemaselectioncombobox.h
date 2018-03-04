#ifndef DBSCHEMASELECTIONCOMBOBOX_H
#define DBSCHEMASELECTIONCOMBOBOX_H

#include <QComboBox>

#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschema.h"

class DBSchemaSelectionComboBox : public QComboBox
{
public:
    DBSchemaSelectionComboBox()
    {
        update ();
    }

    DBSchemaSelectionComboBox(const std::string& selection)
    {
        update ();
        select (selection);
    }

    void update ()
    {
        logdbg  << "DBSchemaSelectionComboBox: update";

        std::string selection;

        if (count() > 0)
            selection = currentText().toStdString();

        while (count() > 0)
            removeItem (0);

        for (auto& schema_it : ATSDB::instance().schemaManager().getSchemas())
            addItem (schema_it.first.c_str());

        if (selection.size())
            select (selection);
    }

    void select (const std::string& selection)
    {
        int index = findText(selection.c_str());

        if (index >= 0)
            setCurrentIndex (index);
    }

protected:
};

#endif // DBSCHEMASELECTIONCOMBOBOX_H
