#include <tab.hpp>


namespace mbe {
    tabpage::tabpage(QString const &cr_title, QWidget *cp_parent)
        : QWidget(cp_parent), c_title(cr_title)
    {
    
    }

    tabpage::~tabpage() {
    
    }

    void tabpage::refreshcontents() {
        /* By default, this function does nothing. */
    }

    bool tabpage::closepage() {
        /*
         * Allow closing the tab-page by
         * default.
         */
        return true;
    }
}


