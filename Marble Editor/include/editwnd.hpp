#include <base.hpp>


namespace mbe {
    /*
     * Main edit window. Acts as the central widget of the main window
     * and as a container for regular tab-pages.
     */
    class editwindow : public QTabWidget {
        Q_OBJECT

    public:
        explicit editwindow(bool visible, QWidget *cp_parent = nullptr);
        ~editwindow();

    private slots:
        /*
         * Attempts to close a tabpage by calling its "closepage" function.
         * If the call succeeds, the page gets removed from the tab-widget
         * and subsequently deleted.
         * 
         * Returns nothing.
         */
        void int_closepage(int index);
    };
} /* namespace mbe */


