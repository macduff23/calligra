/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>

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
#include "KoResourceItemChooser.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <klineedit.h>
#include <QHeaderView>
#include <QAbstractProxyModel>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#ifdef GHNS
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include "KoResource.h"

class KoResourceItemChooser::Private
{
public:
    Private() : model(0), view(0), buttonGroup(0) {}
    KoResourceModel* model;
    KoResourceItemView* view;
    QButtonGroup* buttonGroup;
    KLineEdit *tagSearchLineEdit, *tagOpLineEdit;
    KoResource *curResource;
    QString knsrcFile;
    QCompleter *tagCompleter;
};

KoResourceItemChooser::KoResourceItemChooser( KoAbstractResourceServerAdapter * resourceAdapter, QWidget *parent )
 : QWidget( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);
    d->model = new KoResourceModel(resourceAdapter, this);
    d->view = new KoResourceItemView(this);
    d->view->setModel(d->model);
    d->view->setItemDelegate( new KoResourceItemDelegate( this ) );
    d->view->setSelectionMode( QAbstractItemView::SingleSelection );
    connect( d->view, SIGNAL(clicked( const QModelIndex & ) ),
             this, SLOT(activated ( const QModelIndex & ) ) );

    d->buttonGroup = new QButtonGroup( this );
    d->buttonGroup->setExclusive( false );

    d->tagSearchLineEdit = new KLineEdit(this);
    d->tagSearchLineEdit->setClearButtonShown(true);
    d->tagSearchLineEdit->setClickMessage("Enter search tag here");
    d->tagSearchLineEdit->setEnabled( true );
    d->tagSearchLineEdit->hide();

    connect( d->tagSearchLineEdit, SIGNAL(returnPressed(QString)), this, SLOT(tagSearchLineEditActivated(QString)));
    connect( d->tagSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( d->tagSearchLineEdit );
    layout->addWidget( d->view );

    QGridLayout* buttonLayout = new QGridLayout;

    QPushButton *button = new QPushButton( this );
    button->setIcon( SmallIcon("document-open") );
    button->setToolTip( i18n("Import") );
    button->setEnabled( true );
    d->buttonGroup->addButton( button, Button_Import );
    buttonLayout->addWidget( button, 0, 0 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon("trash-empty") );
    button->setToolTip( i18n("Delete") );
    button->setEnabled( false );
    d->buttonGroup->addButton( button, Button_Remove );
    buttonLayout->addWidget( button, 0, 1 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon("download") );
    button->setToolTip( i18n("Download") );
    button->setEnabled( true );
    button->hide();
    d->buttonGroup->addButton( button, Button_GhnsDownload );
    buttonLayout->addWidget( button, 0, 3 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon("go-up") );
    button->setToolTip( i18n("Share") );
    button->setEnabled( false );
    button->hide();
    d->buttonGroup->addButton( button, Button_GhnsUpload);
    buttonLayout->addWidget( button, 0, 4 );


    connect( d->buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ));

    buttonLayout->setColumnStretch( 0, 1 );
    buttonLayout->setColumnStretch( 1, 1 );
    buttonLayout->setColumnStretch( 2, 2 );
    buttonLayout->setSpacing( 0 );
    buttonLayout->setMargin( 0 );

    d->tagOpLineEdit = new KLineEdit( this );
    d->tagOpLineEdit->setClickMessage("Add / Remove Tag");
    d->tagOpLineEdit->setEnabled( false );
    d->tagOpLineEdit->hide();

    connect( d->tagOpLineEdit, SIGNAL(returnPressed(QString)), this, SLOT(tagOpLineEditActivated(QString)));
    connect( d->tagOpLineEdit, SIGNAL(textChanged(QString)), this, SLOT(tagOpLineEditTextChanged(QString)));

    layout->addWidget( d->tagOpLineEdit );
    layout->addLayout( buttonLayout );

    d->tagCompleter = new QCompleter(getTagNamesList(""),this);
    d->tagSearchLineEdit->setCompleter(d->tagCompleter);


    updateButtonState();
}

KoResourceItemChooser::~KoResourceItemChooser()
{
    delete d;
}

void KoResourceItemChooser::slotButtonClicked( int button )
{
    if( button == Button_Import ) {
        QString extensions = d->model->resourceServerAdapter()->extensions();
        QString filter = extensions.replace(QString(":"), QString(" "));
        QString filename = KFileDialog::getOpenFileName( KUrl(), filter, 0, i18n( "Choose File to Add" ) );

        d->model->resourceServerAdapter()->importResourceFile(filename);
    }
    else if( button == Button_Remove ) {
        QModelIndex index = d->view->currentIndex();
        int row = index.row();
        int column = index.column();
        if( index.isValid() ) {

            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                d->model->resourceServerAdapter()->removeResource(resource);
            }
        }
        if (column == 0) {
            int rowMin = --row;
            row = qBound(0, rowMin, row);
        }
        int columnMin = --column;
        column = qBound(0, columnMin, column);
        setCurrentItem(row, column);
        activated(d->model->index(row, column));
    }
