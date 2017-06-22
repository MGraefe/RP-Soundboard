// src/about_qt.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "about_qt.h"
#include "buildinfo.h"

extern "C"
{
#include <libavutil/avutil.h>
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
AboutQt::AboutQt(QWidget *parent) :
	QWidget(parent, Qt::Window | Qt::WindowTitleHint /*| Qt::CustomizeWindowHint*/ | Qt::WindowCloseButtonHint),
	ui(new Ui::AboutQt)
{
	const char *ffmpeg_version = av_version_info();
	ui->setupUi(this);
	ui->l_version->setText(QString(buildinfo_getPluginVersion()) + 
		"\nBuild on " + buildinfo_getBuildDate() + " " + buildinfo_getBuildTime() +
	    "\nFFmpeg Version: " + (ffmpeg_version ? ffmpeg_version : "unknown"));
	setFixedSize(size());
}
