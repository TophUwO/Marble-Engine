#pragma once

#include <base.hpp>
#include <tab.hpp>
#include <dialog.hpp>


namespace mbe {
    /*
     * Class representing a tileset view; can be added
     * to any QTabWidget.
     */
    class tilesetview : public QAbstractScrollArea, public tabpage {
        Q_OBJECT

    public:
        tilesetview(int32_t srcid, QWidget *cp_parent = nullptr);
        ~tilesetview();

        /*
         * Instructs the object to load an image file as a
         * tileset, from disk.
         * All properties are provided by the **sr_props**
         * parameter.
         * If there is already an image loaded, it will be
         * replaced with the new image if the new image
         * does not fail to load.
         * 
         * Returns true if the image could be loaded successfully,
         * false if there was an error.
         */
        bool loadfromfile(dialog::tilesetprops const &sr_props);
        /*
         * Unloads the image and deallocates all resources
         * used by it.
         * 
         * Returns nothing.
         */
        void unloadimage();
       
    protected:
        virtual void paintEvent(QPaintEvent *cp_event) override;
        virtual void mousePressEvent(QMouseEvent *cp_event) override;
        virtual void mouseReleaseEvent(QMouseEvent *cp_event) override;
        virtual void resizeEvent(QResizeEvent *cp_event) override;

    private:
        /*
         * Sets scrollbar values so that they reflect the
         * size of the viewport contents.
         * 
         * Returns nothing.
         */
        void int_updatescrollbars();
        /*
         * Returns whether the size of the contents is smaller than
         * the size of the viewport in any dimension.
         * 
         * Returns true if the contents are smaller than the viewport,
         * false if not.
         */
        bool int_iscontentssmaller();
        /*
         * Calculates the area of the latest selection, and adds it
         * to the selection buffer.
         * 
         * Returns nothing.
         */
        void int_calcselarea();

        QPixmap *mcp_image;
        QString  mc_path;

        int    m_usablewidth;
        int    m_usableheight;
        int    m_tsize;
        QPoint mc_selstart;
        QPoint mc_selend;
        bool   m_issel;
    };
} /* namespace mbe */


