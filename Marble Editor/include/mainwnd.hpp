#pragma once

#include <base.hpp>
#include <sourcewnd.hpp>


/*
 * Everything defined and used by Marble Editor will be inside
 * namespace mbe.
 */
namespace mbe {
    /*
     * Subclass of QMainWindow for creating a custom main window for
     * the editor application.
     * All subsequently created widgets are children of this window.
     */
    class mainwindow : public QMainWindow {
        Q_OBJECT
    
    public:
        mainwindow(QSize const &cr_dims, QWidget *cp_parent = nullptr);
        ~mainwindow();
    
    private:
        /*
         * Creates the menubar and all required sub-menus.
         * 
         * Returns nothing.
         */
        void int_createmenubar();
        /*
         * Creates all child widgets.
         * 
         * Returns nothing.
         */
        void int_createwidgets();

        QMenuBar     *mw_menubar;
        sourcewindow *mw_sourcewnd;
    };
}


