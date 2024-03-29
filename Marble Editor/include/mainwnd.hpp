#pragma once

#include <base.hpp>
#include <sourcewnd.hpp>
#include <lvlview.hpp>
#include <editwnd.hpp>
#include <dialog.hpp>


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
        /*
         * Shows the dock window docked in its default
         * docking position.
         * 
         * Returns nothing.
         */
        void int_resetdockwnd(dockwindow *cp_wnd);

        QMenuBar     *mw_menubar;
        sourcewindow *mw_sourcewnd;
        editwindow   *mw_editwnd;
        editoraman   *mps_assetman;
        
        /* Provides srcids for all widgets */
        std::atomic_int32_t m_srcidprovider;

        int32_t int_getnextsrcid() {
            int32_t ret = m_srcidprovider.load();

            ++m_srcidprovider;
            return ret;
        }

    private slots:
        void int_onfilenewlvl();
        void int_onfileimpts();
        void int_onfileexit();
    };
} /* namespace mbe */


