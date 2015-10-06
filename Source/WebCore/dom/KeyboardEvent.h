/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KeyboardEvent_h
#define KeyboardEvent_h

#include "EventDispatchMediator.h"
#include "UIEventWithKeyState.h"
#include <wtf/Vector.h>

namespace WebCore {

    class EventDispatcher;
    class Node;
    class PlatformKeyboardEvent;

#if PLATFORM(MAC)
    struct KeypressCommand {
        KeypressCommand() { }
        KeypressCommand(const String& commandName) : commandName(commandName) { ASSERT(isASCIILower(commandName[0U])); }
        KeypressCommand(const String& commandName, const String& text) : commandName(commandName), text(text) { ASSERT(commandName == "insertText:"); }

        String commandName; // Actually, a selector name - it may have a trailing colon, and a name that can be different from an editor command name.
        String text;
    };
#endif
    
    // Introduced in DOM Level 3
    class KeyboardEvent : public UIEventWithKeyState {
    public:
        enum KeyLocationCode {
            DOM_KEY_LOCATION_STANDARD      = 0x00,
            DOM_KEY_LOCATION_LEFT          = 0x01,
            DOM_KEY_LOCATION_RIGHT         = 0x02,
            DOM_KEY_LOCATION_NUMPAD        = 0x03,

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
        
        static PassRefPtr<KeyboardEvent> create()
        {
            return adoptRef(new KeyboardEvent);
        }
        static PassRefPtr<KeyboardEvent> create(const PlatformKeyboardEvent& platformEvent, AbstractView* view)
        {
            return adoptRef(new KeyboardEvent(platformEvent, view));
        }
        static PassRefPtr<KeyboardEvent> create(const AtomicString& type, bool canBubble, bool cancelable, AbstractView* view,
            const String& keyIdentifier, unsigned keyLocation,
            bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, bool altGraphKey)
        {
            return adoptRef(new KeyboardEvent(type, canBubble, cancelable, view, keyIdentifier, keyLocation,
                ctrlKey, altKey, shiftKey, metaKey, altGraphKey));
        }
        virtual ~KeyboardEvent();
    
        void initKeyboardEvent(const AtomicString& type, bool canBubble, bool cancelable, AbstractView*,
                               const String& keyIdentifier, unsigned keyLocation,
                               bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, bool altGraphKey = false);
    
        const String& keyIdentifier() const { return m_keyIdentifier; }
        unsigned keyLocation() const { return m_keyLocation; }

        bool getModifierState(const String& keyIdentifier) const;

        bool altGraphKey() const { return m_altGraphKey; }
    
        const PlatformKeyboardEvent* keyEvent() const { return m_keyEvent.get(); }

        int keyCode() const; // key code for keydown and keyup, character for keypress
        int charCode() const; // character code for keypress, 0 for keydown and keyup
    
        virtual bool isKeyboardEvent() const;
        virtual int which() const;

#if PLATFORM(MAC)
        // We only have this need to store keypress command info on the Mac.
        Vector<KeypressCommand>& keypressCommands() { return m_keypressCommands; }
#endif

    private:
        KeyboardEvent();
        KeyboardEvent(const PlatformKeyboardEvent&, AbstractView*);
        KeyboardEvent(const AtomicString& type, bool canBubble, bool cancelable, AbstractView*,
                      const String& keyIdentifier, unsigned keyLocation,
                      bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, bool altGraphKey);

        OwnPtr<PlatformKeyboardEvent> m_keyEvent;
        String m_keyIdentifier;
        unsigned m_keyLocation;
        bool m_altGraphKey : 1;

#if PLATFORM(MAC)
        // Commands that were sent by AppKit when interpreting the event. Doesn't include input method commands.
        Vector<KeypressCommand> m_keypressCommands;
#endif
    };

    KeyboardEvent* findKeyboardEvent(Event*);

class KeyboardEventDispatchMediator : public EventDispatchMediator {
public:
    static PassRefPtr<KeyboardEventDispatchMediator> create(PassRefPtr<KeyboardEvent>);
private:
    explicit KeyboardEventDispatchMediator(PassRefPtr<KeyboardEvent>);
    virtual bool dispatchEvent(EventDispatcher*) const;
};

} // namespace WebCore

#endif // KeyboardEvent_h
