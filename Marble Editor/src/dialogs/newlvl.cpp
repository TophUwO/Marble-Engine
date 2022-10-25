#include <dialogs/newlvl.hpp>


namespace mbe {
    newlvldlg::newlvldlg(QWidget *cp_parent)
        : QDialog(cp_parent)
    {
        mc_ui.setupUi(this);

        /* Make window non-resizable. */
        setFixedSize(width(), height());

        /* Connect signal handlers. */
        connect(mc_ui.tbtn_browse, SIGNAL(clicked()), SLOT(int_onbrowseclicked()));
        connect(mc_ui.w_btnbox, &QDialogButtonBox::accepted, this, &newlvldlg::accept);
        connect(mc_ui.w_btnbox, &QDialogButtonBox::rejected, this, &newlvldlg::reject);
    }

    newlvldlg::~newlvldlg() {
    
    }

    void newlvldlg::accept() {
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

    void newlvldlg::int_onbrowseclicked() {
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
} /* namespace mbe */


