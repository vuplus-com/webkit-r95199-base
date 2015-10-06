/*
 * MediaPlayerPrivateElisIPC.cpp
 *
 *  Created on: 2013. 9. 29.
 *      Author: kdhong
 */

#include "config.h"
#include "MediaPlayerPrivateElisIPC.h"

#if ENABLE_VIDEO && ENABLE_ELIS_MEDIA

#include "ColorSpace.h"
#include "Document.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "GraphicsTypes.h"
#include "IntRect.h"
#include "KURL.h"
#include "MIMETypeRegistry.h"
#include "MediaPlayer.h"
#include "NotImplemented.h"
#include "SecurityOrigin.h"
#include "TimeRanges.h"
#include <GOwnPtr.h>
#include <limits>
#include <math.h>
#include <wtf/text/CString.h>
#include "Logging.h"
#include <gtk/gtk.h>
#include "PlatformContextCairo.h"
#include <dlfcn.h>

#if defined(XP_UNIX)
#include "RefPtrCairo.h"
#if PLATFORM(X11)
#define Bool int // this got undefined somewhere
#define Status int // ditto
#include <X11/extensions/Xrender.h>
#include <cairo/cairo-xlib.h>
#include <gdk/gdkx.h>
#elif defined(GDK_WINDOWING_DIRECTFB)
#include <gdk/gdk.h>
#endif
#elif defined(GDK_WINDOWING_WIN32)
#include "PluginMessageThrottlerWin.h"
#include <gdk/gdkwin32.h>
#endif

using namespace std;

