/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "koFontDia.h"
#include "qrichtext_p.h"

#include <kcolordialog.h>
#include <klocale.h>
#include <kdebug.h>

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <koGlobal.h>
#include <qvgroupbox.h>
#include <qvbox.h>
#include <qhgroupbox.h>
#include <qhbuttongroup.h>


class KoFontChooser::KoFontChooserPrivate
{
public:
    QComboBox *m_strikeOut;
    QColor m_textColor;
    QCheckBox *m_shadow;
    KIntNumInput *m_relativeSize;
    QLabel *m_lRelativeSize;
    KIntNumInput *m_offsetBaseLine;
    QCheckBox *m_wordByWord;
#ifdef ATTRCOMBO
    QComboBox *m_fontAttribute;
#else
    QButtonGroup* m_fontAttribute;
#endif
    QComboBox *m_language;
    QCheckBox *m_hyphenation;
};

KoFontChooser::KoFontChooser( QWidget* parent, const char* name, bool _withSubSuperScript, uint fontListCriteria)
    : QTabWidget( parent, name )
{
    d = new KoFontChooserPrivate;
    setupTab1(_withSubSuperScript, fontListCriteria );
    setupTab2();
    m_changedFlags = 0;
}

KoFontChooser::~KoFontChooser()
{
    delete d; d = 0;
}

QColor KoFontChooser::color() const
{
    return d->m_textColor;
}

void KoFontChooser::setupTab1(bool _withSubSuperScript, uint fontListCriteria )
{
    QWidget *page = new QWidget( this );
    addTab( page, i18n( "&Fonts" ) );
    QVBoxLayout *lay1 = new QVBoxLayout( page, KDialog::marginHint(), KDialog::spacingHint() );
    QStringList list;
    KFontChooser::getFontList(list,fontListCriteria);
    m_chooseFont = new KFontChooser(page, "FontList", false, list);
    lay1->addWidget(m_chooseFont);

    QVGroupBox *grp = new QVGroupBox(i18n("Position"), page);
    lay1->addWidget(grp);
    QWidget* grpBox = new QWidget( grp ); // container for the grid - laid out inside the QVGroupBox
    QGridLayout *grid = new QGridLayout( grpBox, 2, 3, 0, KDialog::spacingHint() );
    grid->setColStretch( 1, 1 ); // better stretch labels than spinboxes.

    m_superScript = new QRadioButton(i18n("Su&perscript"),grpBox);
    grid->addWidget(m_superScript,0,0);

    m_subScript = new QRadioButton(i18n("Su&bscript"),grpBox);
    grid->addWidget(m_subScript,1,0);

    d->m_lRelativeSize = new QLabel ( i18n("Relative &size:"), grpBox);
    d->m_lRelativeSize->setAlignment( Qt::AlignRight );
    grid->addWidget(d->m_lRelativeSize,0,1);

    // ## How to make this widget smaller? Sounds like the [minimum]sizeHint for KIntNumInput is big...
    d->m_relativeSize = new KIntNumInput( grpBox );
    d->m_lRelativeSize->setBuddy( d->m_relativeSize );
    grid->addWidget(d->m_relativeSize,0,2);

    d->m_relativeSize-> setRange(1, 100, 1,false);
    d->m_relativeSize->setSuffix("%");

    QLabel *lab = new QLabel ( i18n("Offse&t from baseline:"), grpBox);
    lab->setAlignment( Qt::AlignRight );
    grid->addWidget(lab,1,1);

    d->m_offsetBaseLine= new KIntNumInput( grpBox );
    lab->setBuddy( d->m_offsetBaseLine );
    grid->addWidget(d->m_offsetBaseLine,1,2);

    d->m_offsetBaseLine->setRange(-9, 9, 1,false);
    d->m_offsetBaseLine->setSuffix("pt");

    if(!_withSubSuperScript)
    {
        m_subScript->setEnabled(false);
        m_superScript->setEnabled(false);
        d->m_relativeSize->setEnabled( false );
        d->m_lRelativeSize->setEnabled( false );
    }

    QHGroupBox* colorsGrp = new QHGroupBox(i18n("Colors"), page);
    lay1->addWidget(colorsGrp);
    //grid = new QGridLayout( colorsGrp, 2, 2, KDialog::marginHint(), KDialog::spacingHint() );
    m_colorButton = new QPushButton( i18n( "Change Co&lor..." ), colorsGrp );
    //grid->addWidget(m_colorButton,0,0);

    m_backGroundColorButton = new QPushButton( i18n( "Change Bac&kground Color..." ), colorsGrp );
    //grid->addWidget(m_backGroundColorButton,0,1);

    connect( m_subScript, SIGNAL(clicked()), this, SLOT( slotSubScriptClicked() ) );
    connect( m_superScript, SIGNAL(clicked()), this, SLOT( slotSuperScriptClicked() ) );
    connect( m_colorButton, SIGNAL(clicked()), this, SLOT( slotChangeColor() ) );

    connect( m_backGroundColorButton,  SIGNAL(clicked()), this, SLOT( slotChangeBackGroundColor() ) );

    connect( m_chooseFont, SIGNAL( fontSelected( const QFont & )),
             this, SLOT( slotFontChanged(const QFont &) ) );

    connect( d->m_relativeSize, SIGNAL( valueChanged(int) ), this, SLOT( slotRelativeSizeChanged( int )));
    connect( d->m_offsetBaseLine, SIGNAL( valueChanged(int) ), this, SLOT( slotOffsetFromBaseLineChanged( int )));

    updatePositionButton();

}

