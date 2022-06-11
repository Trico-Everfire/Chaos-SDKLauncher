#include <QApplication>
#include <QDialog>

int main(int argc, char** argv)
{
	QApplication app( argc, argv );

	auto pDialog = new QDialog( nullptr );
	pDialog->setWindowTitle( "Hello world!" );
	pDialog->show();

	return QApplication::exec();
}