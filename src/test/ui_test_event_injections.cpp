
#include "ui_test_event_injections.h"
#include "ui_test_find.h"
#include "logger.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QObject>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QTest>

namespace ui_test
{

/**
 * Injects key clicks into a widget.
*/
bool injectKeyEvent(QWidget* root,
                    const QString& obj_name,
                    const Qt::Key& key, 
                    int delay)
{
    auto w = findObjectAs<QWidget>(root, obj_name);
    if (w.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeyEvent", obj_name, w.first);
        return false;
    }

    loginf << "Injecting key " << QKeySequence(key).toString(QKeySequence::NativeText).toStdString();

    QTest::keyClick(w.second, key, Qt::NoModifier, delay);

    return true;
}

/**
 * Injects key clicks into a widget.
*/
bool injectKeysEvent(QWidget* root,
                     const QString& obj_name,
                     const QString& keys, 
                     int delay)
{
    auto w = findObjectAs<QWidget>(root, obj_name);
    if (w.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeysEvent", obj_name, w.first);
        return false;
    }

    //nothing to do?
    if (keys.isEmpty())
        return true;

    loginf << "Injecting key sequence " << keys.toStdString();

    QTest::keyClicks(w.second, keys, Qt::NoModifier, delay);

    return true;
}

/**
 * Injects (modified) key command(s) into windows or widgets.
 */
bool injectKeyCmdEvent(QWidget* root,
                       const QString& obj_name, 
                       const QKeySequence& command)
{
    auto obj = findObject(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectKeyCmdEvent", obj_name, obj.first);
        return false;
    }
    if (!obj.second->isWidgetType() && !obj.second->isWindowType())
    {
        logObjectError("injectKeyCmdEvent", obj_name, FindObjectErrCode::WrongType);
        return false;
    }

    //nothing to do?
    if (command.isEmpty())
        return true;

    //check if command is valid
    QString cmd_str;
    try
    {
        cmd_str = command.toString(QKeySequence::NativeText);
    }
    catch(...)
    {
        loginf << "injectKeyCmdEvent: Unrecognized key command";
        return false;
    }
    if (cmd_str.isEmpty())
    {
        loginf << "injectKeyCmdEvent: Unrecognized key command";
        return false;
    }
    
    auto injectionMsg = [ & ] (const std::string& obj_type) 
    {
        loginf << "Injecting command '" << cmd_str.toStdString();
    };
 
    if (obj.second->isWidgetType())
    {
        injectionMsg("widget");
        QTest::keySequence(dynamic_cast<QWidget*>(obj.second), command);
    }
    else //window
    {
        injectionMsg("window");
        QTest::keySequence(dynamic_cast<QWindow*>(obj.second), command);
    }

    return true;
}

/**
 * Injects mouse clicks into widgets or windows.
 */
bool injectClickEvent(QWidget* root,
                      const QString& obj_name, 
                      int x, 
                      int y, 
                      Qt::MouseButton button, 
                      int delay)
{
    auto obj = findObject(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectClickEvent", obj_name, obj.first);
        return false;
    }

    if (!obj.second->isWidgetType() && !obj.second->isWindowType())
    {
        logObjectError("injectClickEvent", obj_name, FindObjectErrCode::WrongType);
        return false;
    }

    //by default the click will by applied to the widget/window center...
    QPoint pos;

    //...but we can provide values
    if (x >= 0 && y >= 0)
    {
        pos = QPoint(x, y);
    }

    auto injectionMsg = [ & ] (const std::string& obj_type) 
    {
        loginf << "Injecting mouse button '" << (int)button << "' click at (" << std::to_string(x) << "," << std::to_string(y) << ")";
    };
 
    if (obj.second->isWidgetType())
    {
        injectionMsg("widget");
        QTest::mouseClick(dynamic_cast<QWidget*>(obj.second), button, 0, pos, delay);
    }
    else //window
    {
        injectionMsg("window");
        QTest::mouseClick(dynamic_cast<QWindow*>(obj.second), button, 0, pos, delay);
    }

    return true;
}

/**
 * Injects a trigger event to a subitem of the given menu bar.
 * @TODO: Maybe it would be nice to solve this with injected keyboard events rather than mouse events?
 */
bool injectMenuBarEvent(QWidget* root,
                        const QString& obj_name,
                        const QStringList& path,
                        int delay)
{
    auto obj = findObjectAs<QMenuBar>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectMenuBarEvent", obj_name, obj.first);
        return false;
    }

    //@TODO: should this return false?
    if (path.empty())
        return true;

    //finds the action with the given text in the given widget (widget functionality used by qmenu and qmenubar)
    auto findAction = [ = ] (QWidget* w, const QString& txt, bool is_menu)
    {
        for (auto a : w->actions())
        {
            if (!a || a->isSeparator() || (is_menu && !a->menu()))
                continue;

            QString txt_cur = a->text();
            if (txt_cur.startsWith("&"))
                txt_cur = txt_cur.mid(1);

            if (txt_cur == txt)
                return a;
        }
        return (QAction*)nullptr;
    };

