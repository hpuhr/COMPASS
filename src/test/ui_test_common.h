
#pragma once

#include <QString>
#include <QRegularExpression>

#define UI_TEST_OBJ_NAME(widget_ptr, display_name) widget_ptr->setObjectName(ui_test::generateObjectName(display_name));

namespace ui_test
{

inline QString generateObjectName(const QString& text)
{
    QString obj_name = text.toLower();
    obj_name.remove(QRegularExpression("^[-.:\\s]+"));
    obj_name.remove(QRegularExpression("[-.:\\s]+$"));
    obj_name.replace(QRegularExpression("[-.:\\s]+"), "_");

    return obj_name;
}

} // namespace ui_test
