/***************************************************************************
                          kexibrowser.cpp  -  description
                             -------------------
    begin                : Sun Jun 9 2002
    copyright            : (C) 2002 by lucijan busch
    email                : lucijan@gmx.at
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qheader.h>
#include <qpoint.h>
#include <qlistview.h>

#include "kexi.h"
#include "kexibrowser.h"
#include "kexiformedit.h"


KexiBrowser::KexiBrowser(Kexi *mainWin, QWidget *parent, const char *name ) : QWidget(parent,name)
{
	m_mainWin = mainWin;
	m_parent = parent;

	KIconLoader *iconLoader = KGlobal::iconLoader();

	QGridLayout *g = new QGridLayout(this);
	
	m_list = new KListView(this);
	
/*	connect(m_list, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)), this,
		SLOT(slotContextMenu(QListViewItem *, const QPoint)));*/
	
	m_list->header()->hide();
	m_list->setRootIsDecorated(true);
	
	m_list->addColumn(i18n("Database content"));
	m_list->setResizeMode(QListView::LastColumn);
	
	m_database = new KListViewItem(m_list, i18n("Database"));
	m_tables = new KListViewItem(m_database, i18n("Tables"));
	m_queries = new KListViewItem(m_database, i18n("Queries"));
	m_forms = new KListViewItem(m_database, i18n("Forms"));
	m_reports = new KListViewItem(m_database, i18n("Reports"));

	m_database->setPixmap(0, iconLoader->loadIcon("db", KIcon::Small));
	m_tables->setPixmap(0, iconLoader->loadIcon("tables", KIcon::Small));
	m_queries->setPixmap(0, iconLoader->loadIcon("queries", KIcon::Small));
	m_forms->setPixmap(0, iconLoader->loadIcon("forms", KIcon::Small));
	m_reports->setPixmap(0, iconLoader->loadIcon("reports", KIcon::Small));
	
	
	m_list->setOpen(m_database, true);
		
	KPushButton *btnCreate = new KPushButton(i18n("&create"), this);
	connect(btnCreate, SIGNAL(clicked()), this, SLOT(slotCreate()));
	KPushButton *btnDrop = new KPushButton(i18n("&drop"), this);
	KPushButton *btnDevelop = new KPushButton(i18n("&develop"), this);
	
	g->addWidget(m_list,		0,	0);
	g->addWidget(btnCreate,		1,	0);
	g->addWidget(btnDrop,		2,	0);
	g->addWidget(btnDevelop,	3,	0);
}

/*void KexiBrowser::slotContextMenu(QListViewItem *i, const QPoint p)
{
	kdDebug() << "context menu requested..." << endl;
	if(i)
	{
		KPopupMenu *m = new KPopupMenu();
		m->insertItem("create form", SLOT(slotCreate(QListViewItem *i));
		m->exec(p);
	}
}
*/

void KexiBrowser::slotCreate()
{
	kdDebug() << "creating..." << endl;
	KexiFormEdit *nef = new KexiFormEdit(m_parent);
	m_mainWin->setDocumentWidget(nef);
	m_mainWin->setFormEdit(nef);
}

KexiBrowser::~KexiBrowser(){
}

//#include "kexibrowser.moc"
#include "kexibrowser.moc"
