/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kspread_value.h"

#include <float.h>
#include <math.h>
#include <limits.h>

#include <qstring.h>
#include <qtextstream.h>


// helper class for array implementation
class ValueArray
{
public:
  KSpreadValue** ptr;
  unsigned columns;
  unsigned rows;
  
  ValueArray(): ptr(0), columns(0), rows(0) {};
  
  ~ValueArray()
  { 
    clear();
   };  
   
  ValueArray( const ValueArray& va )
  {
    operator=( va );
  }
  
  ValueArray& operator= ( const ValueArray& va )
  {
    init( va.columns, va.rows );
    unsigned count = columns * rows;
    for( unsigned i = 0; i < count; i++ )
      if( va.ptr[i] )
        ptr[i] = new KSpreadValue( *va.ptr[i] );
    return *this;
  }
   
  void clear()
  {
    if( !ptr ) return;
    unsigned count = columns * rows;
    if( !count ) return;
    for( unsigned i = 0; i < count; i++ )
      delete ptr[i];
    delete [] ptr;
    ptr = 0;
  }
  
  void init( unsigned c, unsigned r )
  {
    if( ptr ) clear();
    columns = c; rows = r;
    unsigned count = columns * rows;
    ptr = new KSpreadValue* [count];
    for( unsigned i = 0; i < count; i++ )
      ptr[i] = (KSpreadValue*)0;
  }
  
  KSpreadValue* at( unsigned c, unsigned r )
  {
    if( !ptr ) return 0;
    if( c >= columns ) return 0;
    if( r >= rows ) return 0;
    return ptr[r*columns+c];
  };
  
  void set( unsigned c, unsigned r, KSpreadValue* v )
  {
    if( !ptr ) return;
    if( c >= columns ) return;
    if( r >= rows ) return;
    delete ptr[r*columns+c];
    ptr[r*columns+c] = v;
  }
  
};


// helper class for KSpreadValue
class KSpreadValueData
{
  public:

    KSpreadValue::Type type:4;
    KSpreadValue::Format format:4;

    // reference count, at least one when object exists
    unsigned int count:24;
    
    union
    {
      bool b;
      long i;
      double f;
      QString* ps;
      ValueArray* pa;
    };
    
    // create empty data
    KSpreadValueData(): type( KSpreadValue::Empty ),
      count( 1 ), ps( 0 ) { };

    // destroys data
    ~KSpreadValueData(){ if( this == s_null ) s_null = 0;
       if( type == KSpreadValue::Array ) delete pa;
       if( type == KSpreadValue::String ) delete ps;
       if( type == KSpreadValue::Error ) delete ps;
     }

    // static empty data to be shared
    static KSpreadValueData* null()
      { if( !s_null) s_null = new KSpreadValueData; else s_null->ref(); return s_null; }
      
    // increase reference count
    void ref() { count++; }

    // decrease reference count
    void unref()
      {  --count; if( !count ) delete this; }

    // true if it's null (which is shared)
    bool isNull(){ return this == s_null; }

    /** set most probable formatting based on the type */
    void setFormatByType ();
    
  private:

    static KSpreadValueData* s_null;
};

void KSpreadValueData::setFormatByType ()
{
  switch (type) {
    case KSpreadValue::Empty:
      format = KSpreadValue::fmt_None;
    break;
    case KSpreadValue::Boolean:
      format = KSpreadValue::fmt_Boolean;
    break;
    case KSpreadValue::Integer:
      format = KSpreadValue::fmt_Number;
    break;
    case KSpreadValue::Float:
      format = KSpreadValue::fmt_Number;
    break;
    case KSpreadValue::String:
      format = KSpreadValue::fmt_String;
    break;
    case KSpreadValue::Array:
      format = KSpreadValue::fmt_None;
    break;
    case KSpreadValue::CellRange:
      format = KSpreadValue::fmt_None;
    break;
    case KSpreadValue::Error:
      format = KSpreadValue::fmt_String;
    break;
  };
}

// to be shared between all empty value
KSpreadValueData* KSpreadValueData::s_null = 0;

