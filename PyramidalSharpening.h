/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _PYRAMIDALSHARPENING_H_
#define _PYRAMIDALSHARPENING_H_

// TODO: remove that
#define LCMS_HEADER <lcms.h>
// TODO: remove it !

#include <kparts/plugin.h>
#include <kis_filter.h>

class PyramidalSharpeningFilterConfig;

class KritaPyramidalSharpening : public KParts::Plugin
{
public:
    KritaPyramidalSharpening(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaPyramidalSharpening();
};

class KisPyramidalSharpeningFilter : public KisFilter
{
public:
    KisPyramidalSharpeningFilter();
public:
    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
    static inline KisID id() { return KisID("PyramidalSharpening", i18n("Pyramidal sharpeninig")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return false; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual bool supportsAdjustmentLayers() { return true; }
};

#endif
