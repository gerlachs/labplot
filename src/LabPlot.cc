/***************************************************************************
    File                 : LabPlot.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : main class
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <KSplashScreen>
#include <KDebug>
 
#include "MainWin.h"
#include "core/Project.h"
#include "table/Table.h"

int main (int argc, char *argv[]) {
	KAboutData aboutData( "LabPlot", "LabPlot",
			ki18n("LabPlot"), LVERSION,
			ki18n("An application for plotting and analysis of 2d and 3d functions and data."),
			KAboutData::License_GPL,
			ki18n("Copyright (c) 2008 Stefan Gerlach") );
	aboutData.addAuthor(ki18n("Stefan Gerlach"), ki18n("developer"), "stefan.gerlach@uni-konstanz.de", 0);
	aboutData.addAuthor(ki18n("Alexander Semke"), ki18n("developer"), "alexander.semke@web.de", 0);

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineOptions options;
	options.add("no-splash",ki18n("do not show the splash screen"));
	options.add("+[file]",ki18n("open a project file"));
	KCmdLineArgs::addCmdLineOptions( options );
 
	KApplication app;
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	QString filename;
	if (args->count() > 0)
		filename = args->arg(0);

	KSplashScreen *splash=0;
	if (args->isSet("-splash")) {
		QString file = KStandardDirs::locate("appdata", "labplot.png");
		QPixmap pixmap(file);
		splash= new KSplashScreen(pixmap);
		splash->show();
	}

#ifdef HAVE_GSL
	kDebug()<<"HAVE_GSL defined"<<endl;
#endif
#ifdef GSL_FOUND
	kDebug()<<"GSL_FOUND defined"<<endl;
#endif
#ifdef GSL_VERSION
	kDebug()<<"GSL_VERSION defined"<<endl;
#endif
// TODO:
//#if GSL_VERSION > 1.8
//	kDebug()<<"GSL_VERSION > 1.8"<<endl;
//#endif

	// init global defaults
	Project::staticInit();
	Table::setGlobalDefault("default_comment_visibility", false);
	Table::setGlobalDefault("default_column_width", 120);

	MainWin* window = new MainWin(0,filename);
	window->show();
//	sleep(1);		// to see splash screen
	if(splash)
		splash->finish(window);
	return app.exec();
}
