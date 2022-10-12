#pragma once

#include <base.hpp>

#include <QMainWindow>


namespace mbe {
	class mainwindow : public QMainWindow {
		Q_OBJECT

	public:
		mainwindow(QSize const &cr_dims, QWidget *cp_parent = nullptr);
		~mainwindow();
	};
}


