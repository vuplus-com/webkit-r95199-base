/*
 * Copyright (C) 2006, 2007, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef DOMWindow_h
#define DOMWindow_h

#include "EventTarget.h"
#include "KURL.h"

namespace WebCore {

    class BarInfo;
    class CSSRuleList;
    class CSSStyleDeclaration;
    class Console;
    class Crypto;
    class DOMApplicationCache;
    class DOMSelection;
    class DOMURL;
    class Database;
    class DatabaseCallback;
    class Document;
    class Element;
    class EntryCallback;
    class ErrorCallback;
    class EventListener;
    class FileSystemCallback;
    class FloatRect;
    class Frame;
    class History;
    class IDBFactory;
    class Location;
    class MediaQueryList;
    class Navigator;
    class Node;
    class NotificationCenter;
    class Performance;
    class PostMessageTimer;
    class ScheduledAction;
    class Screen;
    class SecurityOrigin;
    class SerializedScriptValue;
    class Storage;
    class StorageInfo;
    class StyleMedia;
    class WebKitPoint;

#if ENABLE(REQUEST_ANIMATION_FRAME)
    class RequestAnimationFrameCallback;
#endif

    struct WindowFeatures;

    typedef Vector<RefPtr<MessagePort>, 1> MessagePortArray;

    typedef int ExceptionCode;

    enum SetLocationLocking { LockHistoryBasedOnGestureState, LockHistoryAndBackForwardList };

    class DOMWindow : public RefCounted<DOMWindow>, public EventTarget {
    public:
        static PassRefPtr<DOMWindow> create(Frame* frame) { return adoptRef(new DOMWindow(frame)); }
        virtual ~DOMWindow();

        virtual DOMWindow* toDOMWindow() { return this; }
        virtual ScriptExecutionContext* scriptExecutionContext() const;

        Frame* frame() const { return m_frame; }
        void disconnectFrame();

        void clear();

        PassRefPtr<MediaQueryList> matchMedia(const String&);

        void setSecurityOrigin(SecurityOrigin*);
        SecurityOrigin* securityOrigin() const { return m_securityOrigin.get(); }

        void setURL(const KURL& url) { m_url = url; }
        KURL url() const { return m_url; }

        unsigned pendingUnloadEventListeners() const;

        static bool dispatchAllPendingBeforeUnloadEvents();
        static void dispatchAllPendingUnloadEvents();

        static void adjustWindowRect(const FloatRect& screen, FloatRect& window, const FloatRect& pendingChanges);

        // FIXME: We can remove this function once V8 showModalDialog is changed to use DOMWindow.
        static void parseModalDialogFeatures(const String&, HashMap<String, String>&);

        bool allowPopUp(); // Call on first window, not target window.
        static bool allowPopUp(Frame* firstFrame);
        static bool canShowModalDialog(const Frame*);
        static bool canShowModalDialogNow(const Frame*);

        // DOM Level 0

        Screen* screen() const;
        History* history() const;
        Crypto* crypto() const;
        BarInfo* locationbar() const;
        BarInfo* menubar() const;
        BarInfo* personalbar() const;
        BarInfo* scrollbars() const;
        BarInfo* statusbar() const;
        BarInfo* toolbar() const;
        Navigator* navigator() const;
        Navigator* clientInformation() const { return navigator(); }

        Location* location() const;
        void setLocation(const String& location, DOMWindow* activeWindow, DOMWindow* firstWindow,
            SetLocationLocking = LockHistoryBasedOnGestureState);

        DOMSelection* getSelection();

        Element* frameElement() const;

        void focus();
        void blur();
        void close(ScriptExecutionContext* = 0);
        void print();
        void stop();

        PassRefPtr<DOMWindow> open(const String& urlString, const AtomicString& frameName, const String& windowFeaturesString,
            DOMWindow* activeWindow, DOMWindow* firstWindow);

        typedef void (*PrepareDialogFunction)(DOMWindow*, void* context);
        void showModalDialog(const String& urlString, const String& dialogFeaturesString,
            DOMWindow* activeWindow, DOMWindow* firstWindow, PrepareDialogFunction, void* functionContext);

        void alert(const String& message);
        bool confirm(const String& message);
        String prompt(const String& message, const String& defaultValue);
        String btoa(const String& stringToEncode, ExceptionCode&);
        String atob(const String& encodedString, ExceptionCode&);

        bool find(const String&, bool caseSensitive, bool backwards, bool wrap, bool wholeWord, bool searchInFrames, bool showDialog) const;

        bool offscreenBuffering() const;

        int outerHeight() const;
        int outerWidth() const;
        int innerHeight() const;
        int innerWidth() const;
        int screenX() const;
        int screenY() const;
        int screenLeft() const { return screenX(); }
        int screenTop() const { return screenY(); }
        int scrollX() const;
        int scrollY() const;
        int pageXOffset() const { return scrollX(); }
        int pageYOffset() const { return scrollY(); }

        bool closed() const;

        unsigned length() const;

        String name() const;
        void setName(const String&);

        String status() const;
        void setStatus(const String&);
        String defaultStatus() const;
        void setDefaultStatus(const String&);

        // This attribute is an alias of defaultStatus and is necessary for legacy uses.
        String defaultstatus() const { return defaultStatus(); }
        void setDefaultstatus(const String& status) { setDefaultStatus(status); }

        // Self-referential attributes

        DOMWindow* self() const;
        DOMWindow* window() const { return self(); }
        DOMWindow* frames() const { return self(); }

        DOMWindow* opener() const;
        DOMWindow* parent() const;
        DOMWindow* top() const;

        // DOM Level 2 AbstractView Interface

        Document* document() const;

        // CSSOM View Module

        PassRefPtr<StyleMedia> styleMedia() const;

        // DOM Level 2 Style Interface

        PassRefPtr<CSSStyleDeclaration> getComputedStyle(Element*, const String& pseudoElt) const;

        // WebKit extensions

        PassRefPtr<CSSRuleList> getMatchedCSSRules(Element*, const String& pseudoElt, bool authorOnly = true) const;
        double devicePixelRatio() const;

        PassRefPtr<WebKitPoint> webkitConvertPointFromPageToNode(Node*, const WebKitPoint*) const;
        PassRefPtr<WebKitPoint> webkitConvertPointFromNodeToPage(Node*, const WebKitPoint*) const;        

        Console* console() const;

        void printErrorMessage(const String&);
        String crossDomainAccessErrorMessage(DOMWindow* activeWindow);

        void pageDestroyed();
        void resetGeolocation();

        void postMessage(PassRefPtr<SerializedScriptValue> message, const MessagePortArray*, const String& targetOrigin, DOMWindow* source, ExceptionCode&);
        // FIXME: remove this when we update the ObjC bindings (bug #28774).
        void postMessage(PassRefPtr<SerializedScriptValue> message, MessagePort*, const String& targetOrigin, DOMWindow* source, ExceptionCode&);
        void postMessageTimerFired(PassOwnPtr<PostMessageTimer>);

        void scrollBy(int x, int y) const;
        void scrollTo(int x, int y) const;
        void scroll(int x, int y) const { scrollTo(x, y); }

        void moveBy(float x, float y) const;
        void moveTo(float x, float y) const;

        void resizeBy(float x, float y) const;
        void resizeTo(float width, float height) const;

        // Timers
        int setTimeout(PassOwnPtr<ScheduledAction>, int timeout, ExceptionCode&);
        void clearTimeout(int timeoutId);
        int setInterval(PassOwnPtr<ScheduledAction>, int timeout, ExceptionCode&);
        void clearInterval(int timeoutId);

        // WebKit animation extensions
#if ENABLE(REQUEST_ANIMATION_FRAME)
        int webkitRequestAnimationFrame(PassRefPtr<RequestAnimationFrameCallback>, Element*);
        void webkitCancelRequestAnimationFrame(int id);
#endif

        // Events
        // EventTarget API
        virtual bool addEventListener(const AtomicString& eventType, PassRefPtr<EventListener>, bool useCapture);
        virtual bool removeEventListener(const AtomicString& eventType, EventListener*, bool useCapture);
        virtual void removeAllEventListeners();

        using EventTarget::dispatchEvent;
        bool dispatchEvent(PassRefPtr<Event> prpEvent, PassRefPtr<EventTarget> prpTarget);
        void dispatchLoadEvent();
        void dispatchTimedEvent(PassRefPtr<Event> event, Document* target, double* startTime, double* endTime);

        DEFINE_ATTRIBUTE_EVENT_LISTENER(abort);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(beforeunload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(blur);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplay);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplaythrough);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(change);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(click);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(contextmenu);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dblclick);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drag);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragenter);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragleave);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drop);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(durationchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(emptied);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ended);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(focus);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(hashchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(input);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(invalid);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keydown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keypress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keyup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(load);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadeddata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadedmetadata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(message);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousedown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousemove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseout);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousewheel);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(offline);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(online);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pagehide);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pageshow);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pause);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(play);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(playing);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(popstate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(progress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ratechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(reset);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(resize);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(scroll);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(search);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeked);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeking);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(select);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(stalled);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(storage);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(submit);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(suspend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(timeupdate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(unload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(volumechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(waiting);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitbeginfullscreen);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitendfullscreen);

        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationstart, webkitAnimationStart);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationiteration, webkitAnimationIteration);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationend, webkitAnimationEnd);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkittransitionend, webkitTransitionEnd);

        void captureEvents();
        void releaseEvents();

        void finishedLoading();

        using RefCounted<DOMWindow>::ref;
        using RefCounted<DOMWindow>::deref;

#if ENABLE(BLOB)
        DOMURL* webkitURL() const;
#endif

#if ENABLE(SQL_DATABASE)
        PassRefPtr<Database> openDatabase(const String& name, const String& version, const String& displayName, unsigned long estimatedSize, PassRefPtr<DatabaseCallback> creationCallback, ExceptionCode&);
#endif

#if ENABLE(DEVICE_ORIENTATION)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(devicemotion);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(deviceorientation);
#endif

#if ENABLE(DOM_STORAGE)
        // HTML 5 key/value storage
        Storage* sessionStorage(ExceptionCode&) const;
        Storage* localStorage(ExceptionCode&) const;
#endif

#if ENABLE(FILE_SYSTEM)
        // They are placed here and in all capital letters so they can be checked against the constants in the
        // IDL at compile time.
        enum FileSystemType {
            TEMPORARY,
            PERSISTENT,
            EXTERNAL,
        };
        void webkitRequestFileSystem(int type, long long size, PassRefPtr<FileSystemCallback>, PassRefPtr<ErrorCallback>);
        void webkitResolveLocalFileSystemURL(const String&, PassRefPtr<EntryCallback>, PassRefPtr<ErrorCallback>);
#endif

#if ENABLE(INDEXED_DATABASE)
        IDBFactory* webkitIndexedDB() const;
#endif

#if ENABLE(NOTIFICATIONS)
        NotificationCenter* webkitNotifications() const;
#endif

#if ENABLE(QUOTA)
        StorageInfo* webkitStorageInfo() const;
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
        DOMApplicationCache* applicationCache() const;
        DOMApplicationCache* optionalApplicationCache() const { return m_applicationCache.get(); }
#endif

#if ENABLE(ORIENTATION_EVENTS)
        // This is the interface orientation in degrees. Some examples are:
        //  0 is straight up; -90 is when the device is rotated 90 clockwise;
        //  90 is when rotated counter clockwise.
        int orientation() const;

        DEFINE_ATTRIBUTE_EVENT_LISTENER(orientationchange);
#endif

#if ENABLE(TOUCH_EVENTS)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchmove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchcancel);
#endif

#if ENABLE(WEB_TIMING)
        Performance* performance() const;
#endif

	enum
	{
	VK_UNDEFINED = 0,
	VK_CANCEL = 3,
	VK_BACK_SPACE = 8,
	VK_TAB = 9,
	VK_CLEAR = 12,
	VK_ENTER = 13,
	VK_SHIFT = 16,
	VK_CONTROL = 17,
	VK_ALT = 18,
	VK_PAUSE = 19,
	VK_CAPS_LOCK = 20,
	VK_KANA = 21,
	VK_FINAL = 24,
	VK_KANJI = 25,
	VK_ESCAPE = 27,
	VK_CONVERT = 28,
	VK_NONCONVERT = 29,
	VK_ACCEPT = 30,
	VK_MODECHANGE = 31,
	VK_SPACE = 32,
	VK_PAGE_UP = 33,
	VK_PAGE_DOWN = 34,
	VK_END = 35,
	VK_HOME = 36,
	VK_LEFT = 37,
	VK_UP = 38,
	VK_RIGHT = 39,
	VK_DOWN = 40,
	VK_COMMA = 44,
	VK_PERIOD = 46,
	VK_SLASH = 47,
	VK_0 = 48,
	VK_1 = 49,
	VK_2 = 50,
	VK_3 = 51,
	VK_4 = 52,
	VK_5 = 53,
	VK_6 = 54,
	VK_7 = 55,
	VK_8 = 56,
	VK_9 = 57,
	VK_SEMICOLON = 59,
	VK_EQUALS = 61,
	VK_A = 65,
	VK_B = 66,
	VK_C = 67,
	VK_D = 68,
	VK_E = 69,
	VK_F = 70,
	VK_G = 71,
	VK_H = 72,
	VK_I = 73,
	VK_J = 74,
	VK_K = 75,
	VK_L = 76,
	VK_M = 77,
	VK_N = 78,
	VK_O = 79,
	VK_P = 80,
	VK_Q = 81,
	VK_R = 82,
	VK_S = 83,
	VK_T = 84,
	VK_U = 85,
	VK_V = 86,
	VK_W = 87,
	VK_X = 88,
	VK_Y = 89,
	VK_Z = 90,
	VK_OPEN_BRACKET = 91,
	VK_BACK_SLASH = 92,
	VK_CLOSE_BRACKET = 93,
	VK_NUMPAD0 = 96,
	VK_NUMPAD1 = 97,
	VK_NUMPAD2 = 98,
	VK_NUMPAD3 = 99,
	VK_NUMPAD4 = 100,
	VK_NUMPAD5 = 101,
	VK_NUMPAD6 = 102,
	VK_NUMPAD7 = 103,
	VK_NUMPAD8 = 104,
	VK_NUMPAD9 = 105,
	VK_MULTIPLY = 106,
	VK_ADD = 107,
	VK_SEPARATER = 108,
	VK_SUBTRACT = 109,
	VK_DECIMAL = 110,
	VK_DIVIDE = 111,
	VK_F1 = 112,
	VK_F2 = 113,
	VK_F3 = 114,
	VK_F4 = 115,
	VK_F5 = 116,
	VK_F6 = 117,
	VK_F7 = 118,
	VK_F8 = 119,
	VK_F9 = 120,
	VK_F10 = 121,
	VK_F11 = 122,
	VK_F12 = 123,
	VK_DELETE = 127,
	VK_NUM_LOCK = 144,
	VK_SCROLL_LOCK = 145,
	VK_PRINTSCREEN = 154,
	VK_INSERT = 155,
	VK_HELP = 156,
	VK_META = 157,
	VK_BACK_QUOTE = 192,
	VK_QUOTE = 222,
	VK_RED = 403,
	VK_GREEN = 404,
	VK_YELLOW = 405,
	VK_BLUE = 406,
	VK_GREY = 407,
	VK_BROWN = 408,
	VK_POWER = 409,
	VK_DIMMER = 410,
	VK_WINK = 411,
	VK_REWIND = 412,
	VK_STOP = 413,
	VK_EJECT_TOGGLE = 414,
	VK_PLAY = 415,
	VK_RECORD = 416,
	VK_FAST_FWD = 417,
	VK_PLAY_SPEED_UP = 418,
	VK_PLAY_SPEED_DOWN = 419,
	VK_PLAY_SPEED_RESET = 420,
	VK_RECORD_SPEED_NEXT = 421,
	VK_GO_TO_START = 422,
	VK_GO_TO_END = 423,
	VK_TRACK_PREV = 424,
	VK_TRACK_NEXT = 425,
	VK_RANDOM_TOGGLE = 426,
	VK_CHANNEL_UP = 427,
	VK_CHANNEL_DOWN = 428,
	VK_STORE_FAVORITE_0 = 429,
	VK_STORE_FAVORITE_1 = 430,
	VK_STORE_FAVORITE_2 = 431,
	VK_STORE_FAVORITE_3 = 432,
	VK_RECALL_FAVORITE_0 = 433,
	VK_RECALL_FAVORITE_1 = 434,
	VK_RECALL_FAVORITE_2 = 435,
	VK_RECALL_FAVORITE_3 = 436,
	VK_CLEAR_FAVORITE_0 = 437,
	VK_CLEAR_FAVORITE_1 = 438,
	VK_CLEAR_FAVORITE_2 = 439,
	VK_CLEAR_FAVORITE_3 = 440,
	VK_SCAN_CHANNELS_TOGGLE = 441,
	VK_PINP_TOGGLE = 442,
	VK_SPLIT_SCREEN_TOGGLE = 443,
	VK_DISPLAY_SWAP = 444,
	VK_SCREEN_MODE_NEXT = 445,
	VK_VIDEO_MODE_NEXT = 446,
	VK_VOLUME_UP = 447,
	VK_VOLUME_DOWN = 448,
	VK_MUTE = 449,
	VK_SURROUND_MODE_NEXT = 450,
	VK_BALANCE_RIGHT = 451,
	VK_BALANCE_LEFT = 452,
	VK_FADER_FRONT = 453,
	VK_FADER_REAR = 454,
	VK_BASS_BOOST_UP = 455,
	VK_BASS_BOOST_DOWN = 456,
	VK_INFO = 457,
	VK_GUIDE = 458,
	VK_TELETEXT = 459,
	VK_SUBTITLE = 460,
	VK_BACK = 461,
	VK_MENU = 462,
	};

	void debug( const String& arg ); /* OIPF */

    private:
        DOMWindow(Frame*);

        virtual void refEventTarget() { ref(); }
        virtual void derefEventTarget() { deref(); }
        virtual EventTargetData* eventTargetData();
        virtual EventTargetData* ensureEventTargetData();

        static Frame* createWindow(const String& urlString, const AtomicString& frameName, const WindowFeatures&,
            DOMWindow* activeWindow, Frame* firstFrame, Frame* openerFrame,
            PrepareDialogFunction = 0, void* functionContext = 0);
        bool isInsecureScriptAccess(DOMWindow* activeWindow, const String& urlString);

        RefPtr<SecurityOrigin> m_securityOrigin;
        KURL m_url;

        bool m_shouldPrintWhenFinishedLoading;
        Frame* m_frame;
        mutable RefPtr<Screen> m_screen;
        mutable RefPtr<DOMSelection> m_selection;
        mutable RefPtr<History> m_history;
        mutable RefPtr<Crypto>  m_crypto;
        mutable RefPtr<BarInfo> m_locationbar;
        mutable RefPtr<BarInfo> m_menubar;
        mutable RefPtr<BarInfo> m_personalbar;
        mutable RefPtr<BarInfo> m_scrollbars;
        mutable RefPtr<BarInfo> m_statusbar;
        mutable RefPtr<BarInfo> m_toolbar;
        mutable RefPtr<Console> m_console;
        mutable RefPtr<Navigator> m_navigator;
        mutable RefPtr<Location> m_location;
        mutable RefPtr<StyleMedia> m_media;

        EventTargetData m_eventTargetData;

        String m_status;
        String m_defaultStatus;

#if ENABLE(DOM_STORAGE)
        mutable RefPtr<Storage> m_sessionStorage;
        mutable RefPtr<Storage> m_localStorage;
#endif

#if ENABLE(INDEXED_DATABASE)
        mutable RefPtr<IDBFactory> m_idbFactory;
#endif

#if ENABLE(OFFLINE_WEB_APPLICATIONS)
        mutable RefPtr<DOMApplicationCache> m_applicationCache;
#endif

#if ENABLE(NOTIFICATIONS)
        mutable RefPtr<NotificationCenter> m_notifications;
#endif

#if ENABLE(WEB_TIMING)
        mutable RefPtr<Performance> m_performance;
#endif

#if ENABLE(BLOB)
        mutable RefPtr<DOMURL> m_domURL;
#endif

#if ENABLE(QUOTA)
        mutable RefPtr<StorageInfo> m_storageInfo;
#endif
    };

    inline String DOMWindow::status() const
    {
        return m_status;
    }

    inline String DOMWindow::defaultStatus() const
    {
        return m_defaultStatus;
    } 

} // namespace WebCore

#endif // DOMWindow_h
