#pragma once

/* STDLIB includes */
#include <iostream>

/* Qt includes */
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>
#include <QToolBar>
#include <QTabWidget>
#include <QScrollArea>
#include <QDialog>
#include <QCheckBox>
#include <QLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QImageReader>
#include <QScrollBar>
#include <QPainter>
#include <QMessageBox>
#include <QActionGroup>
#include <QMouseEvent>

/* Marble Engine includes */
#include <error.h>
#include <asset.h>
#include <level.h>


namespace mbe {
    typedef marble_assetman editoraman;

    /*
     * This namespace contains some global functions shared by all modules
     * of this application.
     */
    namespace base {
        /* layout item types */
        enum class loitemtype {
            unknown = 0, /* unknown/unspecified */

            widget,      /* widget */
            layout       /* child layout */
        };

        /* app states */
        enum class appstate {
            running    = 1, /* normal state */
            quit       = 2, /* set on quit */
            forcedquit = 3  /* set on forced quit */
        };

        /* structure holding current app-state */
        static struct {
            appstate m_state; /* state id */
            int      m_param; /* opt. parameter */
        } gl_appstate;

        constexpr size_t gl_defurbufsize = 128;

        /*
         * Transforms the state of a given checkbox into a boolean value. Tristate
         * is interpreted as "unchecked".
         * 
         * Returns true if the checkbox is checked, false if not.
         */
        inline bool iscbchecked(QCheckBox *cp_checkbox) {
            if (cp_checkbox == nullptr)
                return false;

            return cp_checkbox->checkState() == Qt::CheckState::Checked;
        }

        /*
         * Gets the type of the layout item. This can either be a widget or
         * a child layout.
         * 
         * Returns type.
         */
        inline loitemtype getloitemtype(QLayoutItem *cp_item) {
            if (cp_item == nullptr)
                return loitemtype::unknown;

            QWidget *cp_widget = cp_item->widget();
            QLayout *cp_layout = cp_item->layout();

            /*
             * If it is neither a layout nor a widget,
             * treat the type as unknown.
             */
            if (cp_widget == nullptr && cp_layout == nullptr)
                return loitemtype::unknown;

            return 
                cp_widget != nullptr
                    ? loitemtype::widget
                    : loitemtype::layout
                ;
        }

        /*
         * Enables or disables all widgets of a given layout, recursively.
         * 
         * Returns nothing.
         */
        inline void setlayoutstate(QLayout *cp_layout, bool newstate, bool recursive) {
            if (cp_layout == nullptr || cp_layout->count() == 0)
                return;

            for (int i = 0; i < cp_layout->count(); i++) {
                QLayoutItem *const cp_item = cp_layout->itemAt(i);

                switch (base::getloitemtype(cp_item)) {
                    case loitemtype::widget:
                        cp_item->widget()->setEnabled(newstate);

                        break;
                    case loitemtype::layout:
                        if (!recursive)
                            break;

                        /* Update state recursively. */
                        base::setlayoutstate(
                            cp_item->layout(),
                            newstate,
                            recursive
                        );

                        break;
                }
            }
        }

        /*
         * Extracts the file name (without extension) from a
         * file path.
         * 
         * Returns file name or "unknown" on error.
         */
        inline QString getfname(QString const &cr_path) {
            if (cr_path.isNull())
                return "unnamed";

            QFileInfo c_info(cr_path);
            QString   c_ret = c_info.baseName();

            return c_ret;
        }

        /*
         * Displays a message box promting the user to fill
         * in required fields of an input mask.
         * 
         * Returns nothing.
         */
        inline void dispmbinsuffinfo(QWidget *cp_parent) {
            QMessageBox::warning(
                cp_parent,
                "Insufficient information",
                "Fields marked with '*' may not be empty upon accepting the dialog.",
                QMessageBox::StandardButton::Ok
            );
        }


        /*
         * Class for handling an undo-redo buffer.
         * Will be used to save selections and other
         * states as well. 
         */
        template <class T>
        class undoredobuf {
        public:
            undoredobuf()
                : m_maxsize(gl_defurbufsize), m_currindex(0)
            { }
            undoredobuf(size_t maxsize)
                : m_maxsize(maxsize), m_currindex(0)
            { }
            ~undoredobuf() { }

            /*
             * Adds a snapshot to the buffer. If the current
             * number of snapshots equals the maximum capacity,
             * the oldest snapshot will be removed.
             * 
             * Returns nothing.
             */
            void addsnapshot(T const &cr_snap) {
                /*
                 * If the buffer is full, remove the
                 * oldest snapshot.
                 */
                if (mc_buffer.size() >= m_maxsize) {
                    mc_buffer.erase(mc_buffer.begin());

                    if (!isbegin())
                        --m_currindex;
                }

                /*
                 * Add the snapshot to the back of
                 * the list.
                 */
                mc_buffer.push_back(cr_snap);
                forward();
            }
            /*
             * Retrieves the currently-selected snapshot. 
             */
            T &getcurrsnapshot() { return mc_buffer[m_currindex]; }

            /*
             * Moves the selection pointer to the right
             * by one unit.
             * 
             * Returns nothing.
             */
            void forward() {
                if (isend())
                    return;

                ++m_currindex;
            }
            /*
             * Moves the selection pointer to the left
             * by one unit.
             * 
             * Returns nothing.
             */
            void backward() { 
                if (isbegin())
                    return;

                --m_currindex;
            }

            /*
             * Returns whether the current selection pointer
             * is pointing to the oldest snapshot in the
             * buffer.
             */
            bool isbegin() { return m_currindex == 0; }
            /*
             * Returns whether the current selection pointer
             * is pointing to the latest snapshot in the
             * buffer.
             */
            bool isend()   { return m_currindex == mc_buffer.size() - 1; }
            /*
             * Updates the maximum size to the new, provided value.
             * 
             * Returns the old size.
             */
            size_t setmaxsize(size_t newsize) {
                size_t oldsize = m_maxsize;
                m_maxsize = newsize;

                /*
                 * If the new size is smaller than the old
                 * size, remove all entries that are no
                 * longer in the buffer. Entries are removed
                 * from the beginning.
                 */
                if (oldsize > m_maxsize) {
                    size_t const diff = oldsize - newsize;

                    mc_buffer.erase(0, diff - 1);
                }
                
                return oldsize;
            }

        private:
            size_t m_currindex;       /* index of the current position */
            size_t m_maxsize;         /* maximum number of stored snapshots */

            std::vector<T> mc_buffer; /* actual selection buffer */
        };
    } /* namespace base */
} /* namespace mbe */


