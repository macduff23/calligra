/*
 *  copyright (c) 2002 patrick julien <freak@codepimps.org>
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
#ifndef KIS_PAINT_DEVICE_H_
#define KIS_PAINT_DEVICE_H_

#include <qcolor.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qvaluelist.h>
#include <qstring.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_render.h"
#include "kis_image.h"
#include "kistilemgr.h"
#include "kis_strategy_colorspace.h"
#include "kispixeldata.h"
#include "kis_scale_visitor.h"

class QImage;
class QSize;
class QPoint;
class KoStore;
class KisImage;
class QWMatrix;
class KisTileCommand;
class KisIteratorLineQuantum;
class KisIteratorLinePixel;
class KisRotateVisitor;
class KisOilPaintFilterVisitor;

/**
 * Class modelled on QPaintDevice.
 */
class KisPaintDevice : public QObject, public KisRenderInterface {
        Q_OBJECT
        typedef KisRenderInterface super;

public:
	KisPaintDevice(Q_INT32 width, Q_INT32 height,
			KisStrategyColorSpaceSP colorStrategy,
			const QString& name);
	KisPaintDevice(KisImage *img,
			Q_INT32 width, Q_INT32 height,
			KisStrategyColorSpaceSP colorStrategy,
			const QString& name);
	KisPaintDevice(KisTileMgrSP tm,
			KisImage *img,
			const QString& name);
	KisPaintDevice(const KisPaintDevice& rhs);
	virtual ~KisPaintDevice();

public:
        // Implement KisRenderInterface
        virtual Q_INT32 tileNum(Q_INT32 xpix, Q_INT32 ypix) const;
        virtual KisTileMgrSP tiles() const;
        virtual bool write(KoStore *store);
        virtual bool read(KoStore *store);

public:
	virtual void configure(KisImage *image, 
			Q_INT32 width, Q_INT32 height, 
			KisStrategyColorSpaceSP colorStrategy, 
			const QString& name,
			CompositeOp compositeOp);
	/**
	   The data() methods return a shared pointer to the tile manager.
	*/
private:
        virtual KisTileMgrSP data();
        virtual const KisTileMgrSP data() const;
public:
        virtual bool shouldDrawBorder() const;

        virtual void move(Q_INT32 x, Q_INT32 y);
        virtual void move(const QPoint& pt);

	virtual const bool visible() const;
        virtual void setVisible(bool v);

        bool contains(Q_INT32 x, Q_INT32 y) const;
        bool contains(const QPoint& pt) const;

        void setTiles(KisTileMgrSP mgr);

