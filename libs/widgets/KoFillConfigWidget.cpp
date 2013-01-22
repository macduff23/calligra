/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoFillConfigWidget.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QSizePolicy>
#include <QBitmap>

#include <klocale.h>

#include <KoGroupButton.h>
#include <KoIcon.h>
#include <KoColor.h>
#include <KoColorPopupAction.h>
#include <KoSliderCombo.h>
#include "KoResourceServerProvider.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceSelector.h"
#include <KoCanvasController.h>
#include <KoSelection.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoShapeBackground.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeTransparencyCommand.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoResourcePopupAction.h>

static const char* const buttonnone[]={
    "16 16 3 1",
    "# c #000000",
    "e c #ff0000",
    "- c #ffffff",
    "################",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "################"};

static const char* const buttonsolid[]={
    "16 16 2 1",
    "# c #000000",
    ". c #969696",
    "################",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "################"};


// FIXME: Smoother gradient button.

static const char* const buttongradient[]={
    "16 16 15 1",
    "# c #000000",
    "n c #101010",
    "m c #202020",
    "l c #303030",
    "k c #404040",
    "j c #505050",
    "i c #606060",
    "h c #707070",
    "g c #808080",
    "f c #909090",
    "e c #a0a0a0",
    "d c #b0b0b0",
    "c c #c0c0c0",
    "b c #d0d0d0",
    "a c #e0e0e0",
    "################",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "################"};

static const char* const buttonpattern[]={
    "16 16 4 1",
    ". c #0a0a0a",
    "# c #333333",
    "a c #a0a0a0",
    "b c #ffffffff",
    "################",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "################"};

class KoFillConfigWidget::Private
{
public:
    Private()
    : canvas(0)
    {
    }

    QToolButton *colorButton;
    KoColorPopupAction *colorAction;
    KoResourcePopupAction *gradientAction;
    KoResourcePopupAction *patternAction;
    QButtonGroup *group;

    KoSliderCombo *opacity;

    QWidget *spacer;
    KoCanvasBase *canvas;
};

