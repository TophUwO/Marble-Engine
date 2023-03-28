#include <tsview.hpp>
#include <sourcewnd.hpp>


namespace mbe {
    /*
     * Size of a tile on the screen, in pixels. This value is fixed for now,
     * will later be scaled by zoom factors and DPI settings.
     */
    static constexpr int gl_viewtsize = 32;

    tilesetview::tilesetview(int32_t srcid, QWidget *cp_parent)
        : QAbstractScrollArea(cp_parent), tabpage(this, srcid, ""),
          mcp_image(nullptr), mc_path("")
    {
        m_tsize = m_usablewidth = m_usableheight = 0;
    }

    tilesetview::~tilesetview() {
        unloadimage();
    }

    void tilesetview::paintEvent(QPaintEvent *cp_event) {
        QAbstractScrollArea::paintEvent(cp_event); /* base class event handler */

#pragma region PAINT-PREPARE
        QPainter c_painter(viewport());

        /*
         * Calculate the ratio between the physical tile-size
         * and the logical (screen) tile-size, used to locate
         * and scale the tiles in the source image.
         */
        float const scale = float(gl_viewtsize) / m_tsize;
        /*
         * Calculate the origin of the current viewport,
         * that is, the top-left corner of the top-left
         * tile that is at least partly visible in the
         * current viewport.
         */
        int const orix = -horizontalScrollBar()->value() % gl_viewtsize;
        int const oriy = -verticalScrollBar()->value() % gl_viewtsize;
        /*
         * Calculate the dimensions of the area occupied
         * by the contents. This area can be either
         * smaller or as large as the viewport.
         */
        int const xdim = min(int(m_usablewidth * scale), viewport()->width());
        int const ydim = min(int(m_usableheight * scale), viewport()->height());
#pragma endregion (PAINT-PREPARE)

#pragma region PAINT-RENDER
        /*
         * Only clear the background with a darker color
         * if the contents of the tileset are smaller
         * than the currently visible viewport.
         */
        if (int_iscontentssmaller())
            c_painter.fillRect(
                0,
                0,
                viewport()->width(),
                viewport()->height(),
                QColor(255, 255, 255)
            );

        /* Clear the background of the contents area. */
        c_painter.fillRect(
            0,
            0,
            int(m_usablewidth * scale),
            int(m_usableheight * scale),
            QColor(255, 255, 255)
        );

        /*
         * Use bitmap interpolation mode "nearest neighbor". This is normally
         * the default one, we explicity set it, however, to account for
         * platforms that use a different mode by default.
         */
        c_painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

        /*
         * Draw current tileset view. We draw every tile one by one to avoid
         * scaling issues that may arise from interpolation. This way, tile
         * edges always align with the grid.
         */
        for (int x = orix, tsx = horizontalScrollBar()->value() / gl_viewtsize * m_tsize; x < xdim; x += gl_viewtsize, tsx += m_tsize)
            for (int y = oriy, tsy = verticalScrollBar()->value() / gl_viewtsize * m_tsize; y < ydim; y += gl_viewtsize, tsy += m_tsize)
                c_painter.drawPixmap(
                    x,
                    y,
                    gl_viewtsize,
                    gl_viewtsize,
                    *mcp_image,
                    tsx,
                    tsy,
                    m_tsize,
                    m_tsize
                );

        /* Set color of grid lines. */
        c_painter.setPen(QColor(55, 55, 55, 128));

        /* Draw vertical grid-lines. */
        for (int x = orix + gl_viewtsize; x < xdim; x += gl_viewtsize)
            c_painter.drawLine(
                QPoint(x, 0),
                QPoint(x, ydim - 1)
            );

        /* Draw horizontal grid-lines. */
        for (int y = oriy + gl_viewtsize; y < ydim; y += gl_viewtsize)
            c_painter.drawLine(
                QPoint(0, y),
                QPoint(xdim - 1, y)
            );
#pragma endregion (PAINT-RENDER)
    }

    void tilesetview::mousePressEvent(QMouseEvent *cp_event) {
        switch (cp_event->button()) {
            case Qt::MouseButton::LeftButton:
                mc_selstart = cp_event->position().toPoint();

                m_issel = true;
                break;
        }
    }

    void tilesetview::mouseReleaseEvent(QMouseEvent *cp_event) {
        switch (cp_event->button()) {
            case Qt::MouseButton::LeftButton:
                mc_selend = cp_event->position().toPoint();
                
                m_issel = false;
                break;
        }

        int_calcselarea();
    }

    void tilesetview::resizeEvent(QResizeEvent *cp_event) {
        QAbstractScrollArea::resizeEvent(cp_event); /* base class event handler */

        int_updatescrollbars();
    }

