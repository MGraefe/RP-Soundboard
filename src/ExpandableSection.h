#pragma once
#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class ExpandableSection : public QWidget 
{
	Q_OBJECT
public:
	explicit ExpandableSection(const QString &title = "", int animationDuration = 300, QWidget *parent = 0);
	void setContentLayout(QLayout & contentLayout);

private:
	QGridLayout mainLayout;
	QToolButton toggleButton;
	QFrame headerLine;
	QParallelAnimationGroup toggleAnimation;
	QScrollArea contentArea;
	int animationDuration = 300;
};

