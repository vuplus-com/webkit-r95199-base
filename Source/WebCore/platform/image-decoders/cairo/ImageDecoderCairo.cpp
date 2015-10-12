/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ImageDecoder.h"

#include <cairo.h>
#include <gtk/gtk.h>
#include <stdio.h>


namespace WebCore {

#if 0
static cairo_surface_t* _platform_surface = NULL;

void ImageFrame::copyReferenceToBitmapData(const ImageFrame& other)
{
	printf( "%s %s %d\n", __FILE__, __func__, __LINE__ );

    ASSERT(this != &other);
    m_backingStore = other.m_backingStore;
	if( m_backingStore.m_surface )
		cairo_surface_reference( m_backingStore.m_surface );

    m_bytes = m_backingStore.m_bytes;
    // FIXME: The rest of this function seems redundant with ImageFrame::copyBitmapData.
    m_size = other.m_size;
    setHasAlpha(other.m_hasAlpha);
}

bool ImageFrame::copyBitmapData(const ImageFrame& other)
{
	printf( "%s %s %d\n", __FILE__, __func__, __LINE__ );

    if (this == &other)
        return true;

	cairo_surface_t* image_surface = other.m_backingStore.m_surface;

	if( image_surface )
	{
	    m_size = other.m_size;
	    setHasAlpha(other.m_hasAlpha);

	    m_backingStore.m_surface = (cairo_image_surface_create( CAIRO_FORMAT_ARGB32, m_size.width(), m_size.height() ));
	    m_bytes = other.m_backingStore.m_bytes;
		m_backingStore.m_bytes = m_bytes;

		printf( "copy data???\n" );

		cairo_t* cr = cairo_create( m_backingStore.m_surface  );
		cairo_set_source_surface( cr, image_surface, 0, 0 );
		cairo_paint( cr );
		cairo_destroy(cr );

		printf( "copy done\n" );
	    return true;
	}

	printf( "error - copy image error\n" );

	return false;
}

bool ImageFrame::setSize(int newWidth, int newHeight)
{
    ASSERT(!m_backingStore);

	if( m_backingStore.m_surface )
	{
		m_backingStore.clear();
	}

	if( _platform_surface == NULL )
	{
		GList* result = gtk_window_list_toplevels();
		g_list_foreach ( result, (GFunc)g_object_ref, NULL);

		GList* list = g_list_first( result );

		while( list )
		{
			GtkWindow* w = GTK_WINDOW( list->data );

			if( w && gtk_window_is_active( w ) )
			{
				_platform_surface = gdk_window_create_similar_surface( gtk_widget_get_window ( GTK_WIDGET( w ) ), CAIRO_CONTENT_COLOR_ALPHA, 10, 10 );
				break;
			}

			list = g_list_next( list );
		}

		g_list_foreach ( result, (GFunc)g_object_unref, NULL);

	}

	cairo_surface_t* surface = NULL;
	cairo_surface_t* image_surface = NULL;

	int allocSize = newWidth*newHeight;

	if( _platform_surface && allocSize > 100 )
	{
		surface = cairo_surface_create_similar( _platform_surface, CAIRO_CONTENT_COLOR_ALPHA, newWidth, newHeight );
	}

	bool success = false;


	if( surface )
	{
		cairo_status_t status = cairo_surface_status( surface );

		if( status == CAIRO_STATUS_SUCCESS )
		{
			image_surface = cairo_surface_map_to_image( surface, NULL );

			if( image_surface )
			{
				success = true;
				cairo_surface_set_user_data( image_surface, (const cairo_user_data_key_t *)0x80, surface, NULL );

				int stride = cairo_image_surface_get_stride( image_surface );

				newWidth = stride / 4;
			}
		}

		if( success == false )
		{
			if( image_surface )
			{
				cairo_surface_unmap_image( surface, image_surface );
				image_surface = NULL;
			}

			cairo_surface_destroy( surface );
		}

	}


	if( image_surface == NULL )
	{
		printf( "can not map device surface to image - fallback\n" );

		if( surface )
		{
			cairo_surface_destroy( surface );
		}

		image_surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                                  newWidth,
                                                  newHeight);
	}

	if ( !image_surface )
	{
		printf( "error - create image surface failed\n" );
        return false;
	}

    m_backingStore.m_surface = image_surface;

    m_bytes = reinterpret_cast<PixelData*>(cairo_image_surface_get_data( m_backingStore.m_surface ));
	m_backingStore.m_bytes = m_bytes;
    m_size = IntSize(newWidth, newHeight);
	printf( "%s %s %d m_bytes = %x\n", __FILE__, __func__, __LINE__, m_bytes );

/*    zeroFillPixelData(); /* TBD */

/*	memset( m_bytes, 0xff, m_size.width()*m_size.height() ); */
    return true;
}

NativeImagePtr ImageFrame::asNewNativeImage() const
{
    cairo_surface_t * surface = NULL;//cairo_surface_map_to_image( m_backingStore.m_surface, NULL );

	if( !surface )
	{
		cairo_surface_t* device_surface = (cairo_surface_t*)cairo_surface_get_user_data( m_backingStore.m_surface, (const cairo_user_data_key_t *)0x80 );

		int stride = cairo_image_surface_get_stride( (cairo_surface_t*)m_backingStore.m_surface );

		surface = cairo_image_surface_create_for_data(
			reinterpret_cast<unsigned char*>(const_cast<PixelData*>(
				m_bytes)), CAIRO_FORMAT_ARGB32, width(), height(), stride);

		if( device_surface ){
			cairo_surface_set_user_data( surface, (const cairo_user_data_key_t *)0x80, device_surface, NULL );
		}
	}

	return surface;
}

#else

NativeImagePtr ImageFrame::asNewNativeImage() const
{
#ifdef WEBCORE_DEBUG
	printf( "Cairo image surface created with data %d %d\n", width(), height() );
#endif

    return cairo_image_surface_create_for_data(
        reinterpret_cast<unsigned char*>(const_cast<PixelData*>(
            m_bytes)), CAIRO_FORMAT_ARGB32, width(), height(),
        width() * sizeof(PixelData));
}

#endif

} // namespace WebCore
