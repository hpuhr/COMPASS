
#pragma once

#include <string>
#include <vector>

#include <QString>
#include <QKeySequence>

class QObject;
class QWidget;

/**
 */
namespace ui_test
{
    //basic ui trigger events
    bool injectKeyEvent(QWidget* root,
                        const QString& obj_name, 
                        const Qt::Key& key, 
                        int delay = -1);
    bool injectKeysEvent(QWidget* root,
                         const QString& obj_name, 
                         const QString& keys, 
                         int delay = -1);
    bool injectKeyCmdEvent(QWidget* root,
                           const QString& obj_name,
                           const QKeySequence& command);
    bool injectClickEvent(QWidget* root,
                          const QString& obj_name, 
                          int x = -1, 
                          int y = -1, 
                          Qt::MouseButton button = Qt::LeftButton, 
                          int delay = -1);

    //special ui trigger events
    bool injectMenuBarEvent(QWidget* root,
                            const QString& obj_name,
                            const QStringList& path_to_action,
                            int delay = -1);
    bool injectComboBoxEditEvent(QWidget* root,
                                 const QString& obj_name,
                                 const QString& entry_txt,
                                 int delay = -1);
    bool injectTabSelectionEvent(QWidget* root,
                                 const QString& obj_name,
                                 const QString& tab_txt,
                                 int delay = -1);
    bool injectToolSelectionEvent(QWidget* root,
                                  const QString& obj_name,
                                  const QString& tool_name,
                                  int delay = -1);
    bool injectLineEditEvent(QWidget* root,
                             const QString& obj_name,
                             const QString& text,
                             int delay = -1);
    bool injectSpinBoxEvent(QWidget* root,
                            const QString& obj_name,
                            int value,
                            int delay = -1);
    bool injectDoubleSpinBoxEvent(QWidget* root,
                                  const QString& obj_name,
                                  double value,
                                  int delay = -1);
    bool injectSliderEditEvent(QWidget* root,
                               const QString& obj_name,
                               double percent,
                               int delay = -1);
    bool injectCheckBoxEvent(QWidget* root,
                             const QString& obj_name,
                             bool on,
                             int delay = -1);
} // namespace ui_test
