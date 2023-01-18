
#include "ui_test_set.h"
#include "ui_test_find.h"
#include "ui_test_widget_setters.h"
#include "ui_test_event_injections.h"

#include <QObject>
#include <QWidget>

/**
 * Attempts a cast of the given widget to the given type and invokes
 * the templated value setter if the cast succeeds.
 */
#define TRY_INVOKE_UI_SETTER(WidgetType, widget, value, delay)                               \
    if (dynamic_cast<WidgetType*>(widget))                                                \
        return setUIElement<WidgetType>(dynamic_cast<WidgetType*>(widget), value, delay);

namespace ui_test
{

/**
 * Widget type agnostic value setter.
 */
bool setUIElement(QWidget* parent, 
                  const QString& obj_name, 
                  const QString& value, 
                  int delay)
{
    auto w = findObjectAs<QWidget>(parent, obj_name);
    if (w.first != FindObjectErrCode::NoError)
        return false;

    //declare setters for each type of widget here
    //corresponding template specializations need to be defined in 
    //ui_test_widget_setters.h
    TRY_INVOKE_UI_SETTER(QMenuBar, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QComboBox, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QTabWidget, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QToolBar, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QLineEdit, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QSpinBox, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QDoubleSpinBox, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QSlider, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QCheckBox, w.second, value, delay)
    TRY_INVOKE_UI_SETTER(QPushButton, w.second, value, delay)

    //type of widget could not be processed
    return false;
}

} // namespace ui_test
