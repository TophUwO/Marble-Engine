#pragma once

#include <base.hpp>
#include <ui_dlg_newlvl.h>


namespace mbe {
    class newlvldlg : public QDialog {
        Q_OBJECT

    public:
        newlvldlg(QWidget *cp_parent = nullptr);
        ~newlvldlg();

    protected:
        virtual void accept() override;

    private slots:
        void int_onbrowseclicked();

    private:
        Ui::mbe_newlvldlg mc_ui;
    };
} /* namespace mbe */


