#pragma once

#include <base.hpp>
#include <ui_dlg_newlvl.h>


namespace mbe {
    /*
     * properties of a new level, created
     * from scratch
     */
    struct levelprops {
        explicit levelprops()
            : m_width(0), m_height(0),
              mc_loc(""), mc_cmt(""), mc_name("")
        { }

        int     m_width;  /* width of the level, in tiles */
        int     m_height; /* height of the level, in tiles */
        QString mc_name;  /* name (max. 32 characters) */
        QString mc_cmt;   /* optional comment (max. 256 characters) */
        QString mc_loc;   /* location (folder name on disk) */
    };


    /*
     * Implements a dialog that allows the user to create an empty
     * level asset using width and height metrics.
     */
    class newlvldlg : public QDialog {
        Q_OBJECT

    public:
        explicit newlvldlg(QWidget *cp_parent = nullptr);
        ~newlvldlg();

        levelprops ms_props;

    protected:
        virtual void accept() override;

    private slots:
        /*
         * Executed when the user selects the "..." button
         * next to the "Location" edit box.
         * If the function succeeds, the text of aforementioned
         * edit-box will be updated to reflect the currently
         * selected location.
         * 
         * Returns nothing.
         */
        void int_onbrowseclicked();

    private:
        Ui::mbe_newlvldlg mc_ui;
    };
} /* namespace mbe */


