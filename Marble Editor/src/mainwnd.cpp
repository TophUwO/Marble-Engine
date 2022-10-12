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
        QMenu   *cp_mfile     = new QMenu("File", mw_menubar);
        QAction *cp_afileexit = new QAction("Exit", cp_mfile);
        
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
}