    //get the geometry of the given action in the given menu widget
    auto actionGeometry = [ = ] (QWidget* w, QAction* action)
    {
        QMenu*    menu     = dynamic_cast<QMenu*>(w);
        QMenuBar* menu_bar = dynamic_cast<QMenuBar*>(w);

        //the menu widget could be either a QMenu or a QMenuBar
        if (menu)
            return menu->actionGeometry(action);
        if (menu_bar)
            return menu_bar->actionGeometry(action);

        return QRect();
    };

    size_t np = path.size();

    QWidget* current_menu   = obj.second;  //we start with the main menu
    QAction* current_action = nullptr;

    //iterate over path strings
    for (size_t i = 0; i < np; ++i)
    {
        //all path items should be menus except the last one
        bool is_menu = (i < np - 1);

        //find action in current menu for given item text
        current_action = findAction(current_menu, path[ i ], is_menu);
        if (!current_action)
        {
            loginf << "injectMainMenuEvent: Item text '" << path[ i ].toStdString() << "' not found";
            return false;
        }

        //get geometry of action in menu
        QRect r = actionGeometry(current_menu, current_action);

        //inject click at action
        if (!injectClickEvent(current_menu, "", r.x() + 1, r.y() + 1, Qt::LeftButton, delay))
            return false;

        //set current menu to the menu associated with the current action
        if (is_menu)
            current_menu = current_action->menu();
    }

    loginf << "injectMainMenuEvent: Final action is '" << current_action->text().toStdString() << "'";
    
    return true;
}

/**
 * Inject a combo selection event into the given combo box object.
 * 'entry_txt' is the text of the combo box entry to be selected.
 */
bool injectComboBoxEditEvent(QWidget* root,
                             const QString& obj_name,
                             const QString& entry_txt,
                             int delay)
{
    auto obj = findObjectAs<QComboBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectComboBoxEditEvent", obj_name, obj.first);
        return false;
    }

    int idx = obj.second->findText(entry_txt);
    if (idx < 0)
    {
        loginf << "injectComboBoxEditEvent: Entry '" << entry_txt.toStdString() << "' not found";
        return false;
    }

    //move either up or down using key injections
    int cur_idx = obj.second->currentIndex();
    Qt::Key key = (idx >= cur_idx ? Qt::Key_Down : Qt::Key_Up);

    int cnt = 0;
    while (obj.second->currentIndex() != idx && cnt++ < obj.second->count())
    {
        if(!injectKeyEvent(obj.second, "", key))
            return false;
    }

    return (obj.second->currentIndex() == idx);
}

/**
 * Inject a tab selection event into the given tab widget object.
 * 'tab_txt' is the text of the tab to be selected.
 */
bool injectTabSelectionEvent(QWidget* root,
                             const QString& obj_name,
                             const QString& tab_txt,
                             int delay)
{
    auto obj = findObjectAs<QTabWidget>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectTabSelectionEvent", obj_name, obj.first);
        return false;
    }

    if (obj.second->count() < 1)
    {
        loginf << "injectTabSelectionEvent: Tab empty";
        return false;
    }

    int idx = -1;
    for (int i = 0; i < obj.second->count(); ++i)
    {
        if (obj.second->tabText(i) == tab_txt)
        {
            idx = i;
            break;
        }
    }
        
    if (idx < 0)
    {
        loginf << "injectTabSelectionEvent: Tab text '" << tab_txt.toStdString() << "' not found";
        return false;
    }

    int cur_idx = obj.second->currentIndex();

    //move either left or right using key injections
    //@TODO: handle vertical tabs
    Qt::Key key = (idx >= cur_idx ? Qt::Key_Right : Qt::Key_Left);

    int cnt = 0;
    while (obj.second->currentIndex() != idx && cnt++ <= obj.second->count() + 1)
    {
        if (!injectKeyEvent(obj.second->tabBar(), "", key, delay))
            return false;
    }
        
    return (obj.second->currentIndex() == idx);
}

/**
 * Inject a tool triggering event into the given tool bar object.
 * 'tool_name' is the display name of the tool to be triggered.
 */
bool injectToolSelectionEvent(QWidget* root,
                              const QString& obj_name,
                              const QString& tool_name,
                              int delay)
{
    auto obj = findObjectAs<QToolBar>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectToolSelectionEvent", obj_name, obj.first);
        return false;
    }

    auto removeShortcut = [ & ] (const QString& txt) 
    {
        int idx = txt.lastIndexOf(" [");
        if (idx < 0)
            return txt;

        QString ret = txt;
        ret.truncate(idx);

        return ret;
    };

    int      idx    = -1;
    QAction* action = nullptr;
    for (int i = 0; i < obj.second->actions().count(); ++i)
    {
        QAction* a  = obj.second->actions()[ i ];
        if (a->isSeparator())
            continue;

        const QString tn = removeShortcut(a->text());
        if (tn == tool_name)
        {
            idx    = i;
            action = a;
            break;
        }
    }

    if (idx < 0)
    {
        loginf << "injectToolSelectionEvent: Tool name '" << tool_name.toStdString() << "' not found";
        return false;
    }

    QWidget* w = obj.second->widgetForAction(action);
    if (!w)
    {
        loginf << "injectToolSelectionEvent: Tool '" << tool_name.toStdString() << "' obtains no widget";
        return false;
    }

    if (!injectClickEvent(w, "", -1, -1, Qt::LeftButton, delay))
        return false;

    return true;
}