namespace WebCore {

static void* _ipc_handle = NULL;
static bool _load_failed = false;

static int (*html5_server_load)( const char* );
static int (*html5_server_cancelLoad)();
static int (*html5_server_play)();
static int (*html5_server_pause)();
static int (*html5_server_seek)( float );
static int (*html5_server_setRate)( float );
static int (*html5_server_setVolume)( float );
static int (*html5_server_setSize)( int, int );
static int (*html5_server_setMuted)( bool );
static int (*html5_server_getStatus)( int*, int*, int*, int*, int*, uint64_t*, uint64_t* );
static int (*html5_server_getVideoDimension)( int*, int* );

static void _ipc_init()
{
	if( _ipc_handle == NULL && _load_failed == false )
	{
		fprintf( stderr, "ipc_init\n");
		html5_server_load = NULL;
		html5_server_cancelLoad = NULL;
		html5_server_play = NULL;
		html5_server_pause = NULL;
		html5_server_seek = NULL;
		html5_server_setRate = NULL;
		html5_server_setVolume = NULL;
		html5_server_setSize = NULL;
		html5_server_setMuted = NULL;
		html5_server_getStatus = NULL;
		html5_server_getVideoDimension = NULL;

		char* ipc_dl_path = "/webkit/usr/lib/libhtml5_video.so";

		if( getenv( "HTML5_IPC_PATH" ) )
		{
			ipc_dl_path = getenv( "HTML5_IPC_PATH" );
		}

		_ipc_handle = dlopen( ipc_dl_path, RTLD_NOW );

		fprintf( stderr, "ipc handle %x\n", _ipc_handle );

		if( _ipc_handle )
		{
			*(void **) (&html5_server_load) = dlsym(_ipc_handle, "html5_video_client_load");
			*(void **) (&html5_server_cancelLoad) = dlsym(_ipc_handle, "html5_video_client_cancel_load");
			*(void **) (&html5_server_play) = dlsym(_ipc_handle, "html5_video_client_play");
			*(void **) (&html5_server_pause) = dlsym(_ipc_handle, "html5_video_client_pause");
			*(void **) (&html5_server_seek) = dlsym(_ipc_handle, "html5_video_client_seek");
			*(void **) (&html5_server_setRate) = dlsym(_ipc_handle, "html5_video_client_setRate");
			*(void **) (&html5_server_setVolume) = dlsym(_ipc_handle, "html5_video_client_setVolume");
			*(void **) (&html5_server_setSize) = dlsym(_ipc_handle, "html5_video_client_setSize");
			*(void **) (&html5_server_setMuted) = dlsym(_ipc_handle, "html5_video_client_setMuted");
			*(void **) (&html5_server_getStatus) = dlsym(_ipc_handle, "html5_video_get_status");
			*(void **) (&html5_server_getVideoDimension) = dlsym(_ipc_handle, "html5_video_get_video_dimension");

		}
		else
		{
			fprintf( stderr, "error %s\n", dlerror() );
			_load_failed = true;
		}
	}
}

bool MediaPlayerPrivateElisIPC::isAvailable()
{
    return true;
}

PassOwnPtr<MediaPlayerPrivateInterface> MediaPlayerPrivateElisIPC::create(MediaPlayer* player)
{
    return adoptPtr(new MediaPlayerPrivateElisIPC(player));
}


MediaPlayerPrivateElisIPC::MediaPlayerPrivateElisIPC(MediaPlayer* player)
    : m_player(player)
    , m_seekTime(0)
    , m_changingRate(false)
    , m_endTime(numeric_limits<float>::infinity())
    , m_networkState(MediaPlayer::Empty)
    , m_readyState(MediaPlayer::HaveNothing)
    , m_isStreaming(false)
    , m_size(IntSize())
    , m_mediaLocationCurrentIndex(0)
    , m_resetPipeline(false)
    , m_paused(true)
    , m_seeking(false)
    , m_buffering(false)
    , m_playbackRate(1)
    , m_errorOccured(false)
    , m_mediaDuration(0)
    , m_startedBuffering(false)
    , m_fillTimer(this, &MediaPlayerPrivateElisIPC::fillTimerFired)
    , m_maxTimeLoaded(0)
    , m_bufferingPercentage(0)
    , m_preload(MediaPlayer::Auto)
    , m_delayingLoad(false)
    , m_mediaDurationKnown(true)
    , m_volumeTimerHandler(0)
    , m_muteTimerHandler(0)
    , m_hasVideo(false)
    , m_hasAudio(false)
    , m_audioTimerHandler(0)
    , m_videoTimerHandler(0)
	, m_videoSize( 0, 0 )
	, m_totalBytes( 0 )
	, m_bytesLoaded( 0 )
{
	fprintf( stderr, "MediaPlayer IPC created\n" );

	if( _ipc_handle == NULL )
		_ipc_init();
}

MediaPlayerPrivateElisIPC::~MediaPlayerPrivateElisIPC()
{
	fprintf(stderr, "MediaPlayer IPC Destroyed\n" );

    if (m_fillTimer.isActive())
        m_fillTimer.stop();

    fprintf( stderr, "cancel load %x", html5_server_cancelLoad );

	if( html5_server_cancelLoad )
		html5_server_cancelLoad();

    m_player = 0;
}

void MediaPlayerPrivateElisIPC::registerMediaEngine(MediaEngineRegistrar registrar)
{
    if (isAvailable())
        registrar(create, getSupportedTypes, supportsType, 0, 0, 0);
}

void MediaPlayerPrivateElisIPC::getSupportedTypes(HashSet<String>& types)
{
    types.add( "video/mp4" );
}

MediaPlayer::SupportsType MediaPlayerPrivateElisIPC::supportsType(const String& type, const String& codecs)
{
    if (type.isNull() || type.isEmpty())
        return MediaPlayer::IsNotSupported;

    // spec says we should not return "probably" if the codecs string is empty
    if ( type == "video/mp4" )
        return MediaPlayer::IsSupported;

    return MediaPlayer::IsNotSupported;
}


void MediaPlayerPrivateElisIPC::load(const String& url)
{
	fprintf( stderr, "Load %s %x\n", url.utf8().data(), html5_server_load );

	m_size = IntSize( 0, 0 );
	m_prevStatus = 0;

    if (m_fillTimer.isActive())
        m_fillTimer.stop();

	m_fillTimer.startRepeating(1.0);

	if( html5_server_load )
		(*html5_server_load)( (const char*)url.utf8().data() );
}


float MediaPlayerPrivateElisIPC::playbackPosition() const
{
    return m_playbackPosition;
}

void MediaPlayerPrivateElisIPC::prepareToPlay()
{
	/* TBD */
}

void MediaPlayerPrivateElisIPC::play()
{
	if( html5_server_play )
		html5_server_play();
}

void MediaPlayerPrivateElisIPC::pause()
{
	if( html5_server_pause )
		html5_server_pause();
}

float MediaPlayerPrivateElisIPC::duration() const
{
    if (m_errorOccured)
        return 0.0f;

    // Media duration query failed already, don't attempt new useless queries.
    if (!m_mediaDurationKnown)
        return numeric_limits<float>::infinity();

    if (m_mediaDuration)
        return m_mediaDuration;

    return 0.0f;
}

float MediaPlayerPrivateElisIPC::currentTime() const
{
    if (m_errorOccured)
        return 0.0f;

    if (m_seeking)
        return m_seekTime;

    return playbackPosition();

}

void MediaPlayerPrivateElisIPC::seek(float time)
{
    // Avoid useless seeking.
    if (time == playbackPosition())
        return;

    if (m_errorOccured)
        return;

    // Extract the integer part of the time (seconds) and the
    // fractional part (microseconds). Attempt to round the
    // microseconds so no floating point precision is lost and we can
    // perform an accurate seek.
    m_seekTime = time;
    m_seeking = true;

	if( html5_server_seek )
		html5_server_seek( time );
}

bool MediaPlayerPrivateElisIPC::paused() const
{
    return m_paused;
}

bool MediaPlayerPrivateElisIPC::seeking() const
{
    return m_seeking;
}

// Returns the size of the video
IntSize MediaPlayerPrivateElisIPC::naturalSize() const
{
//    if (!hasVideo())
//        return IntSize();

    // TODO: handle possible clean aperture data. See
    // https://bugzilla.gnome.org/show_bug.cgi?id=596571
    // TODO: handle possible transformation matrix. See
    // https://bugzilla.gnome.org/show_bug.cgi?id=596326

    // Get the video PAR and original size.
    return m_videoSize;
}

void MediaPlayerPrivateElisIPC::videoChanged()
{
}

void MediaPlayerPrivateElisIPC::notifyPlayerOfVideo()
{
    m_videoTimerHandler = 0;
}

void MediaPlayerPrivateElisIPC::audioChanged()
{
}

void MediaPlayerPrivateElisIPC::notifyPlayerOfAudio()
{
}

void MediaPlayerPrivateElisIPC::setVolume(float volume)
{
	if( html5_server_setVolume )
		html5_server_setVolume( volume );
}

void MediaPlayerPrivateElisIPC::notifyPlayerOfVolumeChange()
{
}

void MediaPlayerPrivateElisIPC::volumeChanged()
{
}

void MediaPlayerPrivateElisIPC::setRate(float rate)
{
	if( html5_server_setRate )
		html5_server_setRate( rate );
}

MediaPlayer::NetworkState MediaPlayerPrivateElisIPC::networkState() const
{
    return m_networkState;
}

MediaPlayer::ReadyState MediaPlayerPrivateElisIPC::readyState() const
{
    return m_readyState;
}

PassRefPtr<TimeRanges> MediaPlayerPrivateElisIPC::buffered() const
{
	RefPtr<TimeRanges> timeRanges = TimeRanges::create();
    return timeRanges.release();
}

void MediaPlayerPrivateElisIPC::fillTimerFired(Timer<MediaPlayerPrivateElisIPC>*)
{
	int playTime = 0;
	int endTime = 0;
	int speed = 100;
	int status = 0;
	int inSeek = 0;
	uint64_t totalBytes = 0;
	uint64_t currentOffset = 0;

	if( html5_server_getStatus )
		html5_server_getStatus( &playTime, &endTime, &speed, &status, &inSeek, &totalBytes, &currentOffset );

	int videoWidth = 0, videoHeight = 0;

	if( html5_server_getVideoDimension )
		html5_server_getVideoDimension( &videoWidth, &videoHeight );

	float duration = endTime / 1000.0;
	float currentPos = playTime / 1000.0;
	float ratio = speed / 100.0;

	fprintf( stderr, "Duration %f, currentPos %f, ratio %f videoWidth = %d, videoHeight = %d Download[%lld:%lld]\n",
						duration, currentPos, ratio, videoWidth, videoHeight, totalBytes, currentOffset );

	m_totalBytes = totalBytes;
	m_bytesLoaded = m_totalBytes;

	if( m_mediaDuration != duration )
	{
		m_mediaDuration = duration;
		m_mediaDurationKnown = true;
		durationChanged();
	}

	MediaPlayer::ReadyState readyState = MediaPlayer::HaveMetadata;
	MediaPlayer::NetworkState networkState = MediaPlayer::Loading;

	switch( status )
	{
	case 0: /* STOPPED */
//		readyState = MediaPlayer::HaveNothing;
//		networkState = MediaPlayer::Idle;
		break;

	case 1: /* PLAYING */
		if( videoWidth > 0 )
			readyState = MediaPlayer::HaveEnoughData;
		networkState = MediaPlayer::Loading;
		break;

	case 2: /* Paused */
		break;
	case 3: /* Connecting */
		readyState = MediaPlayer::HaveMetadata;
		networkState = MediaPlayer::Loading;
		break;
	case 4: /* Buffering */
		readyState = MediaPlayer::HaveMetadata;
		networkState = MediaPlayer::Loading;
		break;
	case 5: /* Finished */
		readyState = MediaPlayer::HaveEnoughData;
		networkState = MediaPlayer::Loaded;
		break;
	case 6: /* Error */
		readyState = MediaPlayer::HaveNothing;
		break;
	}

	if( status == 5 && m_prevStatus != 5 )
	{
		m_prevStatus = status;
		m_player->playbackStateChanged();
	}

	if( status == 1 && m_prevStatus != 1 )
	{
		m_prevStatus = status;
		m_player->playbackStateChanged();
	}

	if( readyState != m_readyState )
	{
		m_readyState = readyState;
		m_player->readyStateChanged();
	}

	if( networkState != m_networkState )
	{
		m_networkState = networkState;
		m_player->networkStateChanged();
	}

	if( m_videoSize.width() != videoWidth || m_videoSize.height() != videoHeight )
	{
		m_videoSize.setWidth( videoWidth );
		m_videoSize.setHeight( videoHeight );

		m_player->sizeChanged();
	}

	m_player->timeChanged();

	bool paused = false;
	if( ratio == 0.0f )
		paused = true;

	if( paused != m_paused && status != 5 && status !=  6 )
	{
		m_paused = paused;
	}

	if( status == 5 )
		m_paused = false;

	if( inSeek == 0 )
		m_seeking = false;

	m_playbackPosition = currentPos;
}

float MediaPlayerPrivateElisIPC::maxTimeSeekable() const
{
    if (m_errorOccured)
        return 0.0f;

    LOG_VERBOSE(Media, "maxTimeSeekable");
    // infinite duration means live stream
    if (isinf(duration()))
        return 0.0f;

    return duration();
}

float MediaPlayerPrivateElisIPC::maxTimeLoaded() const
{
    if (m_errorOccured)
        return 0.0f;

    float loaded = m_maxTimeLoaded;
    if (!loaded && !m_fillTimer.isActive())
        loaded = duration();
    LOG_VERBOSE(Media, "maxTimeLoaded: %f", loaded);
    return loaded;
}

unsigned MediaPlayerPrivateElisIPC::bytesLoaded() const
{
    if (!m_mediaDuration)
        return 0;

    return m_bytesLoaded;
}

unsigned MediaPlayerPrivateElisIPC::totalBytes() const
{
    if (m_errorOccured)
        return 0;

    return m_totalBytes;
}

unsigned MediaPlayerPrivateElisIPC::decodedFrameCount() const
{
	return 0;
}

unsigned MediaPlayerPrivateElisIPC::droppedFrameCount() const
{
	return 0;
}

unsigned MediaPlayerPrivateElisIPC::audioDecodedByteCount() const
{
	return 0;
}

unsigned MediaPlayerPrivateElisIPC::videoDecodedByteCount() const
{
	return 0;
}

void MediaPlayerPrivateElisIPC::updateAudioSink()
{
}


void MediaPlayerPrivateElisIPC::sourceChanged()
{
}

void MediaPlayerPrivateElisIPC::cancelLoad()
{
    if (m_fillTimer.isActive())
        m_fillTimer.stop();

    fprintf( stderr, "cancel load %x", html5_server_cancelLoad );

	if( html5_server_cancelLoad )
		html5_server_cancelLoad();
}

void MediaPlayerPrivateElisIPC::updateStates()
{
}


bool MediaPlayerPrivateElisIPC::loadNextLocation()
{
}

void MediaPlayerPrivateElisIPC::loadStateChanged()
{
    updateStates();
}

void MediaPlayerPrivateElisIPC::sizeChanged()
{
    notImplemented();
}

void MediaPlayerPrivateElisIPC::timeChanged()
{
    updateStates();
    m_player->timeChanged();
}

void MediaPlayerPrivateElisIPC::didEnd()
{
    // EOS was reached but in case of reverse playback the position is
    // not always 0. So to not confuse the HTMLMediaElement we
    // synchronize position and duration values.
    float now = currentTime();
    if (now > 0) {
        m_mediaDuration = now;
        m_mediaDurationKnown = true;
        m_player->durationChanged();
    }
}

void MediaPlayerPrivateElisIPC::cacheDuration()
{
    // Reset cached media duration
    m_mediaDuration = 0;
}

void MediaPlayerPrivateElisIPC::durationChanged()
{
    m_player->durationChanged();
}

bool MediaPlayerPrivateElisIPC::supportsMuting() const
{
    return true;
}

void MediaPlayerPrivateElisIPC::setMuted(bool muted)
{
	if( html5_server_setMuted )
		html5_server_setMuted( muted );
}

void MediaPlayerPrivateElisIPC::notifyPlayerOfMute()
{
    m_muteTimerHandler = 0;

}

void MediaPlayerPrivateElisIPC::muteChanged()
{
    if (m_muteTimerHandler)
        g_source_remove(m_muteTimerHandler);
}

void MediaPlayerPrivateElisIPC::loadingFailed(MediaPlayer::NetworkState error)
{
    m_errorOccured = true;
    if (m_networkState != error) {
        m_networkState = error;
        m_player->networkStateChanged();
    }
    if (m_readyState != MediaPlayer::HaveNothing) {
        m_readyState = MediaPlayer::HaveNothing;
        m_player->readyStateChanged();
    }
}

void MediaPlayerPrivateElisIPC::setSize(const IntSize& size)
{
	fprintf( stderr, "setsize %d %d\n", size.width(), size.height() );
    m_size = size;
}

void MediaPlayerPrivateElisIPC::setVisible(bool visible)
{
}



void MediaPlayerPrivateElisIPC::paint(GraphicsContext* context, const IntRect& rect)
{

    if (context->paintingDisabled())
    {
    	fprintf( stderr, "%s %s %d\n", __FILE__, __func__, __LINE__ );
        return;
    }

    if (!m_player->visible())
    {
    	fprintf( stderr, "%s %s %d\n", __FILE__, __func__, __LINE__ );
        return;
    }

#if defined(XP_UNIX)
#if PLATFORM(X11)
		Display* display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

		cairo_t* cr = context->platformContext()->cr();
		cairo_save(cr);

		cairo_rectangle(cr,
				rect.x(), rect.y(),
				rect.width(), rect.height());

		cairo_set_source_rgba(cr, 0xff, 0x0, 0x0, 0xff );

        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	    cairo_fill(cr);

		cairo_restore(cr);
#else
		cairo_t* cr = context->platformContext()->cr();
		cairo_save(cr);

		cairo_rectangle(cr,
				rect.x(), rect.y(),
				rect.width(), rect.height());

		cairo_set_source_rgba(cr, 0, 0x0, 0x0, 0 );

        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	    cairo_fill(cr);

		cairo_restore(cr);

#endif
#endif
}

static HashSet<String> mimeTypeCache()
{

}

bool MediaPlayerPrivateElisIPC::hasSingleSecurityOrigin() const
{
    return true;
}

bool MediaPlayerPrivateElisIPC::supportsFullscreen() const
{
    return true;
}

PlatformMedia MediaPlayerPrivateElisIPC::platformMedia() const
{
}

void MediaPlayerPrivateElisIPC::setPreload(MediaPlayer::Preload preload)
{
}

}
#endif // USE(GSTREAMER)


