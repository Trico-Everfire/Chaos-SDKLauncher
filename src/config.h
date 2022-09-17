//
// Created by trico on 17-9-22.
//

#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct MediaItem_t
{
	QString name;
	QString icon;
	QString urlType;
	QString url;
	QStringList args;
};

struct MediaSection_t
{
	QString header;
	QVector<MediaItem_t> items;
};

static QVector<MediaSection_t> Sections(
	{
		{
			"Applications",
			{
				{ "Hammer World Editor", ":/resource/hammer.png", "process", "hammer.exe", { "" } },
				{ "Model Viewer", ":/resource/modelviewer.png", "process", "hlmv.exe", { "-game p2ce" } },
				{ "Face Poser", ":/resource/faceposer.png", "process", "hlfaceposer.exe", { "-game p2ce" } },
				{ "P2:CE (Tools Mode)", ":/resource/logo_tools.png", "process", "chaos.exe", { "-game p2ce", "-tools" } },
				{ "P2:CE", ":/resource/logo.png", "process", "chaos.exe", { "-game p2ce" } }
			},
		},
		{
			"Documentation",
			{
				{ "Valve Developer Community", ":/resource/vdc.png", "url", "https://developer.valvesoftware.com", { "" } },
				{ "Chaos Wiki", ":/resource/chaos.png", "url", "https://chaosinitiative.github.io/Wiki/", { "" } },
				{ "Momentum Wiki", ":/resource/momentum.png", "url", "https://docs.momentum-mod.org/", { "" } }
			}
		}
	});

 QJsonDocument defaultConfig(){

		QJsonDocument doc;
		QJsonObject root;

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

				for(int k = 0; k < item.args.count(); k++){
					arguments1.append(item.args.at(k));
				}

				appItems1["args"] = arguments1;
				applications.append(appItems1);

			}
			root[section.header] = applications;
		}
		doc.setObject(root);

		return doc;
	}