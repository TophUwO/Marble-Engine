#include <sourcewnd.hpp>


namespace mbe {
    sourcewindow::sourcewindow(QMainWindow *cp_refmainwnd, bool isvisible, QWidget *cp_parent)
        : dockwindow(
            cp_refmainwnd,
            "Source Window",
            Qt::DockWidgetArea::LeftDockWidgetArea,
            isvisible,
            cp_parent
        )
    {
        /* Create child widgets. */
        int_createwidgets();
    }

    sourcewindow::~sourcewindow() {
    
    }

    void sourcewindow::int_createwidgets() {
        /* Create tool-bar. */
        int_createtoolbar();

        /* Create tab-widget. */
        mw_tabctrl = new QTabWidget(this);

        /* Set tab-widget properties. */
        mw_tabctrl->setMovable(true);
        mw_tabctrl->setTabsClosable(true);

        /* Connect signal handlers. */
        connect(mw_tabctrl, SIGNAL(tabCloseRequested(int)), this, SLOT(int_closepage(int)));

        /*
         * Associate tab-widget with the QMainWindow of the
         * underlying mbe::dockwindow.
         */
        mw_main->setCentralWidget(mw_tabctrl);

        /* Update the toolbar for the first time. */
        int_updatetoolbaractions();
    }

    void sourcewindow::int_createtoolbar() {
        /*
         * Default toolbar layout for the Source Window. This may be
         * overwritten by a settings file. If the file does not exist
         * or the file does not specify a custom toolbar layout for
         * the source window, the hardcoded layout below will be used.
         */
        struct {
            QAction  **mcpp_action;    /* action pointer */
            QToolBar **mcpp_tbar;      /* toolbar to add the action to */
            QString    mc_iconpath;    /* icon path */
            
            bool       m_istogglebtn;  /* Is tool-button toggle or not? */
            bool       m_isdefchecked; /* Is the button checked by default? */
        } const gl_actiontable[] = {
            { &ms_actions.mcp_newdocact,   &mw_maintbar, ":/icons/gui/ico_doc.svg",         false, false },
            { &ms_actions.mcp_opendocact,  &mw_maintbar, ":/icons/gui/ico_folder.svg",      false, false },
            { &ms_actions.mcp_savedocact,  &mw_maintbar, ":/icons/gui/ico_floppy.svg",      false, false },
            { &ms_actions.mcp_saveallact,  &mw_maintbar, ":/icons/gui/ico_multifloppy.svg", false, false },
            { &ms_actions.mcp_selbackward, &mw_seltbar,  ":/icons/gui/ico_backward.svg",    false, false },
            { &ms_actions.mcp_selforward,  &mw_seltbar,  ":/icons/gui/ico_forward.svg",     false, false },
            { nullptr,                     &mw_seltbar,  "",                                false, false },
            { &ms_actions.mcp_multisel,    &mw_seltbar,  ":/icons/gui/ico_multisel.svg",    true,  false },
            { &ms_actions.mcp_bordersel,   &mw_seltbar,  ":/icons/gui/ico_rectborder.svg",  true,  false },
            { nullptr,                     &mw_seltbar,  "",                                false, false },
            { &ms_actions.mcp_tilesel,     &mw_seltbar,  ":/icons/gui/ico_pointer.svg",     true,  false },
            { &ms_actions.mcp_rectsel,     &mw_seltbar,  ":/icons/gui/ico_rect.svg",        true,  true  },
            { &ms_actions.mcp_ellipsesel,  &mw_seltbar,  ":/icons/gui/ico_ellipse.svg",     true,  false },
            { &ms_actions.mcp_rectpathsel, &mw_seltbar,  ":/icons/gui/ico_rectpath.svg",    true,  false },
            { &ms_actions.mcp_pathsel,     &mw_seltbar,  ":/icons/gui/ico_path.svg",        true,  false }
        };

        /* Create main toolbar. */
        mw_maintbar = new QToolBar(this);
        mw_maintbar->setWindowTitle("Source Toolbar");
        mw_maintbar->setIconSize({ 24, 24 });
        mw_maintbar->setStyleSheet("QToolBar { border: none; }");

        /* Create selection toolbar. */
        mw_seltbar = new QToolBar(this);
        mw_seltbar->setWindowTitle("Selection Toolbar");
        mw_seltbar->setIconSize({ 24, 24 });
        mw_seltbar->setStyleSheet("QToolBar { border: none; }");

        /* Add actions to their respective toolbar. */
        for (size_t i = 0; i < sizeof(gl_actiontable) / sizeof(gl_actiontable[0]); i++) {
            /*
             * If "nullptr" is found in the table, interpret this
             * as a command to add a separator.
             */
            if (gl_actiontable[i].mcpp_action == nullptr) {
                (*gl_actiontable[i].mcpp_tbar)->addSeparator();

                continue;
            }

            /* Create the action. */
            *gl_actiontable[i].mcpp_action = new QAction(*gl_actiontable[i].mcpp_tbar);
            (*gl_actiontable[i].mcpp_action)->setIcon(QIcon(gl_actiontable[i].mc_iconpath));
            (*gl_actiontable[i].mcpp_action)->setCheckable(gl_actiontable[i].m_istogglebtn);

            /* Check the action if it is supposed to be checked by default. */
            (*gl_actiontable[i].mcpp_action)->setChecked(gl_actiontable[i].m_isdefchecked);

            /* Add the action to the tool-bar. */
            (*gl_actiontable[i].mcpp_tbar)->addAction(*gl_actiontable[i].mcpp_action);
        }

        /*
         * Create action group for the toggle buttons and
         * add the toggle buttons to it.
         */
        ms_actions.mcp_toggleactions = new QActionGroup(this);
        ms_actions.mcp_toggleactions->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

        ms_actions.mcp_toggleactions->addAction(ms_actions.mcp_tilesel);
        ms_actions.mcp_toggleactions->addAction(ms_actions.mcp_rectsel);
        ms_actions.mcp_toggleactions->addAction(ms_actions.mcp_ellipsesel);
        ms_actions.mcp_toggleactions->addAction(ms_actions.mcp_rectpathsel);
        ms_actions.mcp_toggleactions->addAction(ms_actions.mcp_pathsel);

        /* Add toolbars to the source window. */
        mw_main->addToolBar(Qt::ToolBarArea::LeftToolBarArea, mw_maintbar);
        mw_main->addToolBar(Qt::ToolBarArea::LeftToolBarArea, mw_seltbar);
    }