#ifdef GHNS
    else if (button == Button_GhnsDownload) {

        KNS3::DownloadDialog dialog(d->knsrcFile, this);
        dialog.exec();

        foreach (const KNS3::Entry& e, dialog.changedEntries()) {

             foreach(const QString &file, e.installedFiles()) {
                 QFileInfo fi(file);
                  d->model->resourceServerAdapter()->importResourceFile( fi.absolutePath()+"/"+fi.fileName() , false );
              }

       foreach(const QString &file, e.uninstalledFiles()) {
                 QFileInfo fi(file);
                 d->model->resourceServerAdapter()->removeResourceFile(fi.absolutePath()+"/"+fi.fileName());
              }
      }
     }
    else if (button == Button_GhnsUpload) {

        QModelIndex index = d->view->currentIndex();
        if( index.isValid() ) {


            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                KNS3::UploadDialog dialog(d->knsrcFile, this);
                dialog.setUploadFile(KUrl::fromLocalFile(resource->filename()));
                dialog.setUploadName(resource->name());
                dialog.exec();
            }
        }
    }
#endif
    updateButtonState();
}

void KoResourceItemChooser::showButtons( bool show )
{
    foreach( QAbstractButton * button, d->buttonGroup->buttons() )
        show ? button->show() : button->hide();
}

void KoResourceItemChooser::showGetHotNewStuff( bool showDownload, bool showUpload )
{
#ifdef GHNS
    QAbstractButton *button = d->buttonGroup->button(Button_GhnsDownload);
    showDownload ? button->show() : button->hide();
    button = d->buttonGroup->button(Button_GhnsUpload);
    showUpload ? button->show() : button->hide();
#endif
}

void KoResourceItemChooser::showTaggingBar(bool showSearchBar, bool showOpBar)
{
    showSearchBar ? d->tagSearchLineEdit->show() : d->tagSearchLineEdit->hide();
    showOpBar ? d->tagOpLineEdit->show() : d->tagOpLineEdit->hide();
}

void KoResourceItemChooser::setRowCount( int rowCount )
{
    int resourceCount = d->model->resourceServerAdapter()->resources().count();
    d->model->setColumnCount( static_cast<qreal>(resourceCount) / rowCount );
    //Force an update to get the right row height (in theory)
    QRect geometry = d->view->geometry();
    d->view->setViewMode(KoResourceItemView::FIXED_ROWS);
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
}

void KoResourceItemChooser::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
    //Force an update to get the right column width
    QRect geometry = d->view->geometry();
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
}

void KoResourceItemChooser::setRowHeight( int rowHeight )
{
    d->view->verticalHeader()->setDefaultSectionSize( rowHeight );
}

void KoResourceItemChooser::setColumnWidth( int columnWidth )
{
    d->view->horizontalHeader()->setDefaultSectionSize( columnWidth );
}

void KoResourceItemChooser::setItemDelegate( QAbstractItemDelegate * delegate )
{
    d->view->setItemDelegate(delegate);
}

KoResource *  KoResourceItemChooser::currentResource()
{
    QModelIndex index = d->view->currentIndex();
    if( index.isValid() )
        return resourceFromModelIndex(index);

    return 0;
}

void KoResourceItemChooser::setCurrentResource(KoResource* resource)
{
    QModelIndex index = d->model->indexFromResource(resource);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
    setTagOpLineEdit(d->model->resourceServerAdapter()->getAssignedTagsList(resource));
}

void KoResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->model->index(row, column);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
}

void KoResourceItemChooser::setProxyModel( QAbstractProxyModel* proxyModel )
{
    proxyModel->setSourceModel(d->model);
    d->view->setModel(proxyModel);
}

void KoResourceItemChooser::activated( const QModelIndex & index )
{
    if( ! index.isValid() )
        return;

    d->curResource = resourceFromModelIndex(index);

    if( d->curResource ) {
        emit resourceSelected( d->curResource );
        setTagOpLineEdit(d->model->resourceServerAdapter()->getAssignedTagsList(d->curResource ));
    }

    updateButtonState();
}

void KoResourceItemChooser::updateButtonState()
{
    QAbstractButton * removeButton = d->buttonGroup->button( Button_Remove );
    if( ! removeButton )
        return;

    QAbstractButton * uploadButton = d->buttonGroup->button(Button_GhnsUpload);
    if(!uploadButton)
        return;

    KoResource * resource = currentResource();
    if( resource ) {
        removeButton->setEnabled( resource->removable() );
        uploadButton->setEnabled(resource->removable());
        d->tagOpLineEdit->setEnabled( resource->removable());
        return;
    }

    removeButton->setEnabled( false );
    uploadButton->setEnabled(false);
    d->tagOpLineEdit->setEnabled( false );
}

