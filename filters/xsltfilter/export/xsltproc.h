/* This file is part of the KDE project
   Copyright (C) 2000 Robert JACOLIN <rjacolin@ifrance.com>

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

#ifndef __XSLTPROC_H__
#define __XSLTPROC_H__

#include <libexslt/exslt.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/extensions.h>

class XSLTProc
{
	const char* _fileIn;
	const char* _fileOut;
	const char* _stylesheet;


	
	public:
		/**
		 * Create a new xslt processor 
		 * @param fileIn file to parse.
		 * @param fileOut file to generate.
		 * @param stylesheet xsl file to convert fileIn in fileOut.
		 */
    	XSLTProc(const char* fileIn, const char* fileOut, const char *stylesheet);

		/**
		 * Destroy the processor
		 */
	    virtual ~XSLTProc() {}

		/** Process fileIn throught the stylesheet.
		 * @return return 0 if no error occured
		 */
		int parse();

	private:
		static void xsltProcess(xmlDocPtr doc, xsltStylesheetPtr cur,
					const char *filename);
};
#endif /* __XSLTPROC_H__ */