// static things
KSpreadValue ks_value_empty;
KSpreadValue ks_error_div0;
KSpreadValue ks_error_na;
KSpreadValue ks_error_name;
KSpreadValue ks_error_null;
KSpreadValue ks_error_num;
KSpreadValue ks_error_ref;
KSpreadValue ks_error_value;

// create an empty value
KSpreadValue::KSpreadValue()
{
  d = KSpreadValueData::null();
}

// destructor
KSpreadValue::~KSpreadValue()
{
  d->unref();
}

// create value of certain type
KSpreadValue::KSpreadValue( KSpreadValue::Type _type )
{
  d = new KSpreadValueData;
  d->type = _type;
  d->setFormatByType ();
}

// copy constructor
KSpreadValue::KSpreadValue( const KSpreadValue& _value )
{
  d = KSpreadValueData::null();
  assign( _value );
}

// assignment operator
KSpreadValue& KSpreadValue::operator=( const KSpreadValue& _value )
{
  return assign( _value );
}

// create a boolean value
KSpreadValue::KSpreadValue( bool b )
{
  d = KSpreadValueData::null();
  setValue( b );
}

// create an integer value
KSpreadValue::KSpreadValue( long i )
{
  d = KSpreadValueData::null();
  setValue ( i );
}

// create an integer value
KSpreadValue::KSpreadValue( int i )
{
  d = KSpreadValueData::null();
  setValue ( i );
}

// create a floating-point value
KSpreadValue::KSpreadValue( double f )
{
  d = KSpreadValueData::null();
  setValue( f );
}

// create a string value
KSpreadValue::KSpreadValue( const QString& s )
{
  d = KSpreadValueData::null();
  setValue( s );
}

// create a floating-point value from date/time
KSpreadValue::KSpreadValue( const QDateTime& dt )
{
  d = KSpreadValueData::null();
  setValue( dt );
}

// create a floating-point value from time
KSpreadValue::KSpreadValue( const QTime& dt )
{
  d = KSpreadValueData::null();
  setValue( dt );
}

// create a floating-point value from date
KSpreadValue::KSpreadValue( const QDate& dt )
{
  d = KSpreadValueData::null();
  setValue( dt );
}

// create an array value
KSpreadValue::KSpreadValue( unsigned columns, unsigned rows )
{
  d = new KSpreadValueData;
  d->type = Array;
  d->format = fmt_None;
  d->pa = new ValueArray;
  d->pa->init( columns, rows );
}

// assign value from other
// shallow copy: only copy the data pointer
KSpreadValue& KSpreadValue::assign( const KSpreadValue& _value )
{
  d->unref();
  d = _value.d;
  d->ref();
  return *this;
}

// return type of the value
KSpreadValue::Type KSpreadValue::type() const
{
  return d ? d->type : Empty;
}

// set the value to boolean
void KSpreadValue::setValue( bool b )
{
  detach();
  d->type = Boolean;
  d->b = b;
  d->format = fmt_Boolean;
}

// get the value as boolean
bool KSpreadValue::asBoolean() const
{
  bool result = false;

  if( type() == KSpreadValue::Boolean )
    result = d->b;

  return result;
}

// set the value to integer
void KSpreadValue::setValue( long i )
{
  detach();
  d->type = Integer;
  d->i = i;
  d->format = fmt_Number;
}

// set the value to integer
void KSpreadValue::setValue( int i )
{
  detach();
  d->type = Integer;
  d->i = static_cast<long>( i );
  d->format = fmt_Number;
}

// get the value as integer
long KSpreadValue::asInteger() const
{
  long result = 0;

  if( type() == KSpreadValue::Integer )
    result = d->i;

  if( type() == KSpreadValue::Float )
    result = static_cast<int>(d->f);

  return result;
}

void KSpreadValue::setValue( const KSpreadValue& v )
{
  assign( v );
}

// set the value as floating-point
void KSpreadValue::setValue( double f )
{
  detach();
  d->type = Float;
  d->f = f;
  d->format = fmt_Number;
}

