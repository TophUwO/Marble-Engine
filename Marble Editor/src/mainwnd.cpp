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
        QMenu   *cp_mfile       = new QMenu("File", mw_menubar);
        QMenu   *cp_mfilenew    = new QMenu("New", cp_mfile);
        QAction *cp_afileexit   = new QAction("Exit", cp_mfile);
        QAction *cp_afileimpts  = new QAction("Import image as tileset", cp_mfile);
        QAction *cp_afilenewlvl = new QAction("Level", cp_mfilenew);

        /* Connect action signals. */
        connect(cp_afilenewlvl, &QAction::triggered, this, &mainwindow::int_onfilenewlvl);
        connect(cp_afileimpts, &QAction::triggered, this, &mainwindow::int_onfileimpts);
        connect(cp_afileexit, &QAction::triggered, this, &mainwindow::int_onfileexit);
        
        /* Associate actions with menus. */
        cp_mfile->addAction(cp_mfilenew->menuAction());
        cp_mfile->addAction(cp_afileimpts);
        cp_mfile->addSeparator();
        cp_mfile->addAction(cp_afileexit);
        cp_mfilenew->addAction(cp_afilenewlvl);
        
        /* Associate menubar with main-window. */
        mw_menubar->addAction(cp_mfile->menuAction());
        
        setMenuBar(mw_menubar);
    }
    
    void mainwindow::int_createwidgets() {
        /* Create child widgets. */
        mw_sourcewnd = new sourcewindow(this);
        mw_editwnd   = new editwindow(false, this);

        /*
         * Maintain the widget's size, so that dock windows
         * cannot occupy the entire parent window in case
         * they become visible before the Object View
         * (= central) widget.
         */
        QSizePolicy c_sp = mw_editwnd->sizePolicy();
        c_sp.setRetainSizeWhenHidden(true);
        mw_editwnd->setSizePolicy(c_sp);

        /*
         * Add widgets to their respective (docking)
         * positions.
         */
        setCentralWidget(mw_editwnd);
        addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, mw_sourcewnd);
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

    void mainwindow::int_onfileimpts() {
        bool res;
        tsfromimgdialog c_dlg(this);

        if (c_dlg.exec() == QDialog::Accepted) {
            tilesetview *cp_view = new tilesetview(this);

            /*
             * If the image fails to load, delete the view and
             * do not update the source-window.
             */
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

    void mainwindow::int_onfileexit() {
        QApplication::exit();
    }

    void mainwindow::int_onfilenewlvl() {
        bool res;
        newlvldlg c_dlg(this);

        if (c_dlg.exec() == QDialog::Accepted) {
            
        }
    }
} /* namespace mbe */


