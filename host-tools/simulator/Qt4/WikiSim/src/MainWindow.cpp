/***************************************************************************
 *   Copyright (C) 2008 by Daniel Mack   *
 *   daniel@caiaq.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "MainWindow.h"
#include "WikiDisplay.h"

#include <QVBoxLayout>

void
MainWindow::powerButtonEvent(void)
{
    printf(" --- power button clicked --- \n");
}

MainWindow::MainWindow(QWidget *parent)
 : QWidget(parent)
{
    display = new WikiDisplay();
    powerSwitch = new QPushButton("&power/lid switch");
    cardInserted = new QPushButton("SD &card inserted");
    batteryState = new QSlider(Qt::Horizontal);

    powerSwitch->setCheckable(TRUE);
    powerSwitch->setChecked(TRUE);
    connect(powerSwitch, SIGNAL(clicked()), this, SLOT(powerButtonEvent()));

    cardInserted->setCheckable(TRUE);
    cardInserted->setChecked(TRUE);

    batteryState->setValue(100);

    /*
     * Display and three button side by side
     * below the three hardware buttons
     */
    QVBoxLayout *controlSwitches = new QVBoxLayout;
    controlSwitches->addWidget(powerSwitch);
    controlSwitches->addWidget(cardInserted);
    controlSwitches->addWidget(batteryState);
    controlSwitches->addItem(new QSpacerItem(1,1,
                               QSizePolicy::MinimumExpanding,
                               QSizePolicy::MinimumExpanding));

    QVBoxLayout *displayBox = new QVBoxLayout;
    displayBox->addWidget(display);
    displayBox->addItem(new QSpacerItem(1,1,
                          QSizePolicy::MinimumExpanding,
                          QSizePolicy::MinimumExpanding));

    QHBoxLayout *area = new QHBoxLayout;
    area->addItem(displayBox);
    area->addItem(controlSwitches);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addItem(area);
    setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
	delete display;
}
