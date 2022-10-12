#pragma once

#include <base.hpp>


namespace mbe {
	class mainwindow : public QMainWindow {
		Q_OBJECT

	public:
		mainwindow(QSize const &cr_dims, QWidget *cp_parent = nullptr);
		~mainwindow();

	private:
		void int_createmenubar();
		void int_createwidgets();

		QMenuBar *mw_menubar;
	};
}


