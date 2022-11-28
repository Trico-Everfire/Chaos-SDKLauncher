#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVector>

#ifdef _WIN32
#define OS_BIN_DIR	  "bin/win64"
#define OS_BIN_SUFFIX ".bat"
#else
#define OS_BIN_DIR	  "bin/linux64"
#define OS_BIN_SUFFIX ".sh"
#endif

// We define a struct with the required
// variables, which we later use in MediaSection_t.
struct MediaItem_t
{
	QString name;
	QString icon;
	QString urlType;
	QString url;
	QStringList args;
};

// This struct is used to house MediaItem_t and
// a header name. it's used by Sections to hold
// the default configuration if the file were to
// be missing or deleted by the user.
struct MediaSection_t
{
	QString header;
	QVector<MediaItem_t> items;
};

// the default configuration
static QVector<MediaSection_t> Sections(
	{ {
		  "Applications",
		  { { "Hammer World Editor", ":/resource/hammer.png", "process", "${INSTALLDIR}bin/win64/hammer.exe", { "" } },
			{ "Model Viewer", ":/resource/modelviewer.png", "process", "${INSTALLDIR}bin/win64/hlmv.exe", { "-game", "p2ce" } },
			{ "Face Poser", ":/resource/faceposer.png", "process", "${INSTALLDIR}bin/win64/hlfaceposer.exe", { "-game", "p2ce" } },
			{ "P2:CE (Tools Mode)", ":/resource/logo_tools.png", "process", "${INSTALLDIR}p2ce_legacyui" OS_BIN_SUFFIX, { "-tools" } },
			{ "P2:CE", ":/resource/logo.png", "process", "${INSTALLDIR}p2ce_legacyui" OS_BIN_SUFFIX, { "" } },
			{ "P2:CE (Panorama) (Tools Mode)", ":/resource/logo_tools.png", "process", "${INSTALLDIR}p2ce" OS_BIN_SUFFIX, { "-tools" } },
			{ "P2:CE (Panorama)", ":/resource/logo.png", "process", "${INSTALLDIR}p2ce" OS_BIN_SUFFIX, { "" } } },
	  },
	  { "Documentation",
		{ { "Valve Developer Community", ":/resource/vdc.png", "url", "https://developer.valvesoftware.com", { "" } },
		  { "Chaos Wiki", ":/resource/chaos.png", "url", "https://chaosinitiative.github.io/Wiki/", { "" } },
		  { "Momentum Wiki", ":/resource/momentum.png", "url", "https://docs.momentum-mod.org/", { "" } } } } } );

// This function is used to get the default configuration
// It's used in the original construction of the config file.
QJsonDocument defaultConfig()
{
	QJsonDocument doc;
	QJsonArray root;

	for ( int i = 0; i < Sections.count(); i++ )
	{
		MediaSection_t section = Sections.at( i );
		QJsonArray applications;

		for ( int j = 0; j < section.items.count(); j++ )
		{
			MediaItem_t item = section.items.at( j );

			QJsonObject appItems1;
			appItems1["name"] = item.name;
			appItems1["icon"] = item.icon;
			appItems1["urlType"] = item.urlType;
			appItems1["url"] = item.url;
			QJsonArray arguments1;

			appItems1["args"] = QJsonArray::fromStringList( item.args );
			applications.append( appItems1 );
		}
		auto mainDictionary = QJsonObject();
		mainDictionary["header"] = section.header;
		mainDictionary["content"] = applications;
		root.append( mainDictionary );
	}
	doc.setArray( root );
	return doc;
}