KoResource* KoResourceItemChooser::resourceFromModelIndex(const QModelIndex& index)
{
    if(!index.isValid())
        return 0;

    const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>(index.model());
    if(proxyModel) {
        //Get original model index, because proxy models destroy the internalPointer
        QModelIndex originalIndex = proxyModel->mapToSource(index);
        return static_cast<KoResource*>( originalIndex.internalPointer() );
    }

    return static_cast<KoResource*>( index.internalPointer() );
}

void KoResourceItemChooser::setKnsrcFile(const QString& knsrcFileArg)
{
    d->knsrcFile = knsrcFileArg;
}

QSize KoResourceItemChooser::viewSize()
{
    return d->view->size();
}

void KoResourceItemChooser::setTagOpLineEdit(QStringList tagsList)
{
    QString tags;
    if(!tagsList.isEmpty()) {
        tagsList.sort();
        tags = tagsList.join(", ");
        tags.append(", ");
        d->tagOpLineEdit->setText(tags);
     }
    else
    {
        d->tagOpLineEdit->clear();
    }

    d->tagCompleter = new QCompleter(getTagNamesList(tags),this);
    d->tagOpLineEdit->setCompleter(d->tagCompleter);
}


void KoResourceItemChooser::tagOpLineEditActivated(QString lineEditText)
{
    QStringList tagsListNew = lineEditText.split(", ");
    if(tagsListNew.contains("")) {
        tagsListNew.removeAll("");
    }

    QStringList tagsList = d->model->resourceServerAdapter()->getAssignedTagsList(d->curResource);

    foreach(const QString& tag, tagsListNew) {
        if(!tagsList.contains(tag)) {
            d->model->resourceServerAdapter()->addTag(d->curResource, tag);
        }
     }

     foreach(const QString& tag, tagsList) {
        if(!tagsListNew.contains(tag)) {
            d->model->resourceServerAdapter()->delTag(d->curResource, tag);
        }
     }

    setTagOpLineEdit( d->model->resourceServerAdapter()->getAssignedTagsList(d->curResource));
}

void KoResourceItemChooser::tagOpLineEditTextChanged(QString lineEditText)
{
    if(lineEditText.isEmpty()) {
        QStringList assignedTagsList = d->model->resourceServerAdapter()->getAssignedTagsList(d->curResource);
        foreach(const QString& tag, assignedTagsList) {
            d->model->resourceServerAdapter()->delTag(d->curResource, tag);
        }
    }
    d->tagCompleter = new QCompleter(getTagNamesList(lineEditText),this);
    d->tagOpLineEdit->setCompleter(d->tagCompleter);
}

QStringList KoResourceItemChooser::getTagNamesList(QString lineEditText)
{
    QStringList tagNamesList = d->model->resourceServerAdapter()->getTagNamesList();

    if(lineEditText.contains(", ")) {
        QStringList tagsList = lineEditText.split(", ");
        if(tagsList.contains("")) {
           tagsList.removeAll("");
         }

        QStringList autoCompletionTagsList;
        QString joinText;

        if(lineEditText.endsWith(", ")) {
            joinText=lineEditText;
        }
        else {
            tagsList.removeLast();
            joinText = tagsList.join(", ");
            joinText.append(", ");
        }

        for(int i=0; i< tagNamesList.count(); i++) {
            if (!tagsList.contains(tagNamesList.at(i))) {
                autoCompletionTagsList  << joinText + tagNamesList.at(i);
            }
        }
        return autoCompletionTagsList;
    }
    return tagNamesList;
 }

QStringList KoResourceItemChooser::getTaggedResourceFileNames(QString lineEditText)
{
    return d->model->resourceServerAdapter()->searchTag(lineEditText);
}

void KoResourceItemChooser::tagSearchLineEditActivated(QString lineEditText)
{
    d->model->resourceServerAdapter()->setTagSearch(true);
    d->model->resourceServerAdapter()->setTaggedResourceFileNames(getTaggedResourceFileNames(lineEditText));
    d->model->resourceServerAdapter()->updateServer();

    if(!lineEditText.endsWith(", ")) {
        lineEditText.append(", ");
    }
    d->tagCompleter = new QCompleter(getTagNamesList(lineEditText),this);
    d->tagSearchLineEdit->setCompleter(d->tagCompleter);
    d->tagSearchLineEdit->setText( lineEditText );

}

void KoResourceItemChooser::tagSearchLineEditTextChanged(QString lineEditText)
{
    if(lineEditText.isEmpty()) {
        d->model->resourceServerAdapter()->setTagSearch(false);
        d->model->resourceServerAdapter()->updateServer();
    }
    d->tagCompleter = new QCompleter(getTagNamesList(lineEditText),this);
    d->tagSearchLineEdit->setCompleter(d->tagCompleter);
}

#include <KoResourceItemChooser.moc>
