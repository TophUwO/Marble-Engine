#include <dialog.hpp>


namespace mbe {
    namespace dialog {
#pragma region DLG-NEWLEVEL
        newlevel::newlevel(QWidget *cp_parent)
            : QDialog(cp_parent)
        {
            mc_ui.setupUi(this);

            /* Make window non-resizable. */
            setFixedSize(width(), height());

            /* Connect signal handlers. */
            connect(mc_ui.tbtn_browse, SIGNAL(clicked()), SLOT(int_onbrowseclicked()));
            connect(mc_ui.w_btnbox, &QDialogButtonBox::accepted, this, &newlevel::accept);
            connect(mc_ui.w_btnbox, &QDialogButtonBox::rejected, this, &newlevel::reject);
        }

        newlevel::~newlevel() {

        }

        void newlevel::accept() {
            /*
             * If the user left essential fields empty, show a
             * message box asking the user to provide necessary
             * information and return.
             */
            if (mc_ui.ledit_name->text() == "" || mc_ui.ledit_location->text() == "") {
                QMessageBox::warning(
                    this,
                    "Insufficient information",
                    "Fields marked with '*' may not be empty upon accepting the dialog.",
                    QMessageBox::StandardButton::Ok
                );

                return;
            }

            /* Write-back field values. */
            ms_props.mc_name  = mc_ui.ledit_name->text();
            ms_props.mc_cmt   = mc_ui.ledit_cmt->text();
            ms_props.mc_loc   = mc_ui.ledit_location->text();
            ms_props.m_width  = mc_ui.spn_width->value();
            ms_props.m_height = mc_ui.spn_height->value();

            /* End the dialog. */
            QDialog::accept();
        }

        void newlevel::int_onbrowseclicked() {
            /*
             * Let the user select a directory on the disk. Uses
             * system dialog by default.
             */
            QString c_result = QFileDialog::getExistingDirectory(
                this,
                "Choose file location",
                QDir::homePath()
            );

            /* Update field contents. */
            if (!c_result.isNull())
                mc_ui.ledit_location->setText(c_result);
        }
#pragma endregion (DLG-NEWLEVEL)

#pragma region DLG-TSFROMIMG
        tsfromimg::tsfromimg(QWidget *cp_parent)
            : QDialog(cp_parent)
        {
            mc_ui.setupUi(this);

            /*
             * Let the dialog calculate its size and then
             * set its fixed size to the current size,
             * making it unresiazable.
             */
            setFixedSize(width(), height());

            /* Setup controls. */
            mc_ui.spn_tsize->setValue(ms_props.m_tsize);

            /* Connect signals. */
            connect(mc_ui.cb_usepartial, SIGNAL(clicked()), SLOT(int_onpartialClicked()));
            connect(mc_ui.cb_usemask, SIGNAL(clicked()), SLOT(int_onmaskClicked()));
            connect(mc_ui.tbtn_browse, SIGNAL(clicked()), SLOT(int_onbrowseClicked()));
            connect(mc_ui.tbtn_dialog, SIGNAL(clicked()), SLOT(int_onclrdlgClicked()));
            connect(mc_ui.w_btnbox, &QDialogButtonBox::accepted, this, &tsfromimg::accept);
            connect(mc_ui.w_btnbox, &QDialogButtonBox::rejected, this, &tsfromimg::reject);
        }

        /*
         * Overrides the default QDialog's accept method in order
         * to allow a write-back of control states to the custom
         * userdata structure which can then be grabbed by
         * the owner of the dialog.
         * 
         * Returns nothing.
         */
        void tsfromimg::accept() {
            ms_props.m_tsize = mc_ui.spn_tsize->value();
            ms_props.mc_path = mc_ui.ledit_path->text();

            /*
             * Write-back rectangle bounds only if
             * the "load entire image" flag is set.
             */
            if (ms_props.m_ispartial = base::iscbchecked(mc_ui.cb_usepartial)) {
                ms_props.m_left   = mc_ui.spn_left->value();
                ms_props.m_top    = mc_ui.spn_top->value();
                ms_props.m_right  = mc_ui.spn_right->value();
                ms_props.m_bottom = mc_ui.spn_bottom->value();
            }

            /*
             * If the "Mask-out background" flag is set,
             * write-back the values of the choosen background
             * color components.
             */
            if (ms_props.m_ismask = base::iscbchecked(mc_ui.cb_usemask))
                ms_props.mc_mask = QColor(
                    mc_ui.spn_red->value(),
                    mc_ui.spn_green->value(),
                    mc_ui.spn_blue->value(),
                    mc_ui.spn_alpha->value()
                );

            /* End the dialog. */
            QDialog::accept();
        }

        void tsfromimg::int_onclrdlgClicked() {
            /*
             * Open the dialog and let the user choose
             * a color.
             */
            QColor c_color = QColorDialog::getColor(
                QColor(0, 0, 0),
                this,
                "Choose background color",
                QColorDialog::ColorDialogOption::ShowAlphaChannel
            );

            /*
             * If the user cancels the dialog, "getColor()"
             * will return an invalid color.
             */
            if (c_color.isValid()) {
                /*
                 * Update the values of the spin-boxes for each
                 * RGB color channel.
                 */
                mc_ui.spn_red->setValue(c_color.red());
                mc_ui.spn_green->setValue(c_color.green());
                mc_ui.spn_blue->setValue(c_color.blue());
                mc_ui.spn_alpha->setValue(c_color.alpha());
            }
        }

        void tsfromimg::int_onbrowseClicked() {
            /* Execute the "Open file ..." dialog. */
            QString c_result = QFileDialog::getOpenFileName(
                this,
                "Import tileset from image",
                QDir::homePath(),
                "All files (*.*);; "
                "Image files (*.bmp *.jpg *.jpeg *.jfif *.png *.gif);; "
                "Windows Bitmaps (*.bmp);; "
                "Joint Photographic Experts Group (*.jpg);; "
                "Portable Network Graphics (*.png);; "
                "Graphic Interchange Format (*.gif)"
            );

            /*
             * If the dialog was accepted, change the contents
             * of the "URL" edit box.
             */
            if (!c_result.isNull())
                mc_ui.ledit_path->setText(c_result);
        }

        void tsfromimg::int_onpartialClicked() {
            base::setlayoutstate(
                mc_ui.lo_rect,
                base::iscbchecked(mc_ui.cb_usepartial),
                true
            );
        }

        void tsfromimg::int_onmaskClicked() {
            base::setlayoutstate(
                mc_ui.lo_mask,
                base::iscbchecked(mc_ui.cb_usemask),
                true
            );
        }
#pragma endregion (DLG-TSROMIMG)
    } /* namespace dialog */
} /* namespace mbe */


