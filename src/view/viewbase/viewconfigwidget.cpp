
#include "viewconfigwidget.h"

#include <iostream>

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

/**
*/
void ViewConfigWidget::redrawStarted()
{
    setDisabled(true);
}

/**
*/
void ViewConfigWidget::redrawDone()
{
    setDisabled(false);
}