void KoFontChooser::setupTab2()
{
    QWidget *page = new QWidget( this );
    addTab( page, i18n( "Font &Effects" ) );

    //QVBoxLayout *lay1 = new QVBoxLayout( page, KDialog::marginHint(), KDialog::spacingHint() );
    // A single groupbox, without title, looks stupid.
    //QGroupBox *grp = new QGroupBox(page);
    //lay1->addWidget(grp);
    QWidget* grp = page; // just to be able to go back to a groupbox

    QGridLayout *grid = new QGridLayout( grp, 10, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lab = new QLabel( i18n("&Underlining:"), grp);
    grid->addWidget( lab, 0, 0);

    m_underlining = new QComboBox( grp );
    lab->setBuddy( m_underlining );
    grid->addWidget( m_underlining, 1, 0);

    m_underlining->insertStringList( KoTextFormat::underlineStyleList() );

    m_underlineType = new QComboBox(grp );
    grid->addWidget( m_underlineType, 1, 1);
    m_underlineType->insertStringList( KoTextFormat::underlineLineStyleList() );


    m_underlineColorButton = new QPushButton( i18n( "Change Co&lor..." ), grp );
    grid->addWidget(m_underlineColorButton,1,2);


    QLabel * lab2 = new QLabel( i18n("&Strikethrough:"), grp);
    grid->addWidget( lab2, 2, 0);
    d->m_strikeOut = new QComboBox( grp );
    lab2->setBuddy( d->m_strikeOut );
    grid->addWidget( d->m_strikeOut, 3, 0);
    d->m_strikeOut->insertStringList( KoTextFormat::strikeOutStyleList() );


    m_strikeOutType= new QComboBox(grp );
    grid->addWidget( m_strikeOutType, 3, 1);
    m_strikeOutType->insertStringList( KoTextFormat::strikeOutLineStyleList() );

    d->m_shadow= new QCheckBox( i18n("S&hadow"), grp);
    grid->addWidget( d->m_shadow, 4, 0);

    d->m_wordByWord = new QCheckBox( i18n("&Word by word"), grp);
    grid->addWidget( d->m_wordByWord, 5, 0);

#ifdef ATTRCOMBO
    QLabel * lab3 = new QLabel( i18n("A&ttribute:"), grp);
    grid->addWidget( lab3, 6, 0);

    d->m_fontAttribute = new QComboBox( grp );
    lab3->setBuddy( d->m_fontAttribute );
    grid->addWidget( d->m_fontAttribute, 7, 0);

    d->m_fontAttribute->insertStringList( KoTextFormat::fontAttributeList() );
    connect( d->m_fontAttribute, SIGNAL( activated( int ) ), this, SLOT( slotChangeAttributeFont( int )));
#else
    d->m_fontAttribute = new QHButtonGroup( i18n("Capitalization"), grp );
    grid->addMultiCellWidget( d->m_fontAttribute, 6, 6, 0, grid->numCols()-1 );
    QStringList fontAttributes = KoTextFormat::fontAttributeList();
    for( QStringList::Iterator it = fontAttributes.begin(); it != fontAttributes.end(); ++it ) {
        (void) new QRadioButton( *it, d->m_fontAttribute );
    }
    connect( d->m_fontAttribute,  SIGNAL( clicked( int ) ), this, SLOT( slotChangeAttributeFont( int )));
#endif

    QLabel * lab4 = new QLabel( i18n("La&nguage:"), grp);
    grid->addWidget( lab4, 8, 0);

    d->m_language = new QComboBox( grp );
    d->m_language->insertStringList( KoGlobal::listOfLanguages() );
    lab4->setBuddy( d->m_language );
    grid->addWidget( d->m_language, 9, 0 );

    d->m_hyphenation = new QCheckBox( i18n("Auto h&yphenation"), grp );
    grid->addWidget( d->m_hyphenation, 10, 0 );

    // Add one row that can stretch
    grid->expand( grid->numRows() + 1, grid->numCols() );
    grid->setRowStretch( grid->numRows(), 1 );

    connect( d->m_strikeOut, SIGNAL(activated ( int )), this, SLOT( slotStrikeOutTypeChanged( int ) ) );
    connect( m_underlineColorButton, SIGNAL(clicked()), this, SLOT( slotUnderlineColor() ) );
    connect( m_underlining,  SIGNAL( activated ( int  ) ), this, SLOT( slotChangeUnderlining( int )));
    connect( m_strikeOutType,  SIGNAL( activated ( int  ) ), this, SLOT( slotChangeStrikeOutType( int )));
    connect( m_underlineType,  SIGNAL( activated ( int  ) ), this, SLOT( slotChangeUnderlineType( int )));
    connect( d->m_shadow, SIGNAL(clicked()), this, SLOT( slotShadowClicked() ) );
    connect( d->m_wordByWord, SIGNAL(clicked()), this, SLOT( slotWordByWordClicked() ) );
    connect( d->m_language,  SIGNAL( activated ( int  ) ), this, SLOT( slotChangeLanguage( int )));
    connect( d->m_hyphenation, SIGNAL( clicked()), this, SLOT( slotHyphenationClicked()));
}

void KoFontChooser::updatePositionButton()
{
    bool state = (m_superScript->isChecked() || m_subScript->isChecked());
    d->m_relativeSize->setEnabled( state );
    d->m_lRelativeSize->setEnabled( state );
}

bool KoFontChooser::getShadowText()const
{
    return d->m_shadow->isChecked();
}

void KoFontChooser::setShadowText( bool _b)
{
    d->m_shadow->setChecked( _b);
}

void KoFontChooser::setLanguage( const QString & _tag)
{
    d->m_language->setCurrentItem (KoGlobal::languageIndexFromTag( _tag));
}

QString KoFontChooser::getLanguage() const
{
    return KoGlobal::tagOfLanguage( d->m_language->currentText() );
}

KoTextFormat::AttributeStyle KoFontChooser::getFontAttribute()const
{
#ifdef ATTRCOMBO
    int currentItem = d->m_fontAttribute->currentItem ();
#else
    int currentItem = 0;
    for ( int i = 0; i < d->m_fontAttribute->count(); ++i )
    {
        if ( d->m_fontAttribute->find( i )->isOn() )
        {
            currentItem = i;
            break;
        }
    }
#endif
    switch ( currentItem )
    {
    case 0:
        return KoTextFormat::ATT_NONE;
        break;
    case 1:
        return KoTextFormat::ATT_UPPER;
        break;
    case 2:
        return KoTextFormat::ATT_LOWER;
        break;
    case 3:
        return KoTextFormat::ATT_SMALL_CAPS;
        break;
    default:
        return KoTextFormat::ATT_NONE;
    }
}

void KoFontChooser::setFontAttribute( KoTextFormat::AttributeStyle _att)
{
    int currentItem = 0;
    if ( _att == KoTextFormat::ATT_NONE)
        currentItem = 0;
    else if ( _att == KoTextFormat::ATT_UPPER)
        currentItem = 1;
    else if ( _att == KoTextFormat::ATT_LOWER )
        currentItem = 2;
    else if ( _att == KoTextFormat::ATT_SMALL_CAPS )
        currentItem = 3;
#ifdef ATTRCOMBO
    d->m_fontAttribute->setCurrentItem( currentItem );
#else
    d->m_fontAttribute->setButton( currentItem );
#endif
}

bool KoFontChooser::getWordByWord()const
{
    return d->m_wordByWord->isChecked();
}

void KoFontChooser::setWordByWord( bool _b)
{
    d->m_wordByWord->setChecked( _b);
}


double KoFontChooser::getRelativeTextSize()const
{
    return ((double)d->m_relativeSize->value()/100.0);
}

void KoFontChooser::setRelativeTextSize(double _size)
{
    d->m_relativeSize->setValue( (int)(_size * 100) );
}

int KoFontChooser::getOffsetFromBaseLine()const
{
    return d->m_offsetBaseLine->value();
}

void KoFontChooser::setOffsetFromBaseLine(int _offset)
{
    d->m_offsetBaseLine->setValue( _offset );
}

void KoFontChooser::setFont( const QFont &_font, bool _subscript, bool _superscript )
{

    m_newFont = _font;
    kdDebug()<<" setFont m_newFont.bold() :"<<m_newFont.bold()<<" m_newFont.italic():"<<m_newFont.italic()<<endl;
    kdDebug()<<" setfont m_newFont.family() :"<<m_newFont.family()<<endl;

    m_subScript->setChecked( _subscript );
    m_superScript->setChecked( _superscript );

    m_chooseFont->setFont( m_newFont );
    m_changedFlags = 0;
}

void KoFontChooser::setColor( const QColor & col )
{
    d->m_textColor = col;
    if ( col.isValid() )
        m_chooseFont->setColor( col );
    else
        m_chooseFont->setColor( QApplication::palette().color( QPalette::Active, QColorGroup::Text ) );
    m_changedFlags = 0;
}

void KoFontChooser::setUnderlineColor( const QColor & col )
{
    m_underlineColor = col;
    m_changedFlags = 0;
}

void KoFontChooser::setBackGroundColor ( const QColor & col )
{
    m_backGroundColor = col;
    m_changedFlags = 0;
}

void KoFontChooser::slotFontChanged(const QFont & f)
{
    kdDebug()<<" slotFontChanged m_newFont.bold() :"<<f.bold()<<" m_newFont.italic():"<<f.italic()<<endl;
    kdDebug()<<" slotFontChanged m_newFont.family() :"<<f.family()<<endl;

    if ( f.weight() != m_newFont.weight() )
        m_changedFlags |= KoTextFormat::Bold;
    if ( f.italic() != m_newFont.italic() )
        m_changedFlags |= KoTextFormat::Italic;
    if ( f.family() != m_newFont.family() )
        m_changedFlags |= KoTextFormat::Family;
    if ( f.pointSize() != m_newFont.pointSize() )
        m_changedFlags |= KoTextFormat::Size;
    kdDebug(32500) << "KWFontChooser::slotFontChanged m_changedFlags=" << m_changedFlags << endl;
    m_newFont = f;
}

bool KoFontChooser::getHyphenation() const
{
    return d->m_hyphenation->isChecked();
}
void KoFontChooser::setHyphenation( bool _b)
{
    d->m_hyphenation->setChecked( _b);
}

void KoFontChooser::slotHyphenationClicked()
{
    m_changedFlags |= KoTextFormat::Hyphenation;
}

void KoFontChooser::slotStrikeOutTypeChanged( int _val)
{
    m_changedFlags |= KoTextFormat::StrikeOut;
    m_strikeOutType->setEnabled( _val!=0 );
}

void KoFontChooser::slotSubScriptClicked()
{
    if(m_superScript->isChecked())
        m_superScript->setChecked(false);
    m_changedFlags |= KoTextFormat::VAlign;
    updatePositionButton();
}

void KoFontChooser::slotSuperScriptClicked()
{
    if(m_subScript->isChecked())
        m_subScript->setChecked(false);
    m_changedFlags |= KoTextFormat::VAlign;
    updatePositionButton();
}

void KoFontChooser::slotRelativeSizeChanged( int )
{
   m_changedFlags |= KoTextFormat::VAlign;
}

void KoFontChooser::slotOffsetFromBaseLineChanged( int )
{
    m_changedFlags |= KoTextFormat::OffsetFromBaseLine;
}

void KoFontChooser::slotShadowClicked()
{
    m_changedFlags |= KoTextFormat::ShadowText;
}

void KoFontChooser::slotWordByWordClicked()
{
    m_changedFlags |= KoTextFormat::WordByWord;
}

void KoFontChooser::slotChangeAttributeFont( int )
{
    m_changedFlags |= KoTextFormat::Attribute;
}

void KoFontChooser::slotChangeLanguage( int )
{
    m_changedFlags |= KoTextFormat::Language;
}

void KoFontChooser::slotChangeColor()
{
    QColor color = d->m_textColor;
    QColor defaultTextColor = QApplication::palette().color( QPalette::Active, QColorGroup::Text );
    if ( KColorDialog::getColor( color, defaultTextColor ) )
    {
        if ( color != d->m_textColor )
        {
            d->m_textColor = color;
            m_changedFlags |= KoTextFormat::Color;
            if ( color.isValid() )
                m_chooseFont->setColor( color );
            else
                m_chooseFont->setColor( defaultTextColor );
        }
    }
}

void KoFontChooser::slotChangeBackGroundColor()
{
    QColor color = m_backGroundColor;
    if ( KColorDialog::getColor( color, QApplication::palette().color( QPalette::Active, QColorGroup::Base ) ) )
    {
        if ( color != m_chooseFont->color() )
        {
            m_changedFlags |= KoTextFormat::TextBackgroundColor;
            m_backGroundColor = color;
        }
    }
}

void KoFontChooser::slotUnderlineColor()
{
    QColor color = m_underlineColor;
    if ( KColorDialog::getColor( color, QApplication::palette().color( QPalette::Active, QColorGroup::Base ) ) )
    {
        if ( color != m_underlineColor )
        {
            m_changedFlags |= KoTextFormat::ExtendUnderLine;
            m_underlineColor = color;
        }
    }
}

KoTextFormat::UnderlineLineType KoFontChooser::getUnderlineLineType()
{
    switch (m_underlining->currentItem () )
    {
    case 0:
        return KoTextFormat::U_NONE;
        break;
    case 1:
        return KoTextFormat::U_SIMPLE;
        break;
    case 2:
        return KoTextFormat::U_SIMPLE_BOLD;
        break;
    case 3:
        return KoTextFormat::U_DOUBLE;
        break;
    case 4:
        return KoTextFormat::U_WAVE;
        break;
    default:
        return KoTextFormat::U_NONE;
    }

}

KoTextFormat::StrikeOutLineType KoFontChooser::getStrikeOutLineType()
{
    switch (d->m_strikeOut->currentItem () )
    {
    case 0:
        return KoTextFormat::S_NONE;
        break;
    case 1:
        return KoTextFormat::S_SIMPLE;
        break;
    case 2:
        return KoTextFormat::S_SIMPLE_BOLD;
        break;
    case 3:
        return KoTextFormat::S_DOUBLE;
        break;
    default:
        return KoTextFormat::S_NONE;
    }
}


void KoFontChooser::setStrikeOutlineType(KoTextFormat::StrikeOutLineType nb)
{
    switch ( nb )
    {
    case KoTextFormat::S_NONE:
        d->m_strikeOut->setCurrentItem(0);
        break;
    case KoTextFormat::S_SIMPLE:
         d->m_strikeOut->setCurrentItem(1);
        break;
    case KoTextFormat::S_SIMPLE_BOLD:
        d->m_strikeOut->setCurrentItem(2);
        break;
    case KoTextFormat::S_DOUBLE:
         d->m_strikeOut->setCurrentItem(3);
        break;
    }
    m_strikeOutType->setEnabled( d->m_strikeOut->currentItem()!= 0);

    m_changedFlags = 0;
}


void KoFontChooser::setUnderlineLineType(KoTextFormat::UnderlineLineType nb)
{
    switch ( nb )
    {
    case KoTextFormat::U_NONE:
        m_underlining->setCurrentItem(0);
        break;
    case KoTextFormat::U_SIMPLE:
        m_underlining->setCurrentItem(1);
        break;
    case KoTextFormat::U_SIMPLE_BOLD:
        m_underlining->setCurrentItem(2);
        break;
    case KoTextFormat::U_DOUBLE:
        m_underlining->setCurrentItem(3);
        break;
    case KoTextFormat::U_WAVE:
        m_underlining->setCurrentItem(4);
        break;
    default:
        m_underlining->setCurrentItem(0);
        break;
    }
    m_changedFlags = 0;
}

void KoFontChooser::setUnderlineLineStyle(KoTextFormat::UnderlineLineStyle _t)
{
    switch ( _t )
    {
    case KoTextFormat::U_SOLID:
        m_underlineType->setCurrentItem( 0 );
        break;
    case KoTextFormat::U_DASH:
        m_underlineType->setCurrentItem( 1 );
        break;
    case KoTextFormat::U_DOT:
        m_underlineType->setCurrentItem( 2 );
        break;
    case KoTextFormat::U_DASH_DOT:
        m_underlineType->setCurrentItem( 3 );
        break;
    case KoTextFormat::U_DASH_DOT_DOT:
        m_underlineType->setCurrentItem( 4 );
        break;
    default:
        m_underlineType->setCurrentItem( 0 );
        break;
    }
    m_underlineType->setEnabled( m_underlining->currentItem()!= 0);
    m_underlineColorButton->setEnabled( m_underlining->currentItem()!=0);
    m_changedFlags = 0;
}

void KoFontChooser::setStrikeOutLineStyle(KoTextFormat::StrikeOutLineStyle _t)
{
    switch ( _t )
    {
    case KoTextFormat::S_NONE:
        m_strikeOutType->setCurrentItem( 0 );
        break;
    case KoTextFormat::S_DASH:
        m_strikeOutType->setCurrentItem( 1 );
        break;
    case KoTextFormat::S_DOT:
        m_strikeOutType->setCurrentItem( 2 );
        break;
    case KoTextFormat::S_DASH_DOT:
        m_strikeOutType->setCurrentItem( 3 );
        break;
    case KoTextFormat::S_DASH_DOT_DOT:
        m_strikeOutType->setCurrentItem( 4 );
        break;
    default:
        m_strikeOutType->setCurrentItem( 0 );
        break;
    }
    m_changedFlags = 0;
}

KoTextFormat::UnderlineLineStyle KoFontChooser::getUnderlineLineStyle()
{
    switch ( m_underlineType->currentItem() )
    {
    case 0:
        return KoTextFormat::U_SOLID;
        break;
    case 1:
        return KoTextFormat::U_DASH;
        break;
    case 2:
        return KoTextFormat::U_DOT;
        break;
    case 3:
        return KoTextFormat::U_DASH_DOT;
        break;
    case 4:
        return KoTextFormat::U_DASH_DOT_DOT;
        break;
    default:
        return KoTextFormat::U_SOLID;
    }
}

KoTextFormat::StrikeOutLineStyle KoFontChooser::getStrikeOutLineStyle()
{

    switch ( m_strikeOutType->currentItem() )
    {
    case 0:
        return KoTextFormat::S_SOLID;
        break;
    case 1:
        return KoTextFormat::S_DASH;
        break;
    case 2:
        return KoTextFormat::S_DOT;
        break;
    case 3:
        return KoTextFormat::S_DASH_DOT;
        break;
    case 4:
        return KoTextFormat::S_DASH_DOT_DOT;
        break;
    default:
        return KoTextFormat::S_SOLID;
    }
}


void KoFontChooser::slotChangeUnderlineType( int /*i*/)
{
    m_changedFlags |= KoTextFormat::ExtendUnderLine;
}

void KoFontChooser::slotChangeUnderlining( int i)
{
    m_changedFlags |= KoTextFormat::ExtendUnderLine;
    m_underlineType->setEnabled( i!= 0);
    m_underlineColorButton->setEnabled( i!=0);
}

void KoFontChooser::slotChangeStrikeOutType( int /*i*/ )
{
    m_changedFlags |= KoTextFormat::StrikeOut;
}

KoFontDia::KoFontDia( QWidget* parent, const char* name, const QFont &_font,
                      FontAttributeFlags flags,
                      const QColor & color,
                      const QColor & backGroundColor ,
                      const QColor & underlineColor,
                      KoTextFormat::UnderlineLineStyle _underlineLine,
                      KoTextFormat::UnderlineLineType _underlineType,
                      KoTextFormat::StrikeOutLineType _strikeOutType,
                      KoTextFormat::StrikeOutLineStyle _strikeOutLine,
                      KoTextFormat::AttributeStyle _fontAttribute,
                      const QString &_language,
                      double _relativeSize,
                      int _offsetFromBaseLine,
                      bool _withSubSuperScript )
    : KDialogBase( parent, name, true,
                   i18n("Select Font"), Ok|Cancel|User1|Apply, Ok ),
      m_font(_font),
      m_bSubscript( flags & FontAttributeSubscript ),
      m_bSuperscript(flags & FontAttributeSuperScript ),
      m_color( color),
      m_backGroundColor( backGroundColor),
      m_underlineColor( underlineColor ),
      m_underlineType( _underlineType ),
      m_underlineLineStyle( _underlineLine ),
      m_strikeOutLineStyle( _strikeOutLine ),
      m_strikeOutType( _strikeOutType),
      m_bShadowText( flags & FontAttributeShadowText),
      m_relativeSize( _relativeSize ),
      m_offsetBaseLine( _offsetFromBaseLine),
      m_bWordByWord( flags & FontAttributeWordByWord ),
      m_bHyphenation( flags & FontAttributeHyphenation),
      m_fontAttribute( _fontAttribute),
      m_language ( _language )
{
    setButtonText( KDialogBase::User1, i18n("&Reset") );

    m_chooser = new KoFontChooser( this, "kofontchooser", _withSubSuperScript, KFontChooser::SmoothScalableFonts);
    setMainWidget( m_chooser );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotReset()));

    slotReset();
}

void KoFontDia::slotApply()
{
    emit applyFont();
}

void KoFontDia::slotOk()
{
    slotApply();
    KDialogBase::slotOk();
}

void KoFontDia::slotReset()
{
    m_chooser->setFont( m_font, m_bSubscript, m_bSuperscript );
    m_chooser->setColor( m_color );
    m_chooser->setBackGroundColor(m_backGroundColor);
    m_chooser->setUnderlineColor( m_underlineColor );

    m_chooser->setUnderlineLineType(m_underlineType);
    m_chooser->setUnderlineLineStyle(m_underlineLineStyle);

    m_chooser->setStrikeOutlineType( m_strikeOutType);
    m_chooser->setStrikeOutLineStyle(m_strikeOutLineStyle);
    m_chooser->setShadowText( m_bShadowText);
    m_chooser->setWordByWord( m_bWordByWord );
    m_chooser->setRelativeTextSize( m_relativeSize);
    m_chooser->setOffsetFromBaseLine( m_offsetBaseLine );
    m_chooser->setFontAttribute( m_fontAttribute );
    m_chooser->setLanguage( m_language );
    m_chooser->updatePositionButton();
}

#include "koFontDia.moc"
