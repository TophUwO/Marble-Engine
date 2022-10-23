#include <editwnd.hpp>
#include <tab.hpp>


namespace mbe {
    editwindow::editwindow(bool visible, QWidget *cp_parent)
        : QTabWidget(cp_parent)
    {
        /* Set properties. */
        setTabsClosable(true);
        setMovable(true);
        setVisible(visible);

        /* Connect handlers. */
        connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(int_closepage(int)));
    }

    editwindow::~editwindow() {
    
    }

    void editwindow::int_closepage(int index) {
        base::closetpage(this, index);
    }
} /* namespace mbe */


