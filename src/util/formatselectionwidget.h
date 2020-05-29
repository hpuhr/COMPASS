#ifndef FORMATSELECTIONWIDGET_H
#define FORMATSELECTIONWIDGET_H

#include <QMenu>
#include <QPushButton>

#include "format.h"
#include "property.h"

/**
 * @brief Sets a Unit using a context menu
 */
class FormatSelectionWidget : public QPushButton
{
    Q_OBJECT

  protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot(QAction* action);
    /// @brief Shows the context menu
    void showMenuSlot();

  public:
    /// @brief Constructor, references directly used
    FormatSelectionWidget(PropertyDataType data_type, Format& format);
    /// @brief Destructor
    virtual ~FormatSelectionWidget();

    void update(PropertyDataType data_type, Format& format);

  protected:
    PropertyDataType data_type_;
    Format& format_;

    /// Context menu
    QMenu menu_;

    void createMenu();
};

#endif  // FORMATSELECTIONWIDGET_H
