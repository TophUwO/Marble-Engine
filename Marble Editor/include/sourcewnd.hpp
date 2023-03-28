#pragma once

#include <base.hpp>
#include <dockwnd.hpp>
#include <tsview.hpp>


namespace mbe {
    /*
     * Class holding selection information for the Source
     * Window.
     */
    class sourcewndsel {
    public:
        sourcewndsel() = delete;
        sourcewndsel(int32_t srcid) : m_srcid(srcid) { }
        sourcewndsel(int32_t srcid, std::initializer_list<QPoint> &cr_points) : m_srcid(srcid) {
            addpoints(cr_points);
        }
        ~sourcewndsel() { }

        /*
         * Adds a single point to the selection.
         *
         * Returns nothing.
         */
        void addpoint(QPoint &cr_point) {
            mc_buffer.push_back(cr_point);
        }

        /*
         * Adds multiple points to the selection.
         * 
         * Returns nothing.
         */
        void addpoints(std::initializer_list<QPoint> &cr_points) {
            mc_buffer.insert(mc_buffer.end(), cr_points);
        }

    private:
        int32_t m_srcid;

        std::vector<QPoint> mc_buffer;
    };


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

        /*
         * Adds a selection to the selection buffer.
         * 
         * Returns nothing.
         */
        void addsel(
            mbe::sourcewndsel const &cr_sel /* selection to add */
        );

    private:
        /* 
         * Creates the child widgets of the source window.
         * 
         * Returns nothing.
         */
        void int_createwidgets();
        /*
         * Creates the Source Window tool-bar.
         * 
         * Returns nothing.
         */
        void int_createtoolbar();

        /*
         * The central widget will be a QTabWidget to
         * which we can add pages in the form of
         * mbe::tabpage.
         */
        QTabWidget *mw_tabctrl;
        /*
         * Toolbars for quick access to tools concerning
         * source assets
         */
        QToolBar *mw_maintbar;
        QToolBar *mw_seltbar;

        /*
         * Actions for the source-window toolbar. 
         */
        struct {
            QAction *mcp_newdocact;
            QAction *mcp_opendocact;
            QAction *mcp_savedocact;
            QAction *mcp_saveallact;
            QAction *mcp_selforward;
            QAction *mcp_selbackward;
            QAction *mcp_multisel;
            QAction *mcp_bordersel;
            QAction *mcp_tilesel;
            QAction *mcp_rectsel;
            QAction *mcp_ellipsesel;
            QAction *mcp_rectpathsel;
            QAction *mcp_pathsel;

            QActionGroup *mcp_toggleactions;
        } ms_actions;

        /* undo-redo buffer, specialized for selections. */
        mbe::base::undoredobuf<mbe::sourcewndsel> mc_urbuf;

    private slots:
        /*
         * Attempts to close a tabpage by calling its "closepage" function.
         * If the call succeeds, the page gets removed from the tab-widget
         * and subsequently deleted.
         * 
         * Returns nothing.
         */
        void int_closepage(int index);
        /*
         * Updates the source-window's toolbar (i.e. action states, etc.).
         * 
         * Returns nothing.
         */
        void int_updatetoolbaractions();
    };
} /* namespace mbe */


