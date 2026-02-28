// src/About.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QWidget>

#include "ui_About.h"

namespace Ui {
	class AboutQt;
}


class AboutQt : public QWidget
{
	Q_OBJECT

public:
	explicit AboutQt(QWidget *parent = 0);
	~AboutQt();

private:
	Ui::AboutQt *ui;
};

