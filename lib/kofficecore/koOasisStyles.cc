#include "koOasisStyles.h"
#include <kdebug.h>
#include <qdom.h>

KoOasisStyles::KoOasisStyles()
{

}

KoOasisStyles::~KoOasisStyles()
{

}

void KoOasisStyles::createStyleMap( const QDomDocument& doc )
{
    QDomElement docElement  = doc.documentElement();
    QDomNode docStyles   = docElement.namedItem( "office:document-styles" );
    // We used to have the office:version check here, but better let the apps do that
    QDomNode fontStyles = docElement.namedItem( "office:font-decls" );

    if ( !fontStyles.isNull() ) {
        //kdDebug(30003) << "Starting reading in font-decl..." << endl;

        insertStyles( fontStyles.toElement() );
    }// else
    //   kdDebug(30003) << "No items found" << endl;

    //kdDebug(30003) << "Starting reading in office:automatic-styles" << endl;

    QDomNode autoStyles = docElement.namedItem( "office:automatic-styles" );
    if ( !autoStyles.isNull() ) {
        insertStyles( autoStyles.toElement() );
    }// else
    //    kdDebug(30003) << "No items found" << endl;


    //kdDebug(30003) << "Reading in master styles" << endl;

    QDomNode masterStyles = docElement.namedItem( "office:master-styles" );

    if ( !masterStyles.isNull() ) {
        QDomElement master = masterStyles.firstChild().toElement();
        for ( ; !master.isNull() ; master = master.nextSibling().toElement() ) {
            if ( master.tagName() ==  "style:master-page" ) {
                QString name = master.attribute( "style:name" );
                //kdDebug(30003) << "Master style: '" << name << "' loaded " << endl;
                m_masterPages.insert( name, new QDomElement( master ) );
            } else
                kdWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles" << endl;
        }
    }


    //kdDebug(30003) << "Starting reading in office:styles" << endl;

    QDomElement officeStyle = docElement.namedItem( "office:styles" ).toElement();

    if ( !officeStyle.isNull() ) {
        m_officeStyle = officeStyle;
        insertStyles( m_officeStyle );
        insertDrawStyles( m_officeStyle );

    }

    //kdDebug(30003) << "Styles read in." << endl;
}

QValueVector<QDomElement> KoOasisStyles::userStyles() const
{
    QValueVector<QDomElement> vec;
    // Collect user styles
    QDomNodeList children = m_officeStyle.elementsByTagName( "style:style" );
    vec.resize( children.length() );
    for ( unsigned int i = 0 ; i < children.length(); ++i )
        vec[i] = children.item( i ).toElement();
    return vec;
}

void KoOasisStyles::insertDrawStyles( const QDomElement& styles )
{
    for ( QDomNode n = styles.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        QString tagName = e.tagName();
        if ( e.hasAttribute( "draw:name" ) )
        {
            QString name = e.attribute( "style:name" );
            QDomElement* ep = new QDomElement( e );
            m_drawStyles.insert( name, ep );
        }
    }
}


void KoOasisStyles::insertStyles( const QDomElement& styles )
{
    //kdDebug(30003) << "Inserting styles from " << styles.tagName() << endl;
    for ( QDomNode n = styles.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        QString tagName = e.tagName();

        QString name = e.attribute( "style:name" );
        if ( tagName == "style:style"
             || tagName == "style:page-master"
             || tagName == "style:font-decl" )
        {
            QDomElement* ep = new QDomElement( e );
            m_styles.insert( name, ep );
            //kdDebug(30003) << "Style: '" << name << "' loaded " << endl;
        } else if ( tagName == "style:default-style" ) {
            m_defaultStyle = e;
        } else if ( tagName == "text:list-style" ) {
            QDomElement* ep = new QDomElement( e );
            m_listStyles.insert( name, ep );
            //kdDebug(30003) << "List style: '" << name << "' loaded " << endl;
        } else if ( tagName == "number:number-style" ) {
            // TODO
        } else if ( tagName == "number:date-style"
                    || tagName == "number:time-style" ) {
            importDateTimeStyle( e );
        }
        // The rest (*-configuration and outline-style) is to be done by the apps.
    }
}

