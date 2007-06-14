/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_iptc_io.h"
#include "kis_exif_io.h"

#include "kis_exiv2.h"

#include <kis_meta_data_value.h>

#include <kdebug.h>

// ---- Generic convertion functions ---- //

// Convert an exiv value to a KisMetaData value
KisMetaData::Value exivValueToKMDValue( const Exiv2::Value::AutoPtr value )
{
    switch(value->typeId())
    {
        case Exiv2::invalid6:
        case Exiv2::invalidTypeId:
        case Exiv2::lastTypeId:
        case Exiv2::directory:
            kDebug() << "Invalid value : " << value->typeId() << " value = " << value->toString().c_str() << endl;
            return KisMetaData::Value();
        case Exiv2::undefined:
        {
            kDebug() << "Undefined value : " << value->typeId() << " value = " << value->toString().c_str() << endl;
            QByteArray array( value->count() ,0);
            value->copy((Exiv2::byte*)array.data(), Exiv2::invalidByteOrder);
            return KisMetaData::Value(array);
        }
        case Exiv2::unsignedByte:
        case Exiv2::unsignedShort:
        case Exiv2::unsignedLong:
        case Exiv2::signedShort:
        case Exiv2::signedLong:
        {
            if(value->count() ==1)
            {
                return KisMetaData::Value( (int)value->toLong() );
            } else {
                QList<KisMetaData::Value> array;
                for(int i = 0; i < value->count(); i++)
                    array.push_back( KisMetaData::Value((int)value->toLong(i) ) );
                return KisMetaData::Value( array, KisMetaData::Value::UnorderedArray );
            }
        }
        case Exiv2::asciiString:
        case Exiv2::string:
        case Exiv2::comment: // look at kexiv2 for the problem about decoding correctly that tag
            return KisMetaData::Value( value->toString().c_str() );
        case Exiv2::unsignedRational:
            return KisMetaData::Value( KisMetaData::UnsignedRational( value->toRational().first , value->toRational().second ) );
        case Exiv2::signedRational:
            return KisMetaData::Value( KisMetaData::SignedRational( value->toRational().first , value->toRational().second ));
        case Exiv2::date:
        case Exiv2::time:
            return KisMetaData::Value(QDateTime::fromString(value->toString().c_str(), Qt::ISODate));
    }
    kDebug() << "Unknown type id : " << value->typeId() << " value = " << value->toString().c_str() << endl;
    Q_ASSERT(false); // This point must never be reached !
    return KisMetaData::Value();
}

// Convert a QtVariant to an Exiv value
Exiv2::Value* variantToExivValue( const QVariant& variant, Exiv2::TypeId type )
{
    switch(type)
    {
        case Exiv2::undefined:
            return new Exiv2::DataValue( (Exiv2::byte*)variant.toByteArray().data(), variant.toByteArray().size() );
        case Exiv2::unsignedShort:
            return new Exiv2::ValueType<uint16_t>(variant.toInt(0));
        case Exiv2::unsignedLong:
            return new Exiv2::ValueType<uint32_t>(variant.toUInt(0));
        case Exiv2::signedShort:
            return new Exiv2::ValueType<int16_t>(variant.toInt(0));
        case Exiv2::signedLong:
            return new Exiv2::ValueType<int32_t>(variant.toInt(0));
        case Exiv2::asciiString:
            if(variant.type() == QVariant::DateTime)
            {
                return new Exiv2::AsciiValue(qPrintable(variant.toDateTime().toString("yyyy:MM:dd hh:mm:ss")));
            }
            else
                return new Exiv2::AsciiValue(qPrintable(variant.toString()));
        case Exiv2::string:
        {
            if(variant.type() == QVariant::DateTime)
            {
                return new Exiv2::StringValue(qPrintable(variant.toDateTime().toString("yyyy:MM:dd hh:mm:ss")));
            }
            else
                return new Exiv2::StringValue(qPrintable(variant.toString()));
        }
        case Exiv2::comment:
            return new Exiv2::CommentValue(qPrintable(variant.toString()));
        default:
            Q_ASSERT(false);
            return 0;
    }
}

// Convert a KisMetaData to an Exiv value
Exiv2::Value* kmdValueToExivValue( const KisMetaData::Value& value, Exiv2::TypeId type )
{
    switch(value.type())
    {
        case KisMetaData::Value::Invalid:
            return &*Exiv2::Value::create( Exiv2::invalidTypeId);
        case KisMetaData::Value::Variant:
        {
            return variantToExivValue( value.asVariant(), type );
        }
        case KisMetaData::Value::SignedRational:
            Q_ASSERT( type == Exiv2::signedRational );
            return new Exiv2::ValueType<Exiv2::Rational>(Exiv2::Rational( value.asSignedRational().numerator, value.asSignedRational().denominator ) );
        case KisMetaData::Value::UnsignedRational:
        {
            Q_ASSERT( type == Exiv2::unsignedRational );
            return new Exiv2::ValueType<Exiv2::URational>(Exiv2::URational( value.asUnsignedRational().numerator, value.asUnsignedRational().denominator ) );
        }
        default:
            Q_ASSERT(false);
    }
    return 0;
}


// TODO: There must be a better way to do that !
struct KisExiv2
{
    KisExiv2()
    {
        KisMetaData::IOBackendRegistry::instance()->add(new KisIptcIO);
        KisMetaData::IOBackendRegistry::instance()->add(new KisExifIO);
    }
};

static KisExiv2* exiv2 = new KisExiv2();

// void initKisExiv2()
// {
//     if(not isInit)
//     {
//         KisMetaData::IOBackendRegistry::instance()->add(new KisIptcIO);
//         KisMetaData::IOBackendRegistry::instance()->add(new KisExifIO);
//         isInit = true;
//     }
// }
