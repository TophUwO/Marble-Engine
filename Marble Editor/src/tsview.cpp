#include <tsview.hpp>


namespace mbe {
    tilesetview::tilesetview(QWidget *cp_parent)
        : tabpage("", cp_parent), QAbstractScrollArea(cp_parent)
    {
        
    }

    bool tilesetview::loadfromfile(tilesetprops const &sr_props) {
        return true;
    }
} /* namespace mbe */


