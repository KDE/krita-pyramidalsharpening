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
#include <kis_convolution_painter.h>
#include <kis_transaction.h>

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
    : KisFilter(id(), "PyramidalSharpening", i18n("&Pyramidal sharpening"))
{
}


#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"

void KisPyramidalSharpeningFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, 
                                   KisFilterConfiguration* config, const QRect& rect ) 
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    // See the paper for an explanation on the choice of the parameters
    double c = 0.45;
    double scale = 1.1;
//     double T = 200.0;
    
    KisBasicMathToolbox2 tlb2;
    kdDebug() << "Compute gaussian pyramid" << endl;
//     KisBasicMathToolbox2::Pyramid* gaussianPyramid = tlb2.toSimplePyramid(src, 1, rect);
    KisBasicMathToolbox2::Pyramid* gaussianPyramid = tlb2.toGaussianPyramid(src, 1, rect);
    kdDebug() << "Compute laplacian pyramid" << endl;
    KisBasicMathToolbox2::Pyramid* laplacianPyramid = tlb2.toLaplacianPyramid(gaussianPyramid);
    kdDebug() << " gaussianPyramid.nbLevels == " << gaussianPyramid->levels.size() << " laplacianPyramid.nbLevels == " << laplacianPyramid->levels.size() << endl;
    
    // Compute the bound
    // First threshold the first level of the laplacian pyramid
//     KisPaintDeviceSP lm1 = gaussianPyramid->levels[1].device;
    KisPaintDeviceSP lm1 = laplacianPyramid->levels[0].device;
    QSize s = laplacianPyramid->levels[0].size;
    int depth = lm1->colorSpace()->nColorChannels();
#if 1
    kdDebug() << " Find L0Max" << endl;
    double L0Max;
    {
        KisRectIterator it = lm1->createRectIterator(0,0, s.width(), s.height(), true);
        while(not it.isDone())
        {
            float* arr = reinterpret_cast<float*>(it.rawData());
            for(int i = 0; i < depth; i++)
            {
                if(arr[i] > L0Max)
                {
                    L0Max = arr[i];
                }
            }
            ++it;
        }
    }
    double T = (1.0 - c) * L0Max;
    kdDebug() << " T = " << T << " L0Max = " << L0Max << " c = " << c << " scale = " << scale << endl;
#endif
#if 1
    kdDebug() << "Bound L(0) to give an estimation of L(-1)" << endl;
    {
        KisRectIterator it = lm1->createRectIterator(0,0, s.width(), s.height(), true);
        while(not it.isDone())
        {
            float* arr = reinterpret_cast<float*>(it.rawData());
            for(int i = 0; i < depth; i++)
            {
                if(arr[i] > T)
                {
                    arr[i] = T;
                } else if(arr[i] < -T)
                {
                    arr[i] = -T;
                }
                arr[i] *= scale;
            }
            ++it;
        }
    }
#endif
    // Blur the paint device
    KisPaintDeviceSP lm1Blur = new KisPaintDevice(*lm1);
#if 1
    {
        kdDebug() << "Compute a blured version of L(-1)" << endl;
        double a = 0.5;
        // Compute the kernel
        KisKernelSP kernel = new KisKernel;
        kernel->width = 5;
        kernel->height = 1;
        kernel->offset = 0;
        kernel->factor = 0;
        kernel->data = new Q_INT32[kernel->width];
        // Kernel == [ 1/4-a/2; 1/4; a; 1/4; 1/4-a/2 ] a \in [0.3; 0.6]
        kernel->data[0] = (int)(100 * ( 0.25 - 0.5 * a));
        kernel->data[1] = 25;
        kernel->data[2] = (int)(100 * a);
        kernel->data[3] = 25;
        kernel->data[4] = kernel->data[0];
        kernel->factor = 2*(kernel->data[0] + kernel->data[1]) + kernel->data[2]; // due to rounding it might not be equal to 100
        // Do the bluring
        KisConvolutionPainter painter( lm1Blur );
        painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT, KisChannelInfo::FLAG_COLOR_AND_ALPHA);
        KisTransaction("", lm1Blur);
        kernel->width = 1;
        kernel->height = 5;
        painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT, KisChannelInfo::FLAG_COLOR_AND_ALPHA);
    }
#endif
#if 1
    kdDebug() << rect << s << endl;
    // Filter to keep high frequency and add the result to the destination
    {
        kdDebug() << "Add high frequency to the original image" << endl;
        KisHLineIterator itLm1Blur = lm1Blur->createHLineIterator(0, 0, s.width(), false);
        KisHLineIterator itLm1 = lm1->createHLineIterator(0, 0, s.width(), false);
        KisHLineIterator itDst = gaussianPyramid->levels[0].device->createHLineIterator(0, 0, s.width(), true);
        for(int y = 0; y < s.height(); y++)
        {
            while(not itDst.isDone())
            {
                const float* itLm1BlurA = reinterpret_cast<const float*>( itLm1Blur.oldRawData() );
                const float* itLm1A = reinterpret_cast<const float*>( itLm1.oldRawData() );
                float* itDstA = reinterpret_cast<float*>( itDst.rawData() );
                for(int i = 0; i < depth; i++)
                {
                    itDstA[i] += (int)( itLm1A[i] - itLm1BlurA[i] );
//                     itDstA[i] = (int)( itLm1A[i]  );
                }
                ++itLm1Blur;
                ++itLm1;
                ++itDst;
            }
            itLm1Blur.nextRow();
            itLm1.nextRow();
            itDst.nextRow();
        }

    }
#endif
    
    tlb2.fromFloatDevice(gaussianPyramid->levels[0].device, dst, rect);
    
    delete gaussianPyramid;
    delete laplacianPyramid;
    setProgressDone(); // Must be called even if you don't really support progression
}
