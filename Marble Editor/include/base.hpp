#pragma once

#include <iostream>

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


namespace mbe {
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
    } /* namespace base */
} /* namespace mbe */