	/** 
	 *   Converts the paint device to a different colorspace
	 */
	virtual void convertTo(KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile = 0, Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


	/**
	 * Fill this paint device with the data from img;
	 */
	virtual void convertFromImage(const QImage& img);

	/**
	 * Create an RGBA QImage from a rectangle in the paint device.
	 *
	 * x, y left-top point of the rect of pixels
	 * w, h width and height in pixels
	 * profile RGB profile to use in conversion. May be 0, in which
	 * case it's up to the colour strategy to choose a profile (most 
	 * like sRGB).
	 */
	virtual QImage convertToQImage(KisProfileSP dstProfile, Q_INT32 x = 0, Q_INT32 y = 0, Q_INT32 w = -1, Q_INT32 h = -1);

        virtual QString name() const;
        virtual void setName(const QString& name);


        /** 
	 * fill c and opacity with the values found at x and y
	 * @return true if the operation was succesful
	 */
        bool pixel(Q_INT32 x, Q_INT32 y, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

        /**
	 * Set the specified pixel to the specified color. Note that this
         * bypasses KisPainter. the PaintDevice is here used as an equivalent
         * to QImage, not QPixmap. This means that this is undoable; also,
         * there is no compositing with an existing value at this location.
	 * @return true if the operation was succesful
	 */
        bool setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, QUANTUM opacity, KisProfileSP profile = 0);

        void maskBounds(Q_INT32 *x1, Q_INT32 *y1, Q_INT32 *x2, Q_INT32 *y2);
        void maskBounds(QRect *rc);

        bool alpha() const;

	KisStrategyColorSpaceSP colorStrategy() const;

	/**
	 * Return the icm profile associated with this layer, or 
	 * the profile associated with the image if the color space of
	 * this layer is the same as the color space of the image,
	 * or 0.
	 */
	KisProfileSP profile() const;

	/**
	 * Set the profile associated with this layer to the specified profile
	 * or reset to 0 if the profile does not have the same colorspace signature
	 * as the color model associated with this paint device.
	 */
	void setProfile(KisProfileSP profile);

	CompositeOp compositeOp() { return m_compositeOp; }
	void setCompositeOp(CompositeOp compositeOp) { m_compositeOp = compositeOp; }

        KisTileMgrSP shadow();
        const KisTileMgrSP shadow() const;

	// XXX?
        QRect bounds() const;

        Q_INT32 x() const;
        void setX(Q_INT32 x);

        Q_INT32 y() const;
        void setY(Q_INT32 y);

	Q_INT32 depth() const;
        Q_INT32 width() const;
        Q_INT32 height() const;

	QRect clip() const;
        void clip(Q_INT32 *offx, Q_INT32 *offy, Q_INT32 *offw, Q_INT32 *offh) const;
        void setClip(Q_INT32 offx, Q_INT32 offy, Q_INT32 offw, Q_INT32 offh);

        KisImage *image();
        const KisImage *image() const;
        void setImage(KisImage *image);

        void resize(Q_INT32 w, Q_INT32 h);
        void resize(const QSize& size);
        void resize();

	// XXX: Do all rotations etc. use the visitor instead of the QMatrix-based code by now?
	void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, enumFilterType ftype=MITCHELL_FILTER);
        void rotate(double angle, KisProgressDisplayInterface *m_progress);
        void shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress);
        void oilPaintFilter(Q_UINT32 brushSize, Q_UINT32 smooth, KisProgressDisplayInterface *m_progress);
        
	/**
	   Apply the transformation matrix _in place_.
	*/
	void transform(const QWMatrix & matrix);
	
	/**
	 * Mirror the device along the X axis
	 */
	void mirrorX();
	/**
	 * Mirror the device along the Y axis
	 */
	void mirrorY();

        void expand(Q_INT32 w, Q_INT32 h);
        void expand(const QSize& size);

        void offsetBy(Q_INT32 x, Q_INT32 y);

	/** 
	 * This function return an iterator which point on the first line of the
	 * whole PaintDevice
	 */
	KisIteratorLineQuantum iteratorQuantumBegin(KisTileCommand* command) KDE_DEPRECATED;
	KisIteratorLineQuantum iteratorQuantumBegin(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 ystart) KDE_DEPRECATED;

	/**
	 * This function return an iterator which point on the last line of the
	 * whole PaintDevice
	 */
	KisIteratorLineQuantum iteratorQuantumEnd(KisTileCommand* command) KDE_DEPRECATED;
	KisIteratorLineQuantum iteratorQuantumEnd(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 yend) KDE_DEPRECATED;

	/**
	 * This function return an iterator which point on the first line of the
	 * part of PaintDevice which is selected
	 */
	KisIteratorLineQuantum iteratorQuantumSelectionBegin(KisTileCommand* command) KDE_DEPRECATED;
	KisIteratorLineQuantum iteratorQuantumSelectionBegin(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 ystart) KDE_DEPRECATED;

	/**
	 * This function return an iterator which point on the last line of the
	 * part of PaintDevice which is selected
	 */
	KisIteratorLineQuantum iteratorQuantumSelectionEnd(KisTileCommand* command) KDE_DEPRECATED;
	KisIteratorLineQuantum iteratorQuantumSelectionEnd(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 yend) KDE_DEPRECATED;
	
	/** 
	 * This function return an iterator which points to the first pixel of the
	 * whole PaintDevice
	 */
	KisIteratorLinePixel iteratorPixelBegin(KisTileCommand* command);
	KisIteratorLinePixel iteratorPixelBegin(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 ystart);

	/**
	 * This function return an iterator which points to the last pixel of the
	 * whole PaintDevice
	 */
	KisIteratorLinePixel iteratorPixelEnd(KisTileCommand* command);
	KisIteratorLinePixel iteratorPixelEnd(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 yend);

	/**
	 * This function return an iterator which points to the first pixel of the
	 * part of PaintDevice which is selected
	 */
	KisIteratorLinePixel iteratorPixelSelectionBegin(KisTileCommand* command);
	KisIteratorLinePixel iteratorPixelSelectionBegin(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 ystart);

	/**
	 * This function return an iterator which points to the last pixel of the
	 * part of PaintDevice which is selected
	 */
	KisIteratorLinePixel iteratorPixelSelectionEnd(KisTileCommand* command);
	KisIteratorLinePixel iteratorPixelSelectionEnd(KisTileCommand* command, Q_INT32 xpos, Q_INT32 xend, Q_INT32 yend);

	// Selection stuff. XXX: is it necessary to make the actual
	// selection object available outside the layer? YYY: yes, so
	// selection tools can act on it.

	/** Get the current selection or create one if this layers hasn't got a selection yet. */
	KisSelectionSP selection();

	/** Set the specified selection object as the active selection for this layer */
	void setSelection(KisSelectionSP selection);

	/** Adds the specified selection to the currently active selection for this layer */
	void addSelection(KisSelectionSP selection);

	/** Subtracts the specified selection from the currently active selection for this layer */
	void subtractSelection(KisSelectionSP selection);

	/** Whether there is a valid selection for this layer. */
	bool hasSelection();

	/** Removes the current selection for this layer. */
	void removeSelection();


signals:

        void visibilityChanged(KisPaintDeviceSP device);
        void positionChanged(KisPaintDeviceSP device);
        void ioProgress(Q_INT8 percentage);
	void selectionChanged();
	void selectionCreated();
	void profileChanged(KisProfileSP profile);

protected:
        void setWidth(Q_INT32 w);
        void setHeight(Q_INT32 h);

private:
        KisPaintDevice& operator=(const KisPaintDevice&);
        void init();

