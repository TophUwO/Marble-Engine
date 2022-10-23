#include <sourcewnd.hpp>


namespace mbe {
    sourcewindow::sourcewindow(QMainWindow *cp_refmainwnd, bool isvisible, QWidget *cp_parent)
        : dockwindow(
            cp_refmainwnd,
            "Source Window",
            Qt::DockWidgetArea::LeftDockWidgetArea,
            isvisible,
            cp_parent
        )
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

        /* Connect signal handlers. */
        connect(mw_tabctrl, SIGNAL(tabCloseRequested(int)), this, SLOT(int_closepage(int)));

        /*
         * Associate tab-widget with the QMainWindow of the
         * underlying mbe::dockwindow.
         */
        mw_main->setCentralWidget(mw_tabctrl);
    }

    void sourcewindow::addpage(tabpage *cp_tpage, int index) {
        if (index == -1)
            index = mw_tabctrl->count();

        mw_tabctrl->insertTab(index, cp_tpage->getwidget(), cp_tpage->gettitle());
    }

    void sourcewindow::int_closepage(int index) {
        base::closetpage(mw_tabctrl, index);
    }
} /* namespace mbe */


