#ifndef USERFONT_H
#define USERFONT_H

class KWUserFont;
class KWDisplayFont;
class KWordDocument_impl;

#include "defs.h"

#include <qfont.h>
#include <qfontmet.h>
#include <qlist.h>
#include <qwidget.h>
#include <qstring.h>
#include <qfontmetrics.h>

class KWUserFont
{
 public:
    KWUserFont( KWordDocument_impl *_doc, const char *_name );
    ~KWUserFont();
    
    const char *getFontName() {	return fontName.data(); }

    bool operator==(KWUserFont &_font)
    { return _font.getFontName() == fontName; }
    bool operator!=(KWUserFont &_font)
    { return _font.getFontName() != fontName; }
    
protected:
    QString fontName;
    
    KWordDocument_impl *document;
};

class KWDisplayFont : public QFont
{
public:
    KWDisplayFont( KWordDocument_impl *_doc, KWUserFont *_font, unsigned int _size,
		   int _weight, bool _italic );
    ~KWDisplayFont();
    
    void scaleFont();

    static void scaleAllFonts();

    KWUserFont* getUserFont() { return userFont; }
    unsigned int getPTSize() { return ptSize; }

    unsigned int getPTAscender() { return fm.ascent(); }
    unsigned int getPTDescender() { return fm.descent() + 2; }

    unsigned int getPTWidth( const char *_text );
    unsigned int getPTWidth( char &_c );
    
    void setPTSize(int _size);
    void setWeight(int _weight);
    void setItalic(bool _italic);

protected:
    QFontMetrics fm;

    unsigned int ptSize;

    /**
     * Pointer to the user font family this font belongs to.
     */
    KWUserFont *userFont;
    /**
     * Poiner to the document this font belongs to.
     */
    KWordDocument_impl *document;

};

#endif


