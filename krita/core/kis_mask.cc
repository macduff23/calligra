/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include "kis_global.h"
#include "kis_types.h"
#include "kis_mask.h"
#include "kis_image.h"

KisMask::KisMask(KisImage *img, Q_INT32 width, Q_INT32 height, const QString& name, const KoColor& color) : super(img, width, height, name, color)
{
}

KisMask::KisMask(const KisMask& rhs) : super(rhs)
{
}

KisMask::~KisMask()
{
}