// OO spec 2.5.4. p68. Conversion to Qt format: see qdate.html
// OpenCalcImport::loadFormat has similar code, but slower, intermixed with other stuff,
// lacking long-textual forms.
void KoOasisStyles::importDateTimeStyle( const QDomElement& parent )
{
    QString format;
    for( QDomNode node( parent.firstChild() ); !node.isNull(); node = node.nextSibling() )
    {
        const QDomElement e( node.toElement() );
        QString tagName = e.tagName();
        if ( !tagName.startsWith( "number:" ) )
            continue;
        tagName.remove( 0, 7 );
        const QString numberStyle = e.attribute( "number:style" );
        const bool shortForm = numberStyle == "short" || numberStyle.isEmpty();
        if ( tagName == "day" ) {
            format += shortForm ? "d" : "dd";
        } else if ( tagName == "day-of-week" ) {
            format += shortForm ? "ddd" : "dddd";
        } else if ( tagName == "month" ) {
            // TODO the spec has a strange mention of number:format-source
            if ( e.attribute( "number:textual" ) == "true" ) {
                format += shortForm ? "MMM" : "MMMM";
            } else { // month number
                format += shortForm ? "M" : "MM";
            }
        } else if ( tagName == "year" ) {
            format += shortForm ? "yy" : "yyyy";
        } else if ( tagName == "week-of-year" || tagName == "quarter") {
            // ### not supported in Qt
        } else if ( tagName == "hours" ) {
            format += shortForm ? "h" : "hh";
        } else if ( tagName == "minutes" ) {
            format += shortForm ? "m" : "mm";
        } else if ( tagName == "seconds" ) {
            format += shortForm ? "s" : "ss";
        } else if ( tagName == "am-pm" ) {
            format += "ap";
        } else if ( tagName == "text" ) { // litteral
            format += e.text();
        } // TODO number:decimal-places
    }

#if 0
    // QDate doesn't work both ways!!! It can't parse something back from
    // a string and a format (e.g. 01/02/03 and dd/MM/yy, it will assume MM/dd/yy).
    // So we also need to generate a KLocale-like format, to parse the value
    // Update: we don't need to parse the date back.

    QString kdeFormat;
    for( QDomNode node( parent.firstChild() ); !node.isNull(); node = node.nextSibling() )
    {
        const QDomElement e( node.toElement() );
        QString tagName = e.tagName();
        if ( !tagName.startsWith( "number:" ) )
            continue;
        tagName.remove( 0, 7 );
        const QString numberStyle = e.attribute( "number:style" );
        const bool shortForm = numberStyle == "short" || numberStyle.isEmpty();
        if ( tagName == "day" ) {
            kdeFormat += shortForm ? "%e" : "%d";
        } else if ( tagName == "day-of-week" ) {
            kdeFormat += shortForm ? "%a" : "%A";
        } else if ( tagName == "month" ) {
            // TODO the spec has a strange mention of number:format-source
            if ( e.attribute( "number:textual" ) == "true" ) {
                kdeFormat += shortForm ? "%b" : "%B";
            } else { // month number
                kdeFormat += shortForm ? "%n" : "%m";
            }
        } else if ( tagName == "year" ) {
            kdeFormat += shortForm ? "%y" : "%Y";
        } else if ( tagName == "week-of-year" || tagName == "quarter") {
            // ### not supported in KLocale
        } else if ( tagName == "hours" ) {
            kdeFormat += shortForm ? "%k" : "%H"; // TODO should depend on presence of am/pm
        } else if ( tagName == "minutes" ) {
            kdeFormat += shortForm ? "%M" : "%M"; // KLocale doesn't have 1-digit minutes
        } else if ( tagName == "seconds" ) {
            kdeFormat += shortForm ? "%S" : "%S"; // KLocale doesn't have 1-digit seconds
        } else if ( tagName == "am-pm" ) {
            kdeFormat += "%p";
        } else if ( tagName == "text" ) { // litteral
            kdeFormat += e.text();
        } // TODO number:decimal-places
    }
#endif

    QString styleName = parent.attribute( "style:name" );
    kdDebug(30518) << "datetime style: " << styleName << " qt format=" << format << endl;
    m_dateTimeFormats.insert( styleName, format );
}