/**
 * Clears the given line edit object (via event injections) and injects new text into it.
 */
bool injectLineEditEvent(QWidget* root,
                         const QString& obj_name,
                         const QString& text,
                         int delay)
{
    auto obj = findObjectAs<QLineEdit>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectLineEditEvent", obj_name, obj.first);
        return false;
    }

    //clear line edit
    if (!injectKeyCmdEvent(obj.second, "", Qt::CTRL | Qt::Key_A))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", text, delay))
        return false;

    return true;
}

/**
 * Injects an edit action into the given spin box object.
 */
bool injectSpinBoxEvent(QWidget* root,
                        const QString& obj_name,
                        int value,
                        int delay)
{
    auto obj = findObjectAs<QSpinBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectSpinBoxEvent", obj_name, obj.first);
        return false;
    }

    QString txt = QString::number(value);

    //clear line edit
    if (!injectKeyCmdEvent(obj.second, "", Qt::CTRL | Qt::Key_A))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", txt, delay))
        return false;

    return (obj.second->value() == value);
}


/**
 * Injects an edit action into the given spin box object.
 */
bool injectDoubleSpinBoxEvent(QWidget* root,
                              const QString& obj_name,
                              double value,
                              int delay)
{
    auto obj = findObjectAs<QDoubleSpinBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectDoubleSpinBoxEvent", obj_name, obj.first);
        return false;
    }

    QString txt = QString::number(value, 'f', obj.second->decimals());
    double value_trunc = txt.toDouble();

    //clear line edit
    if (!injectKeyCmdEvent(obj.second, "", Qt::CTRL | Qt::Key_A))
        return false;
 
    //fill with new content
    if (!injectKeysEvent(obj.second, "", txt, delay))
        return false;

    return (obj.second->value() == value_trunc);
}

/**
 * Injects a slider edit action into the given slider object.
 * The given percent [0,100] will result in a discrete slider value between the slider's minimum() and maximum().
 * Note that first pageStep()'s will be injected to get a rough position, and the singleStep()'s will be used for fine tuning.
 * Thus, depending on the slider range and the page step size, a certain number of slider changes will be triggered.
 * Also note that if the single step size is bigger than 1, an exact result cannot be guaranteed.
 */
bool injectSliderEditEvent(QWidget* root,
                           const QString& obj_name,
                           double percent,
                           int delay)
{
    auto obj = findObjectAs<QSlider>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectSliderEditEvent", obj_name, obj.first);
        return false;
    }

    double t         = std::min(1.0, std::max(0.0, percent / 100.0));
    int    value     = (1.0 - t) * obj.second->minimum() + t * obj.second->maximum();

    int    single_step = obj.second->singleStep();
    int    page_step   = obj.second->pageStep();

    auto computeMaxSteps = [ = ] (int v, int vtarget, int step)
    {
        return std::abs(v - vtarget) / step + 2;
    };

    const int max_steps_page = computeMaxSteps(value, obj.second->value(), page_step);

    //first inject page steps until we are below page step accuracy
    int cnt = 0;
    while (std::abs(value - obj.second->value()) >= page_step && cnt++ <= max_steps_page)
        if (!injectKeyEvent(obj.second, "", obj.second->value() <= value ? Qt::Key_PageUp : Qt::Key_PageDown, delay))
            return false;

    const int max_steps_single = computeMaxSteps(value, obj.second->value(), single_step);

    //then inject single steps until we are below single step accuracy (usually single step size = 1, resulting in the sought target value)
    cnt = 0;
    while (std::abs(value - obj.second->value()) >= single_step && cnt++ <= max_steps_single)
        if (!injectKeyEvent(obj.second, "", obj.second->value() <= value ? Qt::Key_Right : Qt::Key_Left, delay))
            return false;

    return (std::abs(value - obj.second->value()) < single_step);
}

/**
 * 
 */
bool injectCheckBoxEvent(QWidget* root,
                         const QString& obj_name,
                         bool on,
                         int delay)
{
    auto obj = findObjectAs<QCheckBox>(root, obj_name);
    if (obj.first != FindObjectErrCode::NoError)
    {
        logObjectError("injectCheckBoxEvent", obj_name, obj.first);
        return false;
    }

    //check state ok?
    if (obj.second->isChecked() == on)
        return true;

    //toggle check state
    if (!injectClickEvent(obj.second, "", 2, 2, Qt::LeftButton, delay))
        return false;

    return obj.second->isChecked() == on;
}

} // namespace ui_test
