/*
 *  kis_tile.cc - part of KImageShop aka Krayon aka Krita
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <stdlib.h>

#include <qtl.h>

#include "kis_global.h"
#include "kis_tile.h"

KisTile::KisTile(int x, int y, uint width, uint height, uchar bpp, const QRgb& defaultColor, bool dirty)
{
	Q_ASSERT(bpp <= 4);
	m_cow = false;
	m_dirty = dirty;
	m_width = width;
	m_height = height;
	m_bpp = bpp;
	m_data = 0;
	m_defaultColor = defaultColor;
	move(x, y);

	if (qAlpha(defaultColor))
		initTile();
}

KisTile::KisTile(const QPoint& parentPos, uint width, uint height, uchar bpp, const QRgb& defaultColor, bool dirty)
{
	Q_ASSERT(bpp <= 4);
	m_cow = false;
	m_dirty = dirty;
	m_width = width;
	m_height = height;
	m_bpp = bpp;
	m_data = 0;
	m_defaultColor = defaultColor;
	m_parentPos = parentPos;

	if (qAlpha(defaultColor))
		initTile();
}

KisTile::KisTile(const KisTile& tile) : KShared(tile)
{
	if (this != &tile)
		copyTile(tile);
}

KisTile& KisTile::operator=(const KisTile& tile)
{
	if (this != &tile)
		copyTile(tile);

	return *this;
}

KisTile::~KisTile()
{
	delete[] m_data;
}
	
void KisTile::copyTile(const KisTile& tile)
{
	m_dirty = tile.m_dirty;
	m_width = tile.m_width;
	m_height = tile.m_height;
	m_bpp = tile.m_bpp;
	m_data = 0;
	m_defaultColor = tile.m_defaultColor;
	m_parentPos = tile.m_parentPos;
	m_cow = false;

	if (tile.m_data) {
		m_data = new uchar[size()];
		memcpy(m_data, tile.m_data, size());
	}
}

void KisTile::setDirty(bool dirty)
{
	m_dirty = dirty;
}

uchar* KisTile::data()
{
	if (!m_data)
		m_data = new uchar[size()];

	return m_data;
}

void KisTile::initTile()
{
	m_data = new uchar[size()];
	qFill(m_data, m_data + size(), m_defaultColor);
//	memset(m_data, rand() % 255, size());
}

void KisTile::move(int x, int y)
{
	m_parentPos.setX(x);
	m_parentPos.setY(y);
}

void KisTile::move(const QPoint& parentPos)
{
	m_parentPos = parentPos;
}

uchar *KisTile::data(int x, int y)
{
	int offset = (y * width() + x) * bpp();

	Q_ASSERT(data() <= data() + offset);
	Q_ASSERT(data() + size() >= data() + offset);
	return data() + offset;
}

const uchar *KisTile::data(int x, int y) const
{
	int offset = (y * width() + x) * bpp();

	Q_ASSERT(data() <= data() + offset);
	Q_ASSERT(data() + size() >= data() + offset);
	return data() + offset;
}