KoFillConfigWidget::KoFillConfigWidget(QWidget *parent)
:  QWidget(parent)
, d(new Private())
{
    setObjectName("Fill widget");
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    d->group = new QButtonGroup(this);
    d->group->setExclusive(true);

    // The button for no fill
    KoGroupButton *button = new KoGroupButton(KoGroupButton::GroupLeft, this);
    QPixmap noFillButtonIcon((const char **) buttonnone);
    noFillButtonIcon.setMask(QBitmap(noFillButtonIcon));
    button->setIcon(noFillButtonIcon);
    button->setToolTip(i18nc("No stroke or fill", "None"));
    button->setCheckable(true);
    d->group->addButton(button, None);
    layout->addWidget(button);

    // The button for solid fill
    button = new KoGroupButton(KoGroupButton::GroupCenter, this);
    button->setIcon(QPixmap((const char **) buttonsolid));
    button->setToolTip(i18nc("Solid color stroke or fill", "Solid"));
    button->setCheckable(true);
    d->group->addButton(button, Solid);
    layout->addWidget(button);

    // The button for gradient fill
    button = new KoGroupButton(KoGroupButton::GroupCenter, this);
    button->setIcon(QPixmap((const char **) buttongradient));
    button->setToolTip(i18n("Gradient"));
    button->setCheckable(true);
    d->group->addButton(button, Gradient);
    layout->addWidget(button);

    // The button for pattern fill
    button = new KoGroupButton(KoGroupButton::GroupRight, this);
    button->setIcon(QPixmap((const char **) buttonpattern));
    button->setToolTip(i18n("Pattern"));
    button->setCheckable(true);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->group->addButton(button, Pattern);
    layout->addWidget(button);

    connect(d->group, SIGNAL(buttonClicked(int)), this, SLOT(styleButtonPressed(int)));

    d->colorButton = new QToolButton(this);
    d->colorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(d->colorButton);

    d->colorAction = new KoColorPopupAction(this);
    d->colorAction->setToolTip(i18n("Change the filling color"));
    connect(d->colorAction, SIGNAL(colorChanged(const KoColor &)), this, SLOT(colorChanged()));

    d->colorButton->setDefaultAction(d->colorAction);
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
    
    // Gradient selector
    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    KoAbstractResourceServerAdapter *gradientResourceAdapter = new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer(), this);
    d->gradientAction = new KoResourcePopupAction(gradientResourceAdapter, d->colorButton);
    d->gradientAction->setToolTip(i18n("Change the filling color"));
    connect(d->gradientAction, SIGNAL(resourceSelected(KoResource*)), this, SLOT(gradientChanged(KoResource*)));

    // Pattern selector
    KoAbstractResourceServerAdapter *patternResourceAdapter = new KoResourceServerAdapter<KoPattern>(serverProvider->patternServer(), this);
    d->patternAction = new KoResourcePopupAction(patternResourceAdapter, d->colorButton);
    d->patternAction->setToolTip(i18n("Change the filling color"));
    connect(d->patternAction, SIGNAL(resourceSelected(KoResource*)), this, SLOT(patternChanged(KoResource*)));

    // Opacity setting
    // FIXME: There is also an opacity setting in the color chooser. How do they interact?
    d->opacity = new KoSliderCombo(this);
    d->opacity->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    d->opacity->setMinimum(0);
    d->opacity->setMaximum(100);
    d->opacity->setValue(100);
    d->opacity->setDecimals(0);
    connect(d->opacity, SIGNAL(valueChanged(qreal, bool)), this, SLOT(updateOpacity(qreal)));

    layout->addWidget(new QLabel(i18n("Opacity:")));
    layout->addWidget(d->opacity);

    // Spacer
    d->spacer = new QWidget();
    d->spacer->setObjectName("SpecialSpacer");
    layout->addWidget(d->spacer);

    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    if (selection) {
        d->canvas = canvasController->canvas();
        connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeChanged()));
    }
}

KoFillConfigWidget::~KoFillConfigWidget()
{
    delete d;
}

void KoFillConfigWidget::setCanvas( KoCanvasBase *canvas )
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeChanged()));

    d->canvas = canvas;
}

void KoFillConfigWidget::styleButtonPressed(int buttonId)
{
    switch (buttonId) {
        case KoFillConfigWidget::None:
            // Direct manipulation
            d->colorButton->setDefaultAction(d->colorAction);
            noColorSelected();
            break;
        case KoFillConfigWidget::Solid:
            d->colorButton->setDefaultAction(d->colorAction);
            colorChanged();
            break;
        case KoFillConfigWidget::Gradient:
            // Only select mode in the widget, don't set actual gradient :/
            d->colorButton->setDefaultAction(d->gradientAction);
            gradientChanged(d->gradientAction->currentResource());
            break;
        case KoFillConfigWidget::Pattern:
            // Only select mode in the widget, don't set actual pattern :/
            d->colorButton->setDefaultAction(d->patternAction);
            patternChanged(d->patternAction->currentResource());
            break;
    }
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
}

void KoFillConfigWidget::noColorSelected()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count())
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty())
        return;

    canvasController->canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, 0));
}

void KoFillConfigWidget::colorChanged()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count())
        return;

    KoShapeBackground *fill = new KoColorBackground(d->colorAction->currentColor());

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty())
        return;

    KUndo2Command *firstCommand = 0;
    foreach (KoShape *shape, selectedShapes) {
        if (! firstCommand)
            firstCommand = new KoShapeBackgroundCommand(shape, fill);
        else
            new KoShapeBackgroundCommand(shape, fill, firstCommand);
    }
    canvasController->canvas()->addCommand(firstCommand);
}

