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


        QDialog::accept();
    }

    void newlvldlg::int_onbrowseclicked() {
        
    }
} /* namespace mbe */


