/***************************************************************************
 *   Copyright (C) 2003 by Lucijan Busch                                   *
 *   lucijan@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#include <qworkspace.h>

#include <kdeversion.h>
#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstdaction.h>

#include "formmanager.h"
#include "objecttreeview.h"
#include "kexipropertyeditor.h"
#include "kfmview.h"

KFMView::KFMView()
 : KMainWindow()
{
	w = new QWorkspace(this);
	setCentralWidget(w);
	w->show();

	manager = new KFormDesigner::FormManager(w, this, "manager");

	QDockWindow *dockTree = new QDockWindow(this);
	KFormDesigner::ObjectTreeView *view = new KFormDesigner::ObjectTreeView(dockTree);
	dockTree->setWidget(view);
	dockTree->setCaption(i18n("Objects"));
	dockTree->setResizeEnabled(true);
	dockTree->setFixedExtentWidth(256);
	moveDockWindow(dockTree, DockRight);

	QDockWindow *dockEditor = new QDockWindow(this);
	KexiPropertyEditor *editor = new KexiPropertyEditor(dockEditor);
	dockEditor->setWidget(editor);
	dockEditor->setCaption(i18n("Properties"));
	dockEditor->setResizeEnabled(true);
	moveDockWindow(dockEditor, DockRight);

	connect(w, SIGNAL(windowActivated(QWidget*)), manager, SLOT(windowChanged(QWidget*)));

	manager->setEditors(editor, view);

	new KAction(i18n("Print object tree"), "view_tree", KShortcut(0), manager, SLOT(debugTree()), actionCollection(), "dtree");
	KStdAction::save(manager, SLOT(saveForm()), actionCollection());
	KStdAction::saveAs(manager, SLOT(saveFormAs()), actionCollection());
	KStdAction::open(manager, SLOT(loadForm()), actionCollection());
	KStdAction::openNew(manager, SLOT(createBlankForm()), actionCollection());
	KStdAction::cut(manager, SLOT(cutWidget()), actionCollection());
	KStdAction::copy(manager, SLOT(copyWidget()), actionCollection());
	KStdAction::paste(manager, SLOT(pasteWidget()), actionCollection());
	KStdAction::printPreview(this, SLOT(slotPreviewForm()), actionCollection());
#if KDE_IS_VERSION(3,1,9)
	KStdAction::clear(manager, SLOT(deleteWidget()), actionCollection());
#else
	//TODO
#endif
	manager->createActions(actionCollection(), this);

	createGUI("kfmui.rc", true);
	setXMLFile("kfmui.rc", true);

}

void
KFMView::slotPreviewForm()
{
	QWidget *widg = new QWidget(w);
	manager->previewForm(manager->activeForm(), widg);
}

KFMView::~KFMView()
{
}

#include "kfmview.moc"