// get the value as floating-point
double KSpreadValue::asFloat() const
{
  double result = 0.0;

  if( type() == KSpreadValue::Float )
    result = d->f;

  if( type() == KSpreadValue::Integer )
    result = static_cast<double>(d->i);

  return result;
}

// set the value as string
void KSpreadValue::setValue( const QString& s )
{
  detach();
  d->type = String;
  d->ps = new QString( s );
  d->format = fmt_String;
}

// get the value as string
QString KSpreadValue::asString() const
{
  QString result;

  if( type() == KSpreadValue::String )
  if( d->ps )
    result = QString( *d->ps );

  return result;
}

// set error message
void KSpreadValue::setError( const QString& msg )
{
  detach();
  d->type = Error;
  d->ps = new QString( msg );
}

// get error message
QString KSpreadValue::errorMessage() const
{
  QString result;

  if( type() == KSpreadValue::Error )
  if( d->ps )
    result = QString( *d->ps );

  return result;
}

// set the value as date/time
// NOTE: date/time is stored as serial number
// Day 61 means 1st of March, 1900
void KSpreadValue::setValue( const QDateTime& dt )
{
  // reference is 31 Dec, 1899 midnight
  QDate refDate( 1899, 12, 31 );
  QTime refTime( 0, 0 );

  double f = refDate.daysTo( dt.date() ) + 1.0;
  f += refTime.secsTo( dt.time() ) / 86400.0;

  setValue( f );
  d->format = fmt_DateTime;
}

void KSpreadValue::setValue( const QTime& time )
{
  // reference time is midnight
  QTime refTime( 0, 0 );
  double f = refTime.msecsTo( time ) / 86400000.0;

  setValue( f );
  d->format = fmt_Time;
}

void KSpreadValue::setValue( const QDate& date )
{
  // reference date is 31 Dec, 1899
  QDate refDate = QDate( 1899, 12, 31 );
  double f = refDate.daysTo( date ) + 1.0;

  setValue( f );
  d->format = fmt_Date;
}

// get the value as date/time
QDateTime KSpreadValue::asDateTime() const
{
  return QDateTime( asDate(), asTime() );
}

// get the value as date
QDate KSpreadValue::asDate() const
{
  QDate dt( 1899, 12, 30 );

  double f = asFloat();
  dt = dt.addDays( (int) f );
  
  return dt;
}

// get the value as time
QTime KSpreadValue::asTime() const
{
  QTime dt;
  
  double f = asFloat();
  dt = dt.addMSecs( qRound( (f-(int)f) * 86400 * 1000 ) );
  
  return dt;
}

KSpreadValue::Format KSpreadValue::format() const
{
  return d ? d->format : fmt_None;
}

void KSpreadValue::setFormat (KSpreadValue::Format fmt)
{
  d->format = fmt;
}

KSpreadValue KSpreadValue::element( unsigned column, unsigned row ) const
{
  if( d->type != Array ) return empty();
  if( !d->pa ) return empty();
  KSpreadValue* v = d->pa->at( column, row );
  return v ? KSpreadValue( *v ) : empty();
}

void KSpreadValue::setElement( unsigned column, unsigned row, const KSpreadValue& v )
{
  if( d->type != Array ) return;
  if( !d->pa ) return;
  detach();
  d->pa->set( column, row, new KSpreadValue( v ) );
}

unsigned KSpreadValue::columns() const
{
  if( d->type != Array ) return 0;
  if( !d->pa ) return 0;
  return d->pa->columns;
}

unsigned KSpreadValue::rows() const
{
  if( d->type != Array ) return 0;
  if( !d->pa ) return 0;
  return d->pa->rows;
}

// reference to empty value
const KSpreadValue& KSpreadValue::empty()
{
  return ks_value_empty;
}

// reference to #DIV/0! error
const KSpreadValue& KSpreadValue::errorDIV0()
{
  if( !ks_error_div0.isError() )
    ks_error_div0.setError( "#DIV/0!" );
  return ks_error_div0;
}

