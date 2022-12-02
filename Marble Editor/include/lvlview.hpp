#pragma once

#include <base.hpp>
#include <tab.hpp>
#include <dialog.hpp>


namespace mbe {
    struct editorlevel {
        editorlevel() : mc_location(""), mps_asset(nullptr) { }
        explicit editorlevel(int width, int height, QString const &cr_loc, QString const &cr_ident, editoraman *ps_aman)
            : mc_location(cr_loc)
        {
            if (!create(width, height, cr_loc, cr_ident, ps_aman))
                throw std::bad_alloc();
        }

        ~editorlevel() {
            /* Decrease ref-count. */
            marble_asset_release(mps_asset);
        }

        /*
         *  
         */
        bool create(int width, int height, QString const &cr_loc, QString const &cr_ident, editoraman *ps_aman) {
            if (ps_aman == nullptr)
                return false;

            /* Prepare create-params. */
            marble_levelasset_crparams s_crparams = {
                width,
                height
            };

            /* Create asset. */
            return 
                marble_asset_createnew(
                    marble_asset_type::MARBLE_ASSETTYPE_LEVEL,
                    0,
                    &s_crparams,
                    ps_aman,
                    &mps_asset
                ) == MARBLE_EC_OK;
        }

        QString       mc_ident;
        QString       mc_location;
        marble_asset *mps_asset;
    };

    /*
     * Class representing a widget that can display a
     * level asset as used by Marble Engine. It can be
     * added to any QTabWidget.
     */
    class levelview : public QAbstractScrollArea, public tabpage {
        Q_OBJECT

    public:
        levelview(editoraman *ps_aman, QWidget *cp_parent = nullptr);
        ~levelview();

        /*
         * Creates a new level asset in the current level-view.
         * If there is already a level asset associated with the
         * view, it is replaced and subsequently deallocated.
         * 
         * Returns true if the load succeeded, false if there
         * was an error.
         */
        bool newlevel(dialog::levelprops const &sr_props);
        /*
         * Unloads the currently loaded asset. After the function
         * returns, the internal asset pointer will no longer
         * be valid.
         * 
         * Returns nothing.
         */
        void unloadlevel();
        /*
         * Pushes an empty layer of **type** to the top of the underlying
         * level asset. If no asset is loaded, the function does nothing.
         * 
         * Returns true if the layer could be added, false if there was
         * an error.
         */
        bool addlayer(enum marble_levelasset_layertype type);

    protected:
        virtual void paintEvent(QPaintEvent *cp_event) override;
        virtual void mousePressEvent(QMouseEvent *cp_event) override;
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

        editorlevel ms_level;
        editoraman *mps_refaman;
    };
} /* namespace mbe */


