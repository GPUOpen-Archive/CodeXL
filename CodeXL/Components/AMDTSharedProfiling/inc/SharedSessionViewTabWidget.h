
#ifndef _SHAREDSESSIONVIEWTABWIDGET_H_
#define _SHAREDSESSIONVIEWTABWIDGET_H_

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4512)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

/// QTabWidget descendant, which prevents the first tab from being closed and allows the middle button to close closable tabs
class SharedSessionViewTabWidget : public QTabWidget
{
public:

    /// Initializes a new instance of the SessionViewTabWidget class
    /// \param parent the parent object
    SharedSessionViewTabWidget(QWidget* parent = 0);

protected:
    /// Overridden method called when a tab in inserted
    /// \param index the index of the tab inserted
    virtual void tabInserted(int index);

    /// Overridden method called when the mouse is pressed
    /// \param mouseEvent the mouse event parameters
    virtual void mousePressEvent(QMouseEvent* mouseEvent);
};

#endif //_SHAREDSESSIONVIEWTABWIDGET_H_;
