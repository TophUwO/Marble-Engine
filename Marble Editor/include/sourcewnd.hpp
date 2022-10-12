#pragma once

#include <base.hpp>
#include <dockwnd.hpp>


namespace mbe {
    /*
     * Class representing the source-view window. Will be
     * placed on the left side of the main window by default
     * and will hold a view to viewable assets that have
     * been imported into the editor. 
     */
    class sourcewindow : public dockwindow {
        Q_OBJECT

    public:
        sourcewindow() = delete;
        explicit sourcewindow(
            QMainWindow *cp_refmainwnd, /* pointer to main window */
            QWidget *cp_parent = nullptr
        );
        ~sourcewindow();

    private:
        /* 
         * Creates tab-widget.
         * 
         * Returns nothing.
         */
        void int_createwidgets();

        /*
         * The central widget will be a QTabWidget to
         * which we can add pages in the form of
         * mbe::tabpage.
         */
        QTabWidget *mw_tabctrl;
    };
}


