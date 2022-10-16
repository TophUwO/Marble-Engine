#pragma once

#include <base.hpp>


namespace mbe {
    /*
     * Class representing an abstract dockable window,
     * based on QDockWidget.
     * This class is, like mbe::tabpage, not meant to
     * be used directly, but to be subclassed.
     */
    class dockwindow : public QDockWidget {
        Q_OBJECT

    public:
        dockwindow() = delete;
        explicit dockwindow(
            QMainWindow *cp_refmainwnd, /* pointer to main window */
            QString const &cr_title,    /* titlebar string */
            Qt::DockWidgetArea defarea, /* default docking area */
            bool isvisible = false,     /* initially visible flag */
            QWidget *cp_parent = nullptr
        );
        virtual ~dockwindow();

        Qt::DockWidgetArea getdefarea() const { return m_defarea; }

    protected:
        QMainWindow *mw_refmainwnd;
        /*
         * Having a QMainWindow as the child of the
         * QDockWidget allows us to natively add menu-
         * and toolbars to the dock widget.
         * 
         * "int_createwidgets()" will setup the needed
         * infrastructure for this to work.
         */
        QMainWindow *mw_main;
        /*
         * Handle to the toolbar; this member is nullptr
         * if there is no toolbar.
         */
        QToolBar *mw_tbar;

    private:
        /*
         * Creates and initializes child widgets.
         * 
         * Returns nothing.
         */
        void int_createwidgets();

        Qt::DockWidgetArea m_defarea;
    };
} /* namespace mbe */


