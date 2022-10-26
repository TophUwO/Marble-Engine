#pragma once

#include <base.hpp>

#include <ui_dlg_newlevel.h>
#include <ui_dlg_importts.h>


namespace mbe {
    namespace dialog {
#pragma region DLG-NEWLEVEL
        /*
         * properties of a new level, created
         * from scratch
         */
        struct levelprops {
            explicit levelprops()
                : m_width(0), m_height(0),
                  mc_loc(""), mc_cmt(""),
                  mc_name("")
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
        class newlevel : public QDialog {
            Q_OBJECT

        public:
            explicit newlevel(QWidget *cp_parent = nullptr);
            ~newlevel();

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
            Ui::mbe_newlevel mc_ui;
        };
#pragma endregion (DLG-NEWLEVEL)

#pragma region DLG-IMPTS
        /*
         * Properties of a tileset represented by an
         * ordinary image file.
         */
        struct tilesetprops {
            tilesetprops()
                : m_ispartial(false), m_ismask(false),
                  m_tsize(16), m_left(0), m_right(0),
                  m_top(0), m_bottom(0),
                  mc_path(""), mc_mask(Qt::black)
            { }

            /*
             * specifies whether the **m_*** fields specifying a
             * partial bitmap are valid
             */
            bool m_ispartial;
            /* specifies whether the **mc_mask** field is valid */
            bool m_ismask;

            int m_tsize;  /* width/height of a tile, in source pixels */
            int m_left;   /* left margin, in tiles */
            int m_top;    /* top margin, in tiles */
            int m_right;  /* right margin, in tiles */
            int m_bottom; /* bottom margin, in tiles */

            QString mc_path; /* file path */
            QColor  mc_mask; /* background color mask */
        };


        /*
         * Implements dialog used for opening a tileset
         * from an ordinary image file.
         */
        class importts : public QDialog {
            Q_OBJECT

        public:
            importts(QWidget *cp_parent = nullptr);

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
            Ui::mbe_importts mc_ui;
        };
#pragma endregion (DLG-IMPTS)
    } /* namespace dialogs */
} /* namespace mbe */