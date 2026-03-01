
// Code posted by StackOverflow user 'x squared' here: http://stackoverflow.com/a/37119983/2416499
// Someone also put his code into a github repository: https://github.com/Elypson/qt-collapsible-section
// Slightly modified.


#include "ExpandableSection.h"
#include <QPropertyAnimation>

ExpandableSection::ExpandableSection(const QString& title, int animationDuration, QWidget* parent) :
	QWidget(parent),
	animationDuration(animationDuration)
{
	toggleButton.setStyleSheet("QToolButton { border: none; }");
	toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toggleButton.setArrowType(Qt::ArrowType::RightArrow);
	toggleButton.setText(title);
	toggleButton.setCheckable(true);
	toggleButton.setChecked(false);

	headerLine.setFrameShape(QFrame::HLine);
	headerLine.setFrameShadow(QFrame::Sunken);
	headerLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	contentArea.setStyleSheet("QScrollArea { border: none; }");
	contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// start out collapsed
	contentArea.setMaximumHeight(0);
	contentArea.setMinimumHeight(0);
	// let the entire widget grow and shrink with its content
	toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
	toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
	toggleAnimation.addAnimation(new QPropertyAnimation(&contentArea, "maximumHeight"));
	// don't waste space
	mainLayout.setVerticalSpacing(0);
	mainLayout.setContentsMargins(0, 0, 0, 0);
	int row = 0;
	mainLayout.addWidget(&toggleButton, row, 0, 1, 1, Qt::AlignLeft);
	mainLayout.addWidget(&headerLine, row++, 2, 1, 1);
	mainLayout.addWidget(&contentArea, row, 0, 1, 3);
	setLayout(&mainLayout);

	connect(&toggleButton, SIGNAL(clicked(bool)), SLOT(setExpanded(bool)));
}

void ExpandableSection::setContentLayout(QLayout& contentLayout)
{
	delete contentArea.layout();
	contentArea.setLayout(&contentLayout);
	const auto collapsedHeight = sizeHint().height() - contentArea.maximumHeight();
	auto contentHeight = contentLayout.sizeHint().height();
	for (int i = 0; i < toggleAnimation.animationCount() - 1; ++i)
	{
		QPropertyAnimation* anim = static_cast<QPropertyAnimation*>(toggleAnimation.animationAt(i));
		anim->setDuration(animationDuration);
		anim->setStartValue(collapsedHeight);
		anim->setEndValue(collapsedHeight + contentHeight);
	}
	QPropertyAnimation* contentAnimation =
		static_cast<QPropertyAnimation*>(toggleAnimation.animationAt(toggleAnimation.animationCount() - 1));
	contentAnimation->setDuration(animationDuration);
	contentAnimation->setStartValue(0);
	contentAnimation->setEndValue(contentHeight);
}


void ExpandableSection::setExpanded(bool expanded)
{
	toggleButton.blockSignals(true);
	toggleButton.setChecked(expanded);
	toggleButton.blockSignals(false);
	toggleButton.setArrowType(expanded ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
	toggleAnimation.setDirection(expanded ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
	toggleAnimation.start();
}
