#pragma once

#include <mainwnd.hpp>


namespace mbe {
	mainwindow::mainwindow(QSize const &cr_dims, QWidget *cp_parent)
		: QMainWindow(cp_parent)
	{
		resize(cr_dims.width(), cr_dims.height());
	}

	mainwindow::~mainwindow() {

	}
}


