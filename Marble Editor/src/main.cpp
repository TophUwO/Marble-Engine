#include <mainwnd.hpp>


int main(int argc, char *argv[]) {
	QApplication c_app(argc, argv);

	/* Create main window. */
	mbe::mainwindow c_mainwnd({ 1200, 800 });
	c_mainwnd.show();

	return c_app.exec();
}