// reference to #N/A error
const KSpreadValue& KSpreadValue::errorNA()
{
  if( !ks_error_na.isError() )
    ks_error_na.setError( "#N/A" );
  return ks_error_na;
}

// reference to #NAME? error
const KSpreadValue& KSpreadValue::errorNAME()
{
  if( !ks_error_name.isError() )
    ks_error_name.setError( "#NAME?" );
  return ks_error_name;
}

// reference to #NUM! error
const KSpreadValue& KSpreadValue::errorNUM()
{
  if( !ks_error_num.isError() )
    ks_error_num.setError( "#NUM!" );
  return ks_error_num;
}

// reference to #NULL! error
const KSpreadValue& KSpreadValue::errorNULL()
{
  if( !ks_error_null.isError() )
    ks_error_null.setError( "#NULL!" );
  return ks_error_null;
}

// reference to #REF! error
const KSpreadValue& KSpreadValue::errorREF()
{
  if( !ks_error_ref.isError() )
    ks_error_ref.setError( "#REF!" );
  return ks_error_ref;
}

// reference to #VALUE! error
const KSpreadValue& KSpreadValue::errorVALUE()
{
  if( !ks_error_value.isError() )
    ks_error_value.setError( "#VALUE!" );
  return ks_error_value;
}

// detach, create deep copy of KSpreadValueData
void KSpreadValue::detach()
{
  if( d->isNull() || ( d->count > 1 ) )
  {
    KSpreadValueData* n;
    n = new KSpreadValueData;

    n->type = d->type;
    switch( n->type )
    {
    case Empty: break;
    case Boolean: n->b = d->b; break;
    case Integer: n->i = d->i; break;
    case Float:   n->f = d->f; break;
    case String:  n->ps = new QString( *d->ps ); break;
    case Array:   n->pa = new ValueArray; *n->pa = (*d->pa); break;
    case Error:   n->ps = new QString( *d->ps ); break;
    default: break;
    }

    d->unref();
    d = n;
  }
}

KSpreadValue KSpreadValue::add( const KSpreadValue& v1, const KSpreadValue& v2 )
{
  if( !v1.isNumber() ) return KSpreadValue::errorVALUE();
  if( !v2.isNumber() ) return KSpreadValue::errorVALUE();
  double result = v1.asFloat() + v2.asFloat();
  
  if( v1.isInteger() && v2.isInteger() ) 
  if( result < (double)INT_MAX )
  if( result > (double)-INT_MAX )
    return KSpreadValue( static_cast<long>( result ) );
    
  return KSpreadValue( result );
}

KSpreadValue& KSpreadValue::add( const KSpreadValue& v )
{
  setValue( add( *this, v ) );
  return *this;
}

KSpreadValue KSpreadValue::sub( const KSpreadValue& v1, const KSpreadValue& v2 )
{
  if( !v1.isNumber() ) return KSpreadValue::errorVALUE();
  if( !v2.isNumber() ) return KSpreadValue::errorVALUE();
  double result = v1.asFloat() - v2.asFloat();
  
  if( v1.isInteger() && v2.isInteger() ) 
  if( result < (double)INT_MAX )
  if( result > (double)-INT_MAX )
    return KSpreadValue( static_cast<long>( result ) );
    
  return KSpreadValue( result );
}

KSpreadValue& KSpreadValue::sub( const KSpreadValue& v )
{
  setValue( sub( *this, v ) );
  return *this;
}

KSpreadValue KSpreadValue::mul( const KSpreadValue& v1, const KSpreadValue& v2 )
{
  if( !v1.isNumber() ) return KSpreadValue::errorVALUE();
  if( !v2.isNumber() ) return KSpreadValue::errorVALUE();
  double result = v1.asFloat() * v2.asFloat();
  
  if( v1.isInteger() && v2.isInteger() ) 
  if( result < (double)INT_MAX )
  if( result > (double)-INT_MAX )
    return KSpreadValue( static_cast<long>( result ) );
      
  return KSpreadValue( result );
}

