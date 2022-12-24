#pragma once

#include <base.hpp>


namespace mbe {
    /*
     * Class representing the basis for a tab-page, that is
     * the widget that is added to a QTabWidget. This allows
     * for custom operations depending on the exact content
     * that is displayed in the widget.
     * 
     * This class should not be used directly, but subclassed
     * to specify more complex behavior.
     */
    class tabpage {
    public:
        explicit tabpage(QWidget *cp_widget, QString const &cr_title = QString())
            : mc_title(cr_title), mw_refwidget(cp_widget)
        { }
        virtual ~tabpage() { }

        QString const &gettitle() const { return mc_title; }
        QWidget *getwidget() const { return mw_refwidget; }
        void settitle(QString const &cr_title) { mc_title = cr_title; }
        void setwidget(QWidget *cp_widget) { mw_refwidget = cp_widget; }

        /*
         * Reimplement this function if you wish to invoke
         * custom behavior when the parent receives the
         * command to refresh the tab-page's contents.
         * 
         * Returns nothing.
         */
        virtual void refreshcontents() { }
        /*
         * Reimplement this function if you wish to invoke
         * custom behavior right before the tab-page is closed.
         * You can return false if you wish to block the page
         * from being closed.
         * 
         * Returns nothing.
         */
        virtual bool closepage() {
            /*
             * By default, tab-pages can always
             * be closed.
             */
            return true;
        }

    private:
        QString  mc_title;
        QWidget *mw_refwidget;
    };


    namespace base {
        /*
         * Attempts to close a tab-page by executing its
         * "closepage" function. If the function returns
         * true, the page will be removed from **cp_twidget**,
         * and subsequently destroyed.
         *
         * Returns nothing.
         */ 
        inline void closetpage(QTabWidget *cp_twidget, int index) {
            QWidget *cp_widget = cp_twidget->widget(index);

            /*
             * Convert to "tabpage" so we can its "closepage()"
             * handler function.
             */
            tabpage *cp_tpage = dynamic_cast<tabpage *>(cp_widget);
            if (cp_tpage == nullptr)
                return;

            /*
             * The "closepage" function allows the user to block
             * this function from closing the page, i.e. by
             * returning false. 
             */
            if (cp_tpage->closepage()) {
                cp_twidget->removeTab(index);

                cp_widget->deleteLater();
            }
        }
    } /* namespace base */
} /* namespace mbe */