void KoFillConfigWidget::gradientChanged(KoResource *resource)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count())
        return;

    KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient*>(resource);
    if (! gradient)
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty())
        return;

    QGradient *newGradient = gradient->toQGradient();
    if (! newGradient)
        return;

    QGradientStops newStops = newGradient->stops();
    delete newGradient;

    KUndo2Command *firstCommand = 0;
    foreach (KoShape *shape, selectedShapes) {
        KoShapeBackground *fill = applyFillGradientStops(shape, newStops);
        if (! fill)
            continue;
        if (! firstCommand)
            firstCommand = new KoShapeBackgroundCommand(shape, fill);
        else
            new KoShapeBackgroundCommand(shape, fill, firstCommand);
    }
    canvasController->canvas()->addCommand(firstCommand);
}

void KoFillConfigWidget::patternChanged(KoResource *resource)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count())
        return;

    KoPattern *pattern = dynamic_cast<KoPattern*>(resource);
    if (! pattern)
        return;

    QList<KoShape*> selectedShapes = canvasController->canvas()->shapeManager()->selection()->selectedShapes();
    if (selectedShapes.isEmpty())
        return;

    KoImageCollection *imageCollection = canvasController->canvas()->shapeController()->resourceManager()->imageCollection();
    if (imageCollection) {
        KoPatternBackground *fill = new KoPatternBackground(imageCollection);
        fill->setPattern(pattern->image());
        canvasController->canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, fill));

    }
}

void KoFillConfigWidget::updateOpacity(qreal opacity)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count())
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    if (!selectedShapes.count())
        return;

    canvasController->canvas()->addCommand(new KoShapeTransparencyCommand(selectedShapes, 1.0 - opacity / 100));
}

void KoFillConfigWidget::shapeChanged()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KoShape *shape = selection->firstSelectedShape();
    if (! shape)
        return;

    updateWidget(shape);
}


void KoFillConfigWidget::updateWidget(KoShape *shape)
{
    // We don't want the opacity slider to send any signals when it's only initialized.
    // Otherwise an undo record is created.
    d->opacity->blockSignals(true);
    d->opacity->setValue(100 - shape->transparency() * 100);
    d->opacity->blockSignals(false);

    KoColorBackground *colorBackground = dynamic_cast<KoColorBackground*>(shape->background());
    KoGradientBackground *gradientBackground = dynamic_cast<KoGradientBackground*>(shape->background());
    KoPatternBackground *patternBackground = dynamic_cast<KoPatternBackground*>(shape->background());

    if (colorBackground) {
        d->colorAction->setCurrentColor(colorBackground->color());
        d->group->button(KoFillConfigWidget::Solid)->setChecked(true);
        d->colorButton->setDefaultAction(d->colorAction);
    } else if (gradientBackground) {
        d->group->button(KoFillConfigWidget::Gradient)->setChecked(true);
        d->colorButton->setDefaultAction(d->gradientAction);
    } else if (patternBackground) {
        d->group->button(KoFillConfigWidget::Pattern)->setChecked(true);
        d->colorButton->setDefaultAction(d->patternAction);
    } else {
        // No Fill
        d->group->button(KoFillConfigWidget::None)->setChecked(true);
        d->colorButton->setDefaultAction(d->colorAction);
    }
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
}

KoShapeBackground *KoFillConfigWidget::applyFillGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return 0;

    KoGradientBackground *newGradient = 0;
    KoGradientBackground *oldGradient = dynamic_cast<KoGradientBackground*>(shape->background());
    if (oldGradient) {
        // just copy the gradient and set the new stops
        QGradient *g = KoFlake::cloneGradient(oldGradient->gradient());
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
        newGradient->setTransform(oldGradient->transform());
    }
    else {
        // no gradient yet, so create a new one
        QLinearGradient *g = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        g->setCoordinateMode(QGradient::ObjectBoundingMode);
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
    }
    return newGradient;
}

void KoFillConfigWidget::blockChildSignals(bool block)
{
    d->colorButton->blockSignals(block);
    d->colorAction->blockSignals(block);
    d->gradientAction->blockSignals(block);
    d->patternAction->blockSignals(block);
    d->group->blockSignals(block);
    d->opacity->blockSignals(block);
}


#include <KoFillConfigWidget.moc>
