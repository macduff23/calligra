/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#ifndef __KSCRIPT_EXT_QPUSHBUTTON_H__
#define __KSCRIPT_EXT_QPUSHBUTTON_H__

#include "kscript_class.h"
#include "kscript_ext_qbutton.h"

class QPushButton;
class KSContext;

class KSClass_QPushButton : public KSClass_QButton
{
public:
  KSClass_QPushButton( KSModule*, const char* name = "QPushButton" );

protected:
  virtual KSScriptObject* createObject( KSClass* c );
};

class KSObject_QPushButton : public KSObject_QButton
{
public:
  KSObject_QPushButton( KSClass* );

  bool ksQPushButton( KSContext& );

  KSValue::Ptr member( KSContext& context, const QString& name );
  bool setMember( KSContext& context, const QString& name, const KSValue::Ptr& v );

  bool inherits( const char* name ) { return ( strcmp( name, "KSObject_QPushButton" ) == 0 || KSObject_QButton::inherits( name ) ); }

  static QPushButton* convert( KSValue* v ) { return (QPushButton*) ((KS_Qt_Object*)v->objectValue())->object(); }

  void setObject( QObject* obj );
};

#endif
