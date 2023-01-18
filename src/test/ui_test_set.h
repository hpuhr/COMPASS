
#pragma once

class QWidget;
class QString;

namespace ui_test
{
    bool setUIElement(QWidget* parent, 
                      const QString& obj_name, 
                      const QString& value, 
                      int delay = -1);
} // namespace ui_test
