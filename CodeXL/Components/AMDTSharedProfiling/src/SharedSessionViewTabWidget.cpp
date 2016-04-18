
// Local:
#include <inc/SharedSessionViewTabWidget.h>


SharedSessionViewTabWidget::SharedSessionViewTabWidget(QWidget* parent) : QTabWidget(parent)
{
}

void SharedSessionViewTabWidget::tabInserted(int index)
{
    // hide the close button on the first tab
    if (index == 0)
    {
        tabBar()->tabButton(index, QTabBar::RightSide)->hide();
    }
}

void SharedSessionViewTabWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
    // close the tab in response to the middle mouse button click
    int tabIndex = tabBar()->tabAt(mouseEvent->pos());

    if (mouseEvent->button() == Qt::MiddleButton && tabIndex != 0)
    {
        emit tabCloseRequested(tabIndex);
    }
}
