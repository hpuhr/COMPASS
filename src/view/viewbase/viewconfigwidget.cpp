
#include "viewconfigwidget.h"

/**
*/
void ViewConfigWidget::loadingStarted()
{
    setDisabled(true);
}

/**
*/
void ViewConfigWidget::loadingDone()
{
    setDisabled(false);
}