    void sourcewindow::addpage(tabpage *cp_tpage, int index) {
        if (index == -1)
            index = mw_tabctrl->count();

        mw_tabctrl->insertTab(index, cp_tpage->getwidget(), cp_tpage->gettitle());

        int_updatetoolbaractions();
    }

    void sourcewindow::addsel(mbe::sourcewndsel const &cr_sel) {
        mc_urbuf.addsnapshot(cr_sel);
    }

    void sourcewindow::int_closepage(int index) {
        base::closetpage(mw_tabctrl, index);

        int_updatetoolbaractions();
    }

    void sourcewindow::int_updatetoolbaractions() {
        ms_actions.mcp_savedocact->setEnabled(mw_tabctrl->count() != 0 /* && currently selected source edited */);
        ms_actions.mcp_selbackward->setEnabled(mw_tabctrl->count() != 0 /* && !selbuf->firstselected() */);
        ms_actions.mcp_selforward->setEnabled(mw_tabctrl->count() != 0 /* && !selbuf->lastselected() */);
        ms_actions.mcp_rectsel->setEnabled(mw_tabctrl->count() != 0);
        ms_actions.mcp_rectpathsel->setEnabled(mw_tabctrl->count() != 0);
        ms_actions.mcp_pathsel->setEnabled(mw_tabctrl->count() != 0);
        ms_actions.mcp_ellipsesel->setEnabled(mw_tabctrl->count() != 0);
        ms_actions.mcp_multisel->setEnabled(mw_tabctrl->count() != 0);
    }
} /* namespace mbe */


