#pragma once

#include <mainwnd.hpp>


namespace mbe {
    mainwindow::mainwindow(QSize const &cr_dims, QWidget *cp_parent)
        : QMainWindow(cp_parent)
    {
        /* Init srcid-provider. */
        m_srcidprovider.store(0);

        /* Init appstate. */
        mbe::base::gl_appstate = {
            mbe::base::appstate::running,
            0
        };

        /* Set main window properties. */
        resize(cr_dims.width(), cr_dims.height());
        setWindowIcon(QIcon(":/icons/ico_appmain.ico"));

        /* Create child widgets. */
        int_createmenubar();
        int_createwidgets();

        /*
         * Create assetman.
         * If this fails, show a messagebox and exit
         * the app.
         */
        if (marble_assetman_create(&mps_assetman) != MARBLE_EC_OK) {
            QMessageBox::critical(
                this,
                "Fatal Error",
                "Could not initialize internal asset-manager."
            );

            throw std::bad_alloc();
        }
    }
    
    mainwindow::~mainwindow() {
        /* Set app-state. */
        mbe::base::gl_appstate = {
            mbe::base::appstate::quit,
            0
        };

        marble_assetman_destroy(&mps_assetman);
    }
    
    void mainwindow::int_createmenubar() {
        /* Create menubar. */
        mw_menubar = new QMenuBar(this);
        
        /* Create and setup sub-menus. */
        QMenu   *cp_mfile       = new QMenu("File", mw_menubar);
        QMenu   *cp_mfilenew    = new QMenu("New", cp_mfile);
        QMenu   *cp_mfileimp    = new QMenu("Import", cp_mfile);
        QAction *cp_afileexit   = new QAction("Exit", cp_mfile);
        QAction *cp_afileimpts  = new QAction("Tileset", cp_mfileimp);
        QAction *cp_afilenewlvl = new QAction("Level", cp_mfilenew);

        /* Connect action signals. */
        connect(cp_afilenewlvl, &QAction::triggered, this, &mainwindow::int_onfilenewlvl);
        connect(cp_afileimpts, &QAction::triggered, this, &mainwindow::int_onfileimpts);
        connect(cp_afileexit, &QAction::triggered, this, &mainwindow::int_onfileexit);
        
        /* Associate actions with menus. */
        cp_mfile->addAction(cp_mfilenew->menuAction());
        cp_mfile->addAction(cp_mfileimp->menuAction());
        cp_mfile->addSeparator();
        cp_mfile->addAction(cp_afileexit);
        cp_mfilenew->addAction(cp_afilenewlvl);
        cp_mfileimp->addAction(cp_afileimpts);
        
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
        dialog::importts c_dlg(this);

        if (c_dlg.exec() == QDialog::Accepted) {
            tilesetview *cp_tsview = new tilesetview(int_getnextsrcid(), this);

            /*
             * If the image fails to load, delete the view and
             * do not update the source-window.
             */
            if (!cp_tsview->loadfromfile(c_dlg.ms_props)) {
                cp_tsview->deleteLater();

                return;
            }

            /* Add the page to the source-view. */
            mw_sourcewnd->addpage(cp_tsview);

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
        dialog::newlevel c_dlg(this);

        if (c_dlg.exec() == QDialog::Accepted) {
            levelview *cp_lvlview = new levelview(int_getnextsrcid(), mps_assetman, mw_editwnd);

            /*
             * Attempt to create a new level from scratch; if this
             * fails, do not proceed and destroy the page.
             */
            if (!cp_lvlview->newlevel(c_dlg.ms_props)) {
                cp_lvlview->deleteLater();

                return;
            }

            /*
             * Make Edit-Window visible if it is currently hidden
             * and add the page to the widget.
             */
            mw_editwnd->setVisible(true);
            mw_editwnd->addTab(cp_lvlview, cp_lvlview->gettitle());
        }
    }
} /* namespace mbe */