private:
        KisImage *m_owner;
        KisTileMgrSP m_tiles;
        KisTileMgrSP m_shadow;
        bool m_visible;
        Q_INT32 m_x;
        Q_INT32 m_y;
        Q_INT32 m_offX;
        Q_INT32 m_offY;
        Q_INT32 m_offW;
        Q_INT32 m_offH;
        QString m_name;
	// Operation used to composite this layer with the layers _under_ this layer
	CompositeOp m_compositeOp;
	KisStrategyColorSpaceSP m_colorStrategy; 
	KisProfileSP m_profile;

	void accept(KisScaleVisitor &);
	void accept(KisRotateVisitor &);
        void accept(KisOilPaintFilterVisitor &);
        
	// Whether there is a selection valid for this layer
	bool m_hasSelection;
	// Contains the actual selection. For now, there can be only
	// one selection per layer. XXX: is this a limitation? 
	KisSelectionSP m_selection;
        
};

inline KisTileMgrSP KisPaintDevice::tiles() const
{
        return m_tiles;
}

inline Q_INT32 KisPaintDevice::depth() const
{
        return tiles()->depth();
;
}

inline KisStrategyColorSpaceSP KisPaintDevice::colorStrategy() const
{
	if (m_colorStrategy == 0 && m_owner != 0) {
		return m_owner -> colorStrategy();
	}
        return m_colorStrategy;
}

inline KisTileMgrSP KisPaintDevice::data()
{
	return m_tiles;
}

inline const KisTileMgrSP KisPaintDevice::data() const
{
        return m_tiles;
}

inline KisTileMgrSP KisPaintDevice::shadow()
{
        return m_shadow;
}

inline const KisTileMgrSP KisPaintDevice::shadow() const
{
        return m_shadow;
}

inline QRect KisPaintDevice::bounds() const
{
        return QRect(m_x, m_y, width(), height());
}

inline Q_INT32 KisPaintDevice::x() const
{
        return m_x;
}

inline void KisPaintDevice::setX(Q_INT32 x)
{
        m_x = x;
}

inline Q_INT32 KisPaintDevice::y() const
{
        return m_y;
}

inline void KisPaintDevice::setY(Q_INT32 y)
{
        m_y = y;
}

inline Q_INT32 KisPaintDevice::width() const
{
        return tiles()->width();
}

inline Q_INT32 KisPaintDevice::height() const
{
        return tiles()->height();
}

inline const bool KisPaintDevice::visible() const
{
        return m_visible;
}

inline void KisPaintDevice::setVisible(bool v)
{
        if (m_visible != v) {
                m_visible = v;
                emit visibilityChanged(this);
        }
}

inline QRect KisPaintDevice::clip() const
{
        return QRect(m_offX, m_offY, m_offW, m_offH);
}

inline void KisPaintDevice::clip(Q_INT32 *offx, Q_INT32 *offy, Q_INT32 *offw, Q_INT32 *offh) const
{
        if (offx && offy && offw && offh) {
                *offx = m_offX;
                *offy = m_offY;
                *offw = m_offW;
                *offh = m_offH;
        }
}

inline void KisPaintDevice::setClip(Q_INT32 offx, Q_INT32 offy, Q_INT32 offw, Q_INT32 offh)
{
        m_offX = offx;
        m_offY = offy;
        m_offW = offw;
        m_offH = offh;
}


inline KisImage *KisPaintDevice::image()
{
        return m_owner;
}

inline const KisImage *KisPaintDevice::image() const
{
        return m_owner;
}

inline void KisPaintDevice::setImage(KisImage *image)
{
        m_owner = image;
}

inline void KisPaintDevice::resize(const QSize& size)
{
        resize(size.width(), size.height());
}

inline void KisPaintDevice::resize()
{
        KisImageSP img = image();

        if (img)
                resize(img -> bounds().size());
}

inline bool KisPaintDevice::alpha() const
{
        return colorStrategy()->alpha();
}

inline bool KisPaintDevice::pixel(Q_INT32 x, Q_INT32 y, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{
	KisTileMgrSP tm = tiles();
	KisPixelDataSP pd = tm -> pixelData(x - m_x, y - m_y, x - m_x, y - m_y, TILEMODE_READ);
	QUANTUM *pix;

	if (!pd)
		return false;

	pix = pd -> data;
	
 	if (!pix) return false;

 	colorStrategy() -> toQColor(pix, c, opacity); //profile

	return true;
}

inline bool KisPaintDevice::setPixel(Q_INT32 x, Q_INT32 y, const QColor& c, QUANTUM opacity, KisProfileSP profile)
{
	KisTileMgrSP tm = tiles();
	KisPixelDataSP pd = tm -> pixelData(x - m_x, y - m_y, x - m_x, y - m_y, TILEMODE_WRITE);
	QUANTUM * pix;

	if (!pd)
		return false;

	pix = pd -> data;
	Q_ASSERT(pix);

	colorStrategy() -> nativeColor(c, opacity, pix); //profile

	tm -> releasePixelData(pd);

	return true;


}


#endif // KIS_PAINT_DEVICE_H_