KSpreadValue& KSpreadValue::mul( const KSpreadValue& v )
{
  setValue( mul( *this, v ) );
  return *this;
}

KSpreadValue KSpreadValue::div( const KSpreadValue& v1, const KSpreadValue& v2 )
{
  if( !v1.isNumber() ) return KSpreadValue::errorVALUE();
  if( !v2.isNumber() ) return KSpreadValue::errorVALUE();
  
  if( KSpreadValue::isZero( v2.asFloat() ) )
    return KSpreadValue::errorDIV0();  
  double result = v1.asFloat() / v2.asFloat();
  
  if( v1.isInteger() && v2.isInteger() ) 
  if( result < (double)INT_MAX )
  if( result > (double)-INT_MAX )
  if( floor( result ) == result )
    return KSpreadValue( static_cast<long>( result ) );
    
  return KSpreadValue( result );
}

KSpreadValue& KSpreadValue::div( const KSpreadValue& v )
{
  setValue( div( *this, v ) );
  return *this;
}

KSpreadValue KSpreadValue::pow( const KSpreadValue& v1, const KSpreadValue& v2 )
{
  if( !v1.isNumber() ) return KSpreadValue::errorVALUE();
  if( !v2.isNumber() ) return KSpreadValue::errorVALUE();
  double result = ::pow( v1.asFloat(), v2.asFloat() );
  
  if( v1.isInteger() && v2.isInteger() ) 
  if( result < (double)INT_MAX )
  if( result > (double)-INT_MAX )
  if( floor( result ) == result )
    return KSpreadValue( static_cast<long>( result ) );
    
  return KSpreadValue( result );
}

KSpreadValue& KSpreadValue::pow( const KSpreadValue& v )
{
  setValue( KSpreadValue::pow( *this, v ) );
  return *this;
}

int KSpreadValue::compare( double v1, double v2 )
{
  double v3 = v1 - v2;
  if( v3 > DBL_EPSILON ) return 1;
  if( v3 < DBL_EPSILON ) return 1;
  return 0;
}

bool KSpreadValue::isZero( double v )
{
  return fabs( v ) < DBL_EPSILON;
}

bool KSpreadValue::isZero() const
{
  if( !isNumber() ) return false;
  return isZero( asFloat() );
}

bool KSpreadValue::allowComparison( const KSpreadValue& v ) const
{
  KSpreadValue::Type t1 = d->type;
  KSpreadValue::Type t2 = v.type();
  
  if( ( t1 == Empty ) && ( t2 == Empty ) ) return true;
  if( ( t1 == Empty ) && ( t2 == String ) ) return true;
  
  if( ( t1 == Boolean ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == Float ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == String ) ) return true;
    
  if( ( t1 == Integer ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Integer ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Integer ) && ( t2 == Float ) ) return true;
  if( ( t1 == Integer ) && ( t2 == String ) ) return true;
  
  if( ( t1 == Float ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Float ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Float ) && ( t2 == Float ) ) return true;
  if( ( t1 == Float ) && ( t2 == String ) ) return true;
  
  if( ( t1 == String ) && ( t2 == Empty ) ) return true;
  if( ( t1 == String ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == String ) && ( t2 == Integer ) ) return true;
  if( ( t1 == String ) && ( t2 == Float ) ) return true;
  if( ( t1 == String ) && ( t2 == String ) ) return true;
  
  return false;
}

