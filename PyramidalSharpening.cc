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

#include "PyramidalSharpening.h"

#include <stdlib.h>
#include <vector>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_selection.h>

#include <kis_basic_math_toolbox2.h>

#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

typedef KGenericFactory<KritaPyramidalSharpening> KritaPyramidalSharpeningFactory;
K_EXPORT_COMPONENT_FACTORY( kritaPyramidalSharpening, KritaPyramidalSharpeningFactory( "krita" ) )

KritaPyramidalSharpening::KritaPyramidalSharpening(QObject *parent, const char *name, const QStringList &)
: KParts::Plugin(parent, name)
{
    setInstance(KritaPyramidalSharpeningFactory::instance());

    kdDebug(41006) << "Pyramidal Sharpening filter plugin. Class: "
    << className()
    << ", Parent: "
    << parent -> className()
    << "\n";

    if(parent->inherits("KisFilterRegistry"))
    {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisPyramidalSharpeningFilter());
    }
}

KritaPyramidalSharpening::~KritaPyramidalSharpening()
{
}

KisPyramidalSharpeningFilter::KisPyramidalSharpeningFilter() 
    : KisFilter(id(), "PyramidalSharpening", i18n("&Pyramidal sharpening..."))
{
}

void KisPyramidalSharpeningFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, 
                                   KisFilterConfiguration* config, const QRect& rect ) 
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    KisBasicMathToolbox2 tlb2;
    KisBasicMathToolbox2::Pyramid* gaussianPyramid = tlb2.toGaussianPyramid(src, 5, rect);
    delete gaussianPyramid;
    
    setProgressDone(); // Must be called even if you don't really support progression
}
