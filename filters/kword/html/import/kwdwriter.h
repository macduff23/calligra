/***************************************************************************
                          kwdwriter.h  -  description
                             -------------------
    begin                : Wed Sep 5 2001
    copyright            : (C) 2001 by Frank Dekervel
    email                : Frank.Dekervel@student.kuleuven.ac.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by                                                          *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWDWRITER_H
#define KWDWRITER_H


#include <qrect.h>
#include <koStore.h>
#include <qdom.h>

/**
  *@author Frank Dekervel
  */

class KWDWriter {
public: 
	KWDWriter(KoStore *store);
	~KWDWriter();

	
	/**
	 * writes the document to the koStore
	 **/
	bool writeDoc();
	
	
	/**
	 * adds a frameset to parent FIXME
	 **/
	QDomElement addFrameSet(QDomElement parent, int frametype=1,
				   int frameinfo=0, QString name=QString::null,
				   int visible=1);
	/**
	 * creates a table
	 **/
	int createTable();

	
	/**
	 * creates a table cell
	 **/
	QDomElement createTableCell(int tableno, int nrow,
				int ncol, int colspan, int x, int y, int right, int bottom);
	
				
				
	/**
	 * fetches the cell of a table
	 **/			
	QDomElement fetchTableCell(int tableno, int rowno, int colno);

	
	/**
	 * finishes a table
	 * if the arguments x,y,w,h are given, each cell is resized to have a 'right'
	 * table. otherwise, the cell sizes are not touched.
	 **/
	void finishTable(int tableno, int x=-1, int y=-1, int w=-1, int h=-1);
	
	/**
	 * inlines something in a paragraph
	 * @parm paragraph: the paragraph the anchor should be placed in
	 * @parm toInLline: the element that should be inlined
	 **/
	QDomElement createInline(QDomElement paragraph, QDomElement toInline);
	
	
	/**
	 *
	 **/
	QDomElement currentLayout(QDomElement paragraph);
	
	/**
	 * adds a frame to frameset FIXME
	 **/
	QDomElement addFrame(QDomElement frameset, int runaround=0, int copy=0,
                                int top=42, int left=28, int bottom=799, int right=567,
                                int newFrameBehaviour=0, int runaroundGap=2 );
	
	
	/**
	 * adds a paragraph
	 **/
	QDomElement addParagraph(QDomElement parent);
	QDomElement addParagraph(QDomElement parent, QDomElement layout);
	
	/**
	 * adds/changes an attribute to/of the current format
	 **/
        QDomElement formatAttribute(QDomElement paragraph, QString name, QString attrName, QString attr);

        /**
         * get a layout attribute
         **/
	QString getLayoutAttribute(QDomElement paragraph, QString name, QString attrName);


        /**
         * adds/changes an attribute to/of the current layout
         **/
        QDomElement layoutAttribute(QDomElement paragraph, QString name, QString attrName, QString attr);

        /**
         * creates a new format in the current paragraph. do this before adding text
         * FIXME: you can only do addText once per format
         **/
        QDomElement startFormat(QDomElement paragraph);
        QDomElement startFormat(QDomElement paragraph, QDomElement formatToClone);


	/**
	 * cleans up the current paragraph (throw away unused formats)
	 * FIXME: find a better solution
	 **/
	 void cleanUpParagraph(QDomElement paragraph);
	
	/**
	 * adds some text to the current format in this paragraph
	 **/
	void addText(QDomElement paragraph, QString text, int format_id=1);
	
	/**
	 * returns the current format
	 * if start_new_one is true, a new format will be started if needed
	 **/
	QDomElement currentFormat(QDomElement paragraph, bool start_new_one=false);
	
	/**
	 * FIXME: remove these
	 **/
        QDomElement currentFrameset();
        QDomElement currentParagraph(QDomElement frameset);
	void setLayout(QDomElement paragraph, QDomElement layout);
	QString getText(QDomElement paragraph);

        /**
         * returns the document root
         **/

        QDomElement docroot();

private:
	/**
	 * creates a rectangle
	 **/
	void addRect(QDomElement e,int x, int y, int w, int h);
	void addRect(QDomElement e, QRect rect);

       	
protected:
	KoStore *_store;
	QDomDocument *_doc;
	int tableNo;
	
};

#endif
