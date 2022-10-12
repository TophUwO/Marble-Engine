#include <dockwnd.hpp>


namespace mbe {
    dockwindow::dockwindow(QMainWindow *cp_refmainwnd, QString const &cr_title, QWidget *cp_parent) 
        : QDockWidget(cr_title, cp_parent)
    {
        /* Create and initialize child widgets. */
        int_createwidgets();
    }

    dockwindow::~dockwindow() {
    
    }

    void dockwindow::int_createwidgets() {
        /* Create child widgets. */
        mw_main = new QMainWindow(this);
        
        /*
         * Associate widget with content area of
         * the underlying QDockWidget.
         */
        mw_main->setWindowFlags(Qt::Widget);
        setWidget(mw_main);
    }
}


