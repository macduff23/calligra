/*
 *  channel.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
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

#include "channel.h"
#include <qstring.h>
#include <qimage.h>
#include <stdio.h>
#include "misc.h"

Channel::Channel()
{
  // puts("new channel");
  xTilesNo=0;
  yTilesNo=0;
  tileInfo=0;
  imageRect=QRect();
  tilesRect=QRect();
	lastTileXOffset=0;
	lastTileYOffset=0;
}

void Channel::loadViaQImage(QImage img, int channel)
{
  // printf("loadViaQImage %dx%d\n",img.width(),img.height());
  // slow but simple - speed it up.
  int w=img.width();
  int h=img.height();
  
  allocateRect(QRect(0,0, w,h));
  
  uchar *imgPtr;
  for(int y=0;y<h;y++) {
	  imgPtr=img.scanLine(y)+channel;
	  for(int x=0;x<w;x++) {
		  setPixel(x,y, *imgPtr);
		  imgPtr+=4;
		}
	}
}


void Channel::setPixel(int x, int y, uchar val)
{
	//resizeChannel(QRect(x,y,1,1));
 	x=x-tilesRect.x(); // Find the point in tile coordinates
	y=y-tilesRect.y();

	// Find the tile
  int tileNo=(y/TILE_SIZE)*xTilesNo + x/TILE_SIZE;
  *(tileInfo[tileNo]+(y%TILE_SIZE)*TILE_SIZE + x%TILE_SIZE)=val;
}

uchar Channel::getPixel(int x, int y)
{
	if (imageRect.contains(QPoint(x,y))) {
		x=x-tilesRect.x(); // Find the point in tile coordinates
		y=y-tilesRect.y();

		// Find the tile
		int tileNo=(y/TILE_SIZE)*xTilesNo + x/TILE_SIZE;
		if (tileInfo[tileNo]==0)
			return(0); // XXX fix this see below
		return(*(tileInfo[tileNo]+(y%TILE_SIZE)*TILE_SIZE + x%TILE_SIZE));
	}
	return(0); // XXX fix this return some sort of undef vial KColor
}


// Resize the channel so that it includes the rectangle newRect (canvasCoords)
// and allocates space for all the pixels in newRect

void Channel::allocateRect(QRect newRect)
{
  if (newRect.isNull())
		return;
  
	if (!imageRect.contains(newRect)) {
		if (!tilesRect.contains(newRect)) {
			puts("\nnew resizeChannel");
			//SHOW_RECT_COORDS(newRect);
			//SHOW_RECT(tilesRect);
			//dumpTileBlock();

			// make a newImageExtents rect which contains imageRect and newRect
			// now make it fall on the closest multiple of TILE_SIZE which contains it
			if (tilesRect.isNull())
				tilesRect=QRect(newRect.topLeft(),QSize(1,1));
			QRect newTileExtents=tilesRect;
			newTileExtents=newTileExtents.unite(newRect);
			//SHOW_RECT(newTileExtents);
			//SHOW_RECT_COORDS(newTileExtents);
			
			if (newTileExtents.left()<tilesRect.left())
				newTileExtents.setLeft(tilesRect.left()-((tilesRect.left()-
											newTileExtents.left()+TILE_SIZE-1)/TILE_SIZE)*TILE_SIZE);
			if (newTileExtents.top()<tilesRect.top())
				newTileExtents.setTop(tilesRect.top()-((tilesRect.top()-
											newTileExtents.top()+TILE_SIZE-1)/TILE_SIZE)*TILE_SIZE);

			newTileExtents.setWidth(((newTileExtents.width()+TILE_SIZE-1)/
															 TILE_SIZE)*TILE_SIZE);
			newTileExtents.setHeight(((newTileExtents.height()+TILE_SIZE-1)/
																TILE_SIZE)*TILE_SIZE);
			
			//SHOW_RECT(newTileExtents);
			//SHOW_RECT_COORDS(newTileExtents);
			
			int newXTiles=newTileExtents.width()/TILE_SIZE;
			int newYTiles=newTileExtents.height()/TILE_SIZE;
			
			//showi(newXTiles);
			//showi(newYTiles);
			
			// Allocate new tileInfo block an zero it
			uchar **newTileInfo=new uchar* [newXTiles*newYTiles];
			for(int yTile=0;yTile<newYTiles;yTile++) {
				for(int xTile=0;xTile<newXTiles;xTile++) {
					newTileInfo[yTile*newXTiles+xTile]=0;
				}
			}
			
			// copy the old descriptors to the new block and resize the image
			// these are where the old tiles start in the new tile block
			int oldXTilePos=(tilesRect.left()-newTileExtents.left())/TILE_SIZE;
			int oldYTilePos=(tilesRect.top()-newTileExtents.top())/TILE_SIZE;
			
			// 	  int oldXTilePos=0, oldYTilePos=0;
			// 	  if (newRect.x() < imageRect.x())
			// 			oldXTilePos=newXTiles-xTilesNo;
			// 	  if (newRect.y() < imageRect.y())
			// 			oldYTilePos=newYTiles-yTilesNo;
			
			//puts("copying old descriptors");
			//showi(oldXTilePos);
			//showi(oldYTilePos);
			
			// 			showi(xTilesNo);
			// 			showi(yTilesNo);
			
			// Copy the old tile descriptors into the new array
			for(int y=0;y<yTilesNo;y++)
				for(int x=0;x<xTilesNo;x++)
					newTileInfo[(y+oldYTilePos)*newXTiles+(x+oldXTilePos)]=
						tileInfo[y*xTilesNo+x];
			
			// 			SHOW_RECT_COORDS(imageRect);
			
			delete tileInfo;
			
			imageRect=imageRect.unite(newRect);
			tileInfo=newTileInfo;
			xTilesNo=newXTiles;
			yTilesNo=newYTiles;
			tilesRect=newTileExtents;
			
			dumpTileBlock();
			SHOW_RECT(imageRect);
			SHOW_RECT(tilesRect);
			puts("resizeChannel2: fin");
		}
		else {
			imageRect=imageRect.unite(newRect);
		}
	}

	// Allocate any unallocated tiles in the newRect
	QRect allocRect=newRect;
	allocRect.moveBy(-tilesRect.x(), -tilesRect.y());
	int minYTile=allocRect.top()/TILE_SIZE;
	int maxYTile=allocRect.bottom()/TILE_SIZE;
	int minXTile=allocRect.left()/TILE_SIZE;
	int maxXTile=allocRect.right()/TILE_SIZE;
	
	for(int y=minYTile; y<=maxYTile; y++)
		for(int x=minXTile; x<=maxXTile; x++)
			if (tileInfo[(y*xTilesNo)+x]==0) {
				tileInfo[(y*xTilesNo)+x]=new uchar [TILE_SIZE*TILE_SIZE];
				memset(tileInfo[(y*xTilesNo)+x], 255, TILE_SIZE*TILE_SIZE);
			}

	lastTileXOffset=(imageRect.width()+offset().x())%TILE_SIZE;
	if (lastTileXOffset==0) lastTileXOffset=TILE_SIZE;
	lastTileYOffset=(imageRect.height()+offset().y())%TILE_SIZE;
	if (lastTileYOffset==0) lastTileYOffset=TILE_SIZE;
}


void Channel::dumpTileBlock()
{
  puts("====dumpTileBlock====");
  SHOW_RECT(imageRect);
  SHOW_RECT(tilesRect);
  showi(xTilesNo);
  showi(yTilesNo);
  for(int y=0;y<yTilesNo;y++)
	{
	  for(int x=0;x<xTilesNo;x++)
		printf("%08x ",(unsigned int)tileInfo[(y*xTilesNo)+x]);
	  puts("");
	}
  puts("");
	printf("mx=%d my=%d\n", lastTileXOffset, lastTileYOffset);
}

void Channel::moveBy(int dx, int dy)
{
  imageRect.moveBy(dx, dy);
  tilesRect.moveBy(dx, dy);
}

void Channel::moveTo(int x, int y)
{
	int dx=x-imageRect.x();
	int dy=y-imageRect.y();
  imageRect.moveTopLeft(QPoint(x, y));
	tilesRect.moveBy(dx,dy);
}

QRect Channel::tileRect(int tileNo)
{
	int xTile=tileNo%xTilesNo;
	int yTile=tileNo/xTilesNo;
	QRect tr(xTile*TILE_SIZE,yTile*TILE_SIZE, TILE_SIZE, TILE_SIZE);
	tr.moveBy(tilesRect.x(), tilesRect.y());
	return(tr);
}
