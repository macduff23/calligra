/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "wavefilter.h"

#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_layer.h>
#include <kis_random_sub_accessor.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include "kis_wdg_wave.h"
#include "ui_wdgwaveoptions.h"

typedef KGenericFactory<KritaWaveFilter> KritaWaveFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritawavefilter, KritaWaveFilterFactory( "krita" ) )

class KisWaveCurve {
    public:
        virtual ~KisWaveCurve() {}
        virtual double valueAt(int x, int y) =0;
};

class KisSinusoidalWaveCurve : public KisWaveCurve {
    public:

        KisSinusoidalWaveCurve(int amplitude, int wavelenght, int shift) : m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift)
        {
        }

        virtual ~KisSinusoidalWaveCurve() {}

        virtual double valueAt(int x, int y)
        {
            return y + m_amplitude * cos( (double) ( m_shift + x) / m_wavelength );
        }
    private:
        int m_amplitude, m_wavelength, m_shift;
};

class KisTriangleWaveCurve : public KisWaveCurve {
    public:

        KisTriangleWaveCurve(int amplitude, int wavelenght, int shift) :  m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift)
        {
        }

        virtual ~KisTriangleWaveCurve() {}

        virtual double valueAt(int x, int y)
        {
            return y +  m_amplitude * pow( -1, (m_shift + x) / m_wavelength )  * (0.5 - (double)( (m_shift + x) % m_wavelength ) / m_wavelength );
        }
    private:
        int m_amplitude, m_wavelength, m_shift;
};



KritaWaveFilter::KritaWaveFilter(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData(KritaWaveFilterFactory::componentData());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterWave()));
    }
}

KritaWaveFilter::~KritaWaveFilter()
{
}

KisFilterWave::KisFilterWave() : KisFilter(id(), CategoryOther, i18n("&Wave..."))
{
}

KisFilterConfiguration* KisFilterWave::factoryConfiguration(const KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration("wave", 1);
    config->setProperty("horizontalwavelength", 50 );
    config->setProperty("horizontalshift", 50 );
    config->setProperty("horizontalamplitude", 4 );
    config->setProperty("horizontalshape", 0 );
    config->setProperty("verticalwavelength", 50 );
    config->setProperty("verticalshift", 50 );
    config->setProperty("verticalamplitude", 4 );
    config->setProperty("verticalshape", 0 );
    return config;
}

KisFilterConfigWidget * KisFilterWave::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    return new KisWdgWave((KisFilter*)this, (QWidget*)parent);
}

void KisFilterWave::process(KisFilterConstantProcessingInformation src,
                 KisFilterProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoProgressUpdater* progressUpdater
        ) const
{
#if 0
    Q_ASSERT(src.data() != 0);
    Q_ASSERT(dst.data() != 0);

    setProgressTotalSteps(size.width() * size.height());

    QVariant value;
    int horizontalwavelength = (config && config->getProperty("horizontalwavelength", value)) ? value.toInt() : 50;
    int horizontalshift = (config && config->getProperty("horizontalshift", value)) ? value.toInt() : 50;
    int horizontalamplitude = (config && config->getProperty("horizontalamplitude", value)) ? value.toInt() : 4;
    int horizontalshape = (config && config->getProperty("horizontalshape", value)) ? value.toInt() : 0;
    int verticalwavelength = (config && config->getProperty("verticalwavelength", value)) ? value.toInt() : 50;
    int verticalshift = (config && config->getProperty("verticalshift", value)) ? value.toInt() : 50;
    int verticalamplitude = (config && config->getProperty("verticalamplitude", value)) ? value.toInt() : 4;
    int verticalshape = (config && config->getProperty("verticalshape", value)) ? value.toInt() : 0;
    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
    KisWaveCurve* verticalcurve;
    if(verticalshape == 1)
        verticalcurve = new KisTriangleWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    else
        verticalcurve = new KisSinusoidalWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    KisWaveCurve* horizontalcurve;
    if(horizontalshape == 1)
        horizontalcurve = new KisTriangleWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    else
        horizontalcurve = new KisSinusoidalWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    KisRandomSubAccessorPixel srcRSA = src->createRandomSubAccessor();
    int tx = srcTopLeft.x() - dstTopLeft.x();
    int ty = srcTopLeft.y() - dstTopLeft.y();
    while(!dstIt.isDone())
    {
        double xv = horizontalcurve->valueAt( dstIt.y(), dstIt.x() );
        double yv = verticalcurve->valueAt( dstIt.x(), dstIt.y() );
        srcRSA.moveTo( QPointF( xv - tx, yv - ty ) );
        srcRSA.sampledOldRawData(dstIt.rawData());
        ++dstIt;
        incProgress();
    }
    delete horizontalcurve;
    delete verticalcurve;
    setProgressDone(); // Must be called even if you don't really support progression
#endif
}
