#pragma once

#include <base.hpp>
#include <dockwnd.hpp>
#include <tsview.hpp>


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
            bool isvisible = false,     /* initially visible flag */
            QWidget *cp_parent = nullptr
        );
        ~sourcewindow();

        /*
         * Adds a tab-page to the underlying QTabWidget at
         * position **index**. By default, the pages are
         * appended, i.e. added at the end of the page-list.
         * 
         * Returns nothing.
         */
        void addpage(
            tabpage *cp_tpage, /* tab-page to add */
            int index = -1     /* insertion index */
        );

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

    private slots:
        /*
         * Attempts to close a tabpage by calling its "closepage" function.
         * If the call succeeds, the page gets removed from the tab-widget
         * and subsequently deleted.
         * 
         * Returns nothing.
         */
        void int_closepage(int index);
    };
} /* namespace mbe */


