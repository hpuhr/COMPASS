#ifndef DATATYPEFORMATSELECTIONWIDGET_H
#define DATATYPEFORMATSELECTIONWIDGET_H

#include "format.h"
#include "property.h"

#include <QPushButton>
#include <QMenu>

/**
 * @brief Sets a Unit using a context menu
 */
class DataTypeFormatSelectionWidget : public QPushButton
{
    Q_OBJECT

protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot (QAction* action);
    /// @brief Shows the context menu
    void showMenuSlot();

public:
    /// @brief Constructor, references directly used
    DataTypeFormatSelectionWidget (std::string& data_type_str, Format& format);
    /// @brief Destructor
    virtual ~DataTypeFormatSelectionWidget();

protected:
    std::string& data_type_str_;
    Format& format_;

    /// Context menu
    QMenu menu_;

    void showValues();
    void createMenu();
};


#endif // DATATYPEFORMATSELECTIONWIDGET_H