// compare values. looks strange in order to be compatible with Excel
int KSpreadValue::compare( const KSpreadValue& v ) const 
{
  KSpreadValue::Type t1 = d->type;
  KSpreadValue::Type t2 = v.type();
  
  // empty == empty
  if( ( t1 == Empty ) && ( t2 == Empty ) )
    return 0;
  
  // empty value is always less than string
  // (except when the string is empty)
  if( ( t1 == Empty ) && ( t2 == String ) )
    return( v.asString().isEmpty() ) ? 0 : -1;
    
  // boolean vs boolean
  if( ( t1 == Boolean ) && ( t2 == Boolean ) )
  {
    bool p = asBoolean();
    bool q = v.asBoolean();
    if( p ) return q ? 0 : 1;
    else return q ? -1 : 0;
  }
  
  // boolean is always greater than integer
  if( ( t1 == Boolean ) && ( t2 == Integer ) )
    return 1;
  
  // boolean is always greater than float
  if( ( t1 == Boolean ) && ( t2 == Float ) )
    return 1;
  
  // boolean is always greater than string
  if( ( t1 == Boolean ) && ( t2 == String ) )
    return 1;
  
  // integer is always less than boolean
  if( ( t1 == Integer ) && ( t2 == Boolean ) )
    return -1;
    
  // integer vs integer
  if( ( t1 == Integer ) && ( t2 == Integer ) )
  {
    long p = asInteger();
    long q = v.asInteger();   
    return ( p == q ) ? 0 : ( p < q ) ? -1 : 1;
  }  
  
  // integer vs float
  if( ( t1 == Integer ) && ( t2 == Float ) )
    return compare( asFloat(), v.asFloat() );
  
  // integer is always less than string
  if( ( t1 == Integer ) && ( t2 == String ) )
    return -1;
  
  // float is always less than boolean
  if( ( t1 == Float ) && ( t2 == Boolean ) )
    return -1;
  
  // float vs integer
  if( ( t1 == Float ) && ( t2 == Integer ) )
    return compare( asFloat(), v.asFloat() );
  
  // float vs float
  if( ( t1 == Float ) && ( t2 == Float ) )
    return compare( asFloat(), v.asFloat() );
  
  // float is always less than string
  if( ( t1 == Float ) && ( t2 == String ) )
    return -1;

  // string is always greater than empty value
  // (except when the string is empty)
  if( ( t1 == String ) && ( t2 == Empty ) )
    return( asString().isEmpty() ) ? 0 : 1;
  
  // string is always less than boolean
  if( ( t1 == String ) && ( t2 == Boolean ) )
    return -1;
  
  // string is always greater than integer
  if( ( t1 == String ) && ( t2 == Integer ) )
    return 1;
    
  // string is always greater than float
  if( ( t1 == String ) && ( t2 == Float ) )
    return 1;

  // The-Real-String comparison
  if( ( t1 == String ) && ( t2 == String ) )
    return asString().compare( v.asString() );

  // Undefined, actually allowComparison would return false
  return 0;
}

bool KSpreadValue::equal( const KSpreadValue& v ) const
{
  return compare( v ) == 0;
}

bool KSpreadValue::less( const KSpreadValue& v ) const
{
  return compare( v ) < 0;
}

bool KSpreadValue::greater( const KSpreadValue& v ) const
{
  return compare( v ) > 0;
}

QTextStream& operator<<( QTextStream& ts, KSpreadValue::Type type ) 
{
  switch( type )
  {
    case KSpreadValue::Empty:   ts << "Empty"; break;
    case KSpreadValue::Boolean: ts << "Boolean"; break;
    case KSpreadValue::Integer: ts << "Integer"; break;
    case KSpreadValue::Float:   ts << "Float"; break;
    case KSpreadValue::String:  ts << "String"; break;
    case KSpreadValue::Array:   ts << "Array"; break;
    case KSpreadValue::Error:   ts << "Error"; break;
    default: ts << "Unknown!"; break;
  };
  return ts;
}

QTextStream& operator<<( QTextStream& ts, KSpreadValue value ) 
{
  ts << value.type();
  switch( value.type() )
  {
    case KSpreadValue::Empty:   break;
    
    case KSpreadValue::Boolean: 
      ts << ": "; 
      if (value.asBoolean()) ts << "TRUE"; 
      else ts << "FALSE"; break;
      
    case KSpreadValue::Integer: 
      ts << ": " << value.asInteger(); break;
      
    case KSpreadValue::Float:   
      ts << ": " << value.asFloat(); break;
      
    case KSpreadValue::String:  
      ts << ": " << value.asString(); break;
      
    case KSpreadValue::Error:   break;
    
    default: break;
  }
  return ts;
}
