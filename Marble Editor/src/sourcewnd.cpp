#include <sourcewnd.hpp>


namespace mbe {
    sourcewindow::sourcewindow(QMainWindow *cp_refmainwnd, QWidget *cp_parent)
        : dockwindow(cp_refmainwnd, "Source Window", cp_parent)
    {
        /* Create child widgets. */
        int_createwidgets();
    }

    sourcewindow::~sourcewindow() {
    
    }

    void sourcewindow::int_createwidgets() {
        /* Create tab-widget. */
        mw_tabctrl = new QTabWidget(this);

        /* Set tab-widget properties. */
        mw_tabctrl->setMovable(true);
        mw_tabctrl->setTabsClosable(true);

        /*
         * Associate tab-widget with the QMainWindow of the
         * underlying mbe::dockwindow.
         */
        mw_main->setCentralWidget(mw_tabctrl);
    }
}


