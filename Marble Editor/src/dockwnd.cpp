#include <dockwnd.hpp>


namespace mbe {
    dockwindow::dockwindow(QMainWindow *cp_refmainwnd, QString const &cr_title, Qt::DockWidgetArea defarea, bool isvisible, QWidget *cp_parent) 
        : QDockWidget(cr_title, cp_parent), m_defarea(defarea)
    {
        /* Create and initialize child widgets. */
        int_createwidgets();

        /* Set properties. */
        setVisible(isvisible);
    }

    dockwindow::~dockwindow() {
    
    }

    void dockwindow::int_createwidgets() {
        /* Create child widgets. */
        mw_main = new QMainWindow(this);
        mw_main->setWindowFlags(Qt::Widget);

        /*
         * Associate widget with content area of
         * the underlying QDockWidget.
         */
        setWidget(mw_main);
    }
} /* namespace mbe */


