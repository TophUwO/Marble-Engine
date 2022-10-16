#pragma once

#include <mainwnd.hpp>


namespace mbe {
    mainwindow::mainwindow(QSize const &cr_dims, QWidget *cp_parent)
        : QMainWindow(cp_parent)
    {
        /* Set main window properties. */
        resize(cr_dims.width(), cr_dims.height());
        setWindowIcon(QIcon(":/icons/ico_appmain.ico"));

        /* Create child widgets. */
        int_createmenubar();
        int_createwidgets();
    }
    
    mainwindow::~mainwindow() {
    
    }
    
    void mainwindow::int_createmenubar() {
        /* Create menubar. */
        mw_menubar = new QMenuBar(this);
        
        /* Create and setup sub-menus. */
        QMenu   *cp_mfile      = new QMenu("File", mw_menubar);
        QAction *cp_afileexit  = new QAction("Exit", cp_mfile);
        QAction *cp_afileimpts = new QAction("Import image as tileset", cp_mfile);

        /* Connect action signals. */
        connect(cp_afileimpts, &QAction::triggered, this, &mainwindow::int_onfileimpts);
        
        /* Associate actions with menus. */
        cp_mfile->addAction(cp_afileimpts);
        cp_mfile->addSeparator();
        cp_mfile->addAction(cp_afileexit);
        
        /* Associate menubar with main-window. */
        mw_menubar->addAction(cp_mfile->menuAction());
        
        setMenuBar(mw_menubar);
    }
    
    void mainwindow::int_createwidgets() {
        /* Create child widgets. */
        mw_sourcewnd = new sourcewindow(this);

        /*
         * Add widgets to their respective docking
         * positions.
         */
        addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, mw_sourcewnd);
    }

    void mainwindow::int_onfileimpts() {
        bool res;
        tsfromimgdialog c_dlg;

        if (c_dlg.exec() == QDialog::Accepted) {
            tilesetview *cp_view = new tilesetview(this);

            if (!cp_view->loadfromfile(c_dlg.ms_props)) {
                delete cp_view;

                return;
            }

            /* Add the page to the source-view. */
            mw_sourcewnd->addpage(cp_view);

            /*
             * This will show the docked window if it is
             * currently not visible. It will also restore
             * its original dock-position and make it non-
             * floating.
             */
            int_resetdockwnd(mw_sourcewnd);
        }
    }

    void mainwindow::int_resetdockwnd(dockwindow *cp_wnd) {
        if (cp_wnd == nullptr)
            return;

        if (!cp_wnd->isVisible()) {
            cp_wnd->setVisible(true);
            cp_wnd->setFloating(false);

            addDockWidget(cp_wnd->getdefarea(), cp_wnd);
        }
    }
} /* namespace mbe */


