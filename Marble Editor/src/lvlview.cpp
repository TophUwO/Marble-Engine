#include <lvlview.hpp>


namespace mbe {
    levelview::levelview(editoraman *ps_aman, QWidget *cp_parent)
        : QAbstractScrollArea(cp_parent), tabpage(this, ""), mps_refaman(ps_aman)
    {

    }

    levelview::~levelview() {
    
    }

    bool levelview::newlevel(dialog::levelprops const &sr_props) {
        return 
            ms_level.create(
                sr_props.m_width,
                sr_props.m_height,
                sr_props.mc_loc,
                sr_props.mc_name,
                mps_refaman
            );
    }

    void levelview::paintEvent(QPaintEvent *cp_event) {
    
    }

    void levelview::mousePressEvent(QMouseEvent *cp_event) {
    
    }

    void levelview::resizeEvent(QResizeEvent *cp_event) {
    
    }
} /* namespace mbe */


