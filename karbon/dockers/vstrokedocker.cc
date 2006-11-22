/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002-2005 The Karbon Developers
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <Q3HButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include <QToolTip>
#include <QGridLayout>
#include <QFrame>
#include <QDockWidget>

#include <kiconloader.h>
#include <klocale.h>
#include <KoToolManager.h>
#include <KoCanvasController.h>
#include <KoDockFactory.h>
#include <KoUnitWidgets.h>

#include "vstroke.h"
#include "vselection.h"
#include "vstrokecmd.h"

#include "vstrokedocker.h"

VStrokeDockerFactory::VStrokeDockerFactory()
{
}

QString VStrokeDockerFactory::dockId() const
{
    return QString("Stroke Properties");
}

Qt::DockWidgetArea VStrokeDockerFactory::defaultDockWidgetArea() const
{
    return Qt::RightDockWidgetArea;
}

QDockWidget* VStrokeDockerFactory::createDockWidget()
{
    VStrokeDocker* widget = new VStrokeDocker();
    widget->setObjectName(dockId());

    return widget;
}


VStrokeDocker::VStrokeDocker()
{
	setWindowTitle( i18n( "Stroke Properties" ) );

	QPushButton *button;

	QWidget *mainWidget = new QWidget();
	QGridLayout *mainLayout = new QGridLayout( mainWidget );
	
	QLabel* widthLabel = new QLabel( i18n ( "Width:" ), mainWidget );
	mainLayout->addWidget( widthLabel, 0, 0 );
	// set min/max/step and value in points, then set actual unit
	m_setLineWidth = new KoUnitDoubleSpinBox( mainWidget, 0.0, 1000.0, 0.5, 1.0, KoUnit::U_PT, 2 );
	KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
	m_setLineWidth->setUnit( canvasController->canvas()->unit() );
    m_setLineWidth->setToolTip( i18n( "Set line width of actual selection" ) );
	mainLayout->addWidget ( m_setLineWidth, 0, 1 );
	connect( m_setLineWidth, SIGNAL( valueChanged( double ) ), mainWidget, SLOT( widthChanged() ) );
	
	QLabel* capLabel = new QLabel( i18n ( "Cap:" ), mainWidget );
	mainLayout->addWidget( capLabel, 1, 0 );
	m_capGroup = new Q3HButtonGroup( mainWidget );
	//port:
        // m_capGroup->setFrameShape( QFrame::NoFrame );
	m_capGroup->setInsideMargin( 1 );
	m_capGroup->setExclusive( true );
	button = new QPushButton( "", m_capGroup );
	button->setIcon( SmallIcon( "cap_butt" ) );
	button->setCheckable( true );
    button->setToolTip( i18n( "Butt cap" ) );
	m_capGroup->insert( button );
	button = new QPushButton( "", m_capGroup );
	button->setIcon( SmallIcon( "cap_round" ) );
	button->setCheckable( true );
    button->setToolTip( i18n( "Round cap" ) );
	m_capGroup->insert( button );
	button = new QPushButton( "", m_capGroup );
	button->setIcon( SmallIcon( "cap_square" ) );
	button->setCheckable( true );
    button->setToolTip( i18n( "Square cap" ) );
	m_capGroup->insert( button );
	mainLayout->addWidget( m_capGroup, 1, 1 );
	connect( m_capGroup, SIGNAL( clicked( int ) ), this, SLOT( slotCapChanged( int ) ) );
	
	QLabel* joinLabel = new QLabel( i18n ( "Join:" ), this );
	mainLayout->addWidget( joinLabel, 2, 0 );
	
	m_joinGroup = new Q3HButtonGroup( mainWidget );
	//port:
        // m_joinGroup->setFrameShape( QFrame::NoFrame );
	m_joinGroup->setInsideMargin( 1 );
	m_joinGroup->setExclusive( true );
	button = new QPushButton( "", m_joinGroup );
	button->setIcon( SmallIcon( "join_miter" ) );
	button->setCheckable( true );
	button->setToolTip( i18n( "Miter join" ) );
	m_joinGroup->insert( button );
	button = new QPushButton( "", m_joinGroup );
	button->setIcon( SmallIcon( "join_round" ) );
	button->setCheckable( true );
	button->setToolTip( i18n( "Round join" ) );
	m_joinGroup->insert( button );
	button = new QPushButton( "", m_joinGroup );
	button->setIcon( SmallIcon( "join_bevel" ) );
	button->setCheckable( true );
	button->setToolTip( i18n( "Bevel join" ) );
	m_joinGroup->insert( button );
	mainLayout->addWidget( m_joinGroup, 2, 1 );
	connect( m_joinGroup, SIGNAL( clicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );

	mainLayout->setRowStretch( 3, 1 );
	mainLayout->setColumnStretch( 1, 1 );
	mainLayout->activate();

	setWidget( mainWidget );

	updateDocker();
}

void VStrokeDocker::updateCanvas()
{
	/*
	if( m_part && m_part->document().selection()->objects().count() > 0 )
		m_part->addCommand( new VStrokeCmd( &m_part->document(), &m_stroke ), true );
	*/
}

void VStrokeDocker::slotCapChanged( int ID )
{
	switch( ID )
	{
		case 1:
			m_stroke.setLineCap( VStroke::capRound ); break;
		case 2:
			m_stroke.setLineCap( VStroke::capSquare ); break;
		default:
			m_stroke.setLineCap( VStroke::capButt );
	}
	updateCanvas();
}

void VStrokeDocker::slotJoinChanged( int ID )
{
	switch( ID )
	{
		case 1:
			m_stroke.setLineJoin( VStroke::joinRound ); break;
		case 2:
			m_stroke.setLineJoin( VStroke::joinBevel ); break;
		default:
			m_stroke.setLineJoin( VStroke::joinMiter );
	}
	updateCanvas();
}

void VStrokeDocker::updateDocker()
{
	disconnect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( widthChanged() ) ); 
	disconnect( m_capGroup, SIGNAL( clicked( int ) ), this, SLOT( slotCapChanged( int ) ) );
	disconnect( m_joinGroup, SIGNAL( clicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );

	switch( m_stroke.lineCap() )
	{
		case VStroke::capRound:
			m_capGroup->setButton( 1 ); break;
		case VStroke::capSquare:
			m_capGroup->setButton( 2 ); break;
		default:
			m_capGroup->setButton( 0 );
	}

	switch( m_stroke.lineJoin() )
	{
		case VStroke::joinRound:
			m_joinGroup->setButton( 1 ); break;
		case VStroke::joinBevel:
			m_joinGroup->setButton( 2 ); break;
		default:
			m_joinGroup->setButton( 0 );
	}
	
	m_setLineWidth->changeValue( m_stroke.lineWidth() );
	
	connect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( widthChanged() ) ); 
	connect( m_capGroup, SIGNAL( clicked( int ) ), this, SLOT( slotCapChanged( int ) ) );
	connect( m_joinGroup, SIGNAL( clicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );
}

void VStrokeDocker::widthChanged()
{
	m_stroke.setLineWidth( m_setLineWidth->value() );
	updateCanvas();
}

void VStrokeDocker::setStroke( const VStroke &stroke )
{
	m_stroke = stroke;
	updateDocker();
}

void VStrokeDocker::setUnit( KoUnit::Unit unit )
{
	disconnect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( widthChanged() ) ); 
	disconnect( m_capGroup, SIGNAL( clicked( int ) ), this, SLOT( slotCapChanged( int ) ) );
	disconnect( m_joinGroup, SIGNAL( clicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );

	m_setLineWidth->setUnit( unit );

	connect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( widthChanged() ) ); 
	connect( m_capGroup, SIGNAL( clicked( int ) ), this, SLOT( slotCapChanged( int ) ) );
	connect( m_joinGroup, SIGNAL( clicked( int ) ), this, SLOT( slotJoinChanged( int ) ) );
}
#include "vstrokedocker.moc"

