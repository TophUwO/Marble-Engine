#pragma once

#include <base.hpp>
#include <tab.hpp>


namespace mbe {
    /*
     * Properties of a tileset represented by an
     * ordinary image file.
     */
    struct tilesetprops {
        tilesetprops()
            : m_ispartial(false), m_ismask(false),
              m_tsize(16), m_left(0), m_right(0),
              m_top(0), m_bottom(0),
              mc_path(""), mc_mask(Qt::black)
        { }

        /*
         * specifies whether the **m_*** fields specifying a
         * partial bitmap are valid
         */
        bool m_ispartial;
        /* specifies whether the **mc_mask** field is valid */
        bool m_ismask;

        int m_tsize;  /* width/height of a tile, in source pixels */
        int m_left;   /* left margin, in tiles */
        int m_top;    /* top margin, in tiles */
        int m_right;  /* right margin, in tiles */
        int m_bottom; /* bottom margin, in tiles */

        QString mc_path; /* file path */
        QColor  mc_mask; /* background color mask */
    };

    /*
     * Class representing a tileset view; can be added
     * to any QTabWidget.
     */
    class tilesetview : public tabpage, virtual public QAbstractScrollArea {
        Q_OBJECT

    public:
        tilesetview(QWidget *cp_parent = nullptr);
        ~tilesetview() { }

        /*
         * Instructs the object to load an image file as a
         * tileset, from disk.
         * All properties are provided by the **sr_props**
         * parameter.
         * 
         * Returns true if the image could be loaded successfully,
         * false if there was an error.
         */
        bool loadfromfile(tilesetprops const &sr_props);

    private:
        QPixmap mc_image;
    };
} /* namespace mbe */


