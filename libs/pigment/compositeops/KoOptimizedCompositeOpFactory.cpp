/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoOptimizedCompositeOpFactory.h"

#include "config-vc.h"


#ifdef HAVE_VC

#include <Vc/global.h>
#include <Vc/common/support.h>
#endif

#include "KoOptimizedCompositeOpFactoryPerArch.h"

static struct ArchReporter {
    ArchReporter() {
        createOptimizedClass<KoReportCurrentArch>(0);
    }
} StaticReporter;


KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(const KoColorSpace *cs)
{
    return createOptimizedClass<KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpAlphaDarken32> >(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createOverOp32(const KoColorSpace *cs)
{
    return createOptimizedClass<KoOptimizedCompositeOpFactoryPerArch<KoOptimizedCompositeOpOver32> >(cs);
}
