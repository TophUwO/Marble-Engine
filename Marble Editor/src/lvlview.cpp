#include <lvlview.hpp>


namespace mbe {
    levelview::levelview(editoraman *ps_aman, QWidget *cp_parent)
        : QAbstractScrollArea(cp_parent), tabpage(this, ""), mps_refaman(ps_aman)
    {
        mps_level =  NULL;
    }

    levelview::~levelview() {
        delete mps_level;
    }

    bool levelview::newlevel(dialog::levelprops const &sr_props) {
        mps_level = new editorlevel;

        /* Set tab-page title. */
        settitle(sr_props.mc_name);

        return 
            mps_level->create(
                sr_props.m_width,
                sr_props.m_height,
                sr_props.mc_loc,
                sr_props.mc_name,
                mps_refaman
            );
    }

    void levelview::paintEvent(QPaintEvent *cp_event) {
        QAbstractScrollArea::paintEvent(cp_event); /* base class event handler */
        QPainter c_painter(viewport());

        /*
         * Calculate the origin of the current viewport,
         * that is, the top-left corner of the top-left
         * tile that is at least partly visible in the
         * current viewport.
         */
        int const orix = -horizontalScrollBar()->value() % 32;
        int const oriy = -verticalScrollBar()->value() % 32;
        int tx = horizontalScrollBar()->value() / 32;
        int ty = verticalScrollBar()->value() / 32;
        /*
         * Calculate the dimensions of the area occupied
         * by the contents. This area can be either
         * smaller or as large as the viewport.
         */
        int const xdim = min(mps_level->m_width, viewport()->width());
        int const ydim = min(mps_level->m_height, viewport()->height());

        /*
         * If the entire map is smaller than the viewport,
         * clear the background of the viewport with a dark
         * color.
         */
        if (int_iscontentssmaller())
            c_painter.fillRect(
                0,
                0,
                viewport()->width(),
                viewport()->height(),
                QColor(33, 33, 33)
            );

        /* Clear the background of the contents area. */
        c_painter.fillRect(
            0,
            0,
            mps_level->m_width,
            mps_level->m_height,
            QColor(255, 255, 255)
        );

        /* Draw grid. */

        /* Draw vertical grid-lines. */
        for (int x = orix; x < xdim; x += 32, tx++) {
            if (x <= 0)
                continue;

            if (tx % 16 == 0) {
                c_painter.setPen(QPen(QColor(55, 55, 55, 128), 3));
            } else {
                c_painter.setPen(QPen(QColor(55, 55, 55, 128), 1));
            }

            c_painter.drawLine(
                QPoint(x, 0),
                QPoint(x, ydim)
            );
        }

        /* Draw horizontal grid-lines. */
        for (int y = oriy; y < ydim; y += 32, ty++) {
            if (y <= 0)
                continue;

            if (ty % 16 == 0) {
                c_painter.setPen(QPen(QColor(55, 55, 55, 128), 3));
            } else {
                c_painter.setPen(QPen(QColor(55, 55, 55, 128), 1));
            }

            c_painter.drawLine(
                QPoint(0, y),
                QPoint(xdim, y)
            );
        }
    }

    void levelview::mousePressEvent(QMouseEvent *cp_event) {
    
    }

    void levelview::resizeEvent(QResizeEvent *cp_event) {
        QAbstractScrollArea::resizeEvent(cp_event); /* base class event handler */

        int_updatescrollbars();
    }
    
    void levelview::int_updatescrollbars() {
        QScrollBar *cp_horz = horizontalScrollBar();
        QScrollBar *cp_vert = verticalScrollBar();

        /* Update horizontal scrollbar. */
        cp_horz->setSingleStep(32);
        cp_horz->setRange(
            0,
            (viewport()->width() < mps_level->m_width
                ? mps_level->m_width - viewport()->width()
                : 0
            )
        );
        cp_horz->setPageStep(viewport()->size().width());

        /* Update vertical scrollbar. */
        cp_vert->setSingleStep(32);
        cp_vert->setRange(
            0,
            (viewport()->height() < mps_level->m_height
                ? mps_level->m_height - viewport()->height()
                : 0
            )
        );
        cp_vert->setPageStep(viewport()->size().height());
    }

    bool levelview::int_iscontentssmaller() {
        return
            mps_level->m_width < viewport()->width()
            || mps_level->m_height < viewport()->height()
        ;
    }
} /* namespace mbe */


