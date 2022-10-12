#pragma once

#include <base.hpp>


namespace mbe {
    /*
     * Class representing a basis for a tab-page, that is
     * the widget that is added to a QTabWidget. This allows
     * for custom operations depending on the exact content
     * that is displayed in the widget.
     * 
     * This class should not be used directly, but subclassed
     * to specify more complex behavior.
     */
    class tabpage : public QWidget {
        Q_OBJECT

    public:
        tabpage() = delete;
        explicit tabpage(QString const &cr_title, QWidget *cp_parent = nullptr);
        virtual ~tabpage();

    protected:
        /*
         * Reimplement this function if you wish to invoke
         * custom behavior when the parent receives the
         * command to refresh the tab-page's contents.
         * 
         * Returns nothing.
         */
        virtual void refreshcontents();
        /*
         * Reimplement this function if you wish to invoke
         * custom behavior right before the tab-page is closed.
         * You can return false if you wish to block the page
         * from being closed.
         * 
         * Returns nothing.
         */
        virtual bool closepage();

    private:
        QString c_title;
    };
}


