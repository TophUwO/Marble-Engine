#pragma once

#include <base.hpp>
#include <tsview.hpp>
#include <ui_dlg_tsfromimg.h>


namespace mbe {
    /*
     * Implements dialog used for opening a tileset
     * from an ordinary image file.
     */
    class tsfromimgdialog : public QDialog {
        Q_OBJECT

    public:
        tsfromimgdialog(QWidget *cp_parent = nullptr);

        tilesetprops ms_props;

    protected:
        virtual void accept() override;

    private slots:
        /*
         * Executed when the user clicks the "Dialog >>" button
         * to select an RGBA color from the system dialog.
         * 
         * Returns nothing.
         */
        void int_onclrdlgClicked();
        /*
         * Exeuted when the user clicks the "..." button to
         * select a file using the system-provided "Open ..."
         * dialog.
         * 
         * Returns nothing.
         */
        void int_onbrowseClicked();
        /*
         * Executed when the user changes the state of the "Use
         * partial image" button.
         * 
         * Returns nothing.
         */
        void int_onpartialClicked();
        /*
         * Executed when the user changes the state of the "Use background
         * color mask" button.
         * 
         * Returns nothing.
         */
        void int_onmaskClicked();

    private:
        Ui::mbe_tsfromimgdlg mc_ui;
    };
} /* namespace mbe */