    bool tilesetview::loadfromfile(dialog::tilesetprops const &sr_props) {
        QImageReader c_reader(sr_props.mc_path);

        /*
         * Check whether the path can be opened as an image.
         * This can fail when
         *  (1) the file is not an image
         *  (2) the file path does not exist
         *  (3) the device is unavailable
         */
        if (!c_reader.canRead()) {
            std::cout <<
                "ERROR: "
                "Could not read file \"" + sr_props.mc_path.toStdString() + "\". "
                "The file does not exist or the image format is not supported."
            << std::endl;

            return false;
        }

        /*
         * Calculate image dimensions, based on the margins. The usable size
         * of a tileset is the largest size that only consists of full tiles.
         * If the tileset has additional pixels in either dimension, these
         * would form a partial tile which cannot be used by the tileset view.
         */
        m_tsize        = sr_props.m_tsize;
        m_usablewidth  = c_reader.size().width() / m_tsize * m_tsize;
        m_usableheight = c_reader.size().height() / m_tsize * m_tsize;

        /*
         * Display a warning in case the tileset width or height (in tiles)
         * differ from the truncated tile-size, resulting in some rows
         * and/or columns of tiles being cut-away from the asset.
         * 
         * This has to be done as it does not make sense to select or use
         * partial tiles.
         */
        if (   m_usablewidth != c_reader.size().width()
            || m_usableheight != c_reader.size().height()
        ) {
            std::cout <<
                "WARNING: "
                "The size of the image does not align with the entered tile-size. "
                "Bottom/right rows/columns of tiles may not be "
                "part of the tileset view."
            << std::endl;
        }

        /*
         * Calculate bitmap margins and update QImageReader
         * clip rectangle.
         */
        if (sr_props.m_ispartial) {
            /* Calculate margins, in pixels. */
            int const pleft   = sr_props.m_left * m_tsize;
            int const pright  = sr_props.m_right * m_tsize;
            int const ptop    = sr_props.m_top * m_tsize;
            int const pbottom = sr_props.m_bottom * m_tsize;

            /*
             * If the left/top margin extends beyond the right/bottom margin,
             * display a warning, but continue to use the entire (valid) image
             * instead.
             */
            if (pleft > m_usablewidth - pright || ptop > m_usableheight - pbottom) {
                std::cout <<
                    "WARNING: "
                    "Given image margins overlap. Using "
                    "entire image instead."
                << std::endl;

                goto lbl_LOADIMAGE;
            }

            /* Calculate new width and height using margins. */
            int const width  = m_usablewidth - pleft - pright;
            int const height = m_usableheight - ptop - pbottom;

            /*
             * If the margins exceed the image's (valid) size, display a warning
             * and continue using the entire (valid) image instead.
             */
            if (width <= 0 || height <= 0) {
                std::cout <<
                    "WARNING: Given image margins exceed source image size. "
                    "Using entire image instead."
                    << std::endl;

                goto lbl_LOADIMAGE;
            }

            m_usablewidth  = width;
            m_usableheight = height;

            /*
             * Instruct QImageReader to load the partial image only,
             * as given by the margins.
             */
            c_reader.setClipRect(
                QRect(
                    pleft,
                    ptop,
                    width,
                    height
                )
            );
        }

    lbl_LOADIMAGE:
        mcp_image = new QPixmap(QPixmap::fromImage(c_reader.read()));
        if (mcp_image->isNull()) {
            std::cout <<
                "ERROR: "
                "Could not read file \"" + sr_props.mc_path.toStdString() + "\". "
                "There was a problem with reading from the disk."
            << std::endl;

            return false;
        }

        /*
         * Mask-out the background color if the user
         * wishes to do so.
         */
        if (sr_props.m_ismask) {
            QBitmap const c_mask = mcp_image->createMaskFromColor(
                sr_props.mc_mask,
                Qt::MaskInColor
            );

            /*
             * Set the mask to the one created, effectively
             * rendering all pixels that correspond to the
             * given background color as transparent.
             */
            mcp_image->setMask(c_mask);
        }

        /*
         * Setup the scrollbars so they reflect the image dimensions.
         * The dimensions are scaled by the ratio between the physical
         * and the logical (screen) tile-size.
         */
        int_updatescrollbars();

        /* Update the title of the underlying tab-page. */
        settitle(base::getfname(mc_path = sr_props.mc_path));
        return true;
    }

    void tilesetview::unloadimage() {
        if (mcp_image != nullptr) {
            delete mcp_image;

            mcp_image = nullptr;
        }
    }

    void tilesetview::int_updatescrollbars() {
        QScrollBar *cp_horz = horizontalScrollBar();
        QScrollBar *cp_vert = verticalScrollBar();

        float const scale = float(gl_viewtsize) / m_tsize;

        /* Update horizontal scrollbar. */
        cp_horz->setSingleStep(gl_viewtsize);
        cp_horz->setRange(
            0,
            (viewport()->width() < int(m_usablewidth * scale)
                ? int(m_usablewidth * scale) - viewport()->width()
                : 0
            )
        );
        cp_horz->setPageStep(viewport()->size().width());

        /* Update vertical scrollbar. */
        cp_vert->setSingleStep(gl_viewtsize);
        cp_vert->setRange(
            0,
            (viewport()->height() < int(m_usableheight * scale)
                ? int(m_usableheight * scale) - viewport()->height()
                : 0
            )
        );
        cp_vert->setPageStep(viewport()->size().height());
    }

    bool tilesetview::int_iscontentssmaller() {
        float const scale = float(gl_viewtsize) / m_tsize;

        return
               int(m_usablewidth * scale) < viewport()->width()
            || int(m_usableheight * scale) < viewport()->height()
        ;
    }

    void tilesetview::int_calcselarea() {
        mbe::sourcewndsel sel(m_srcid);

        /* Calculate selection area. */

        /* Add the selection to the buffer. */
        mbe::sourcewindow *cp_refparent = dynamic_cast<mbe::sourcewindow *>(parentWidget());
        if (cp_refparent == NULL)
            return;

        cp_refparent->addsel(sel);
    }
} /* namespace mbe */


