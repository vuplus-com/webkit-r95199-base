#ifndef KeyEvent_h
#define KeyEvent_h

#include "EventDispatchMediator.h"
#include "UIEventWithKeyState.h"
#include <wtf/Vector.h>

namespace WebCore {

    class EventDispatcher;
    class Node;
    class PlatformKeyboardEvent;
    // Introduced in DOM Level 3
    class KeyEvent : public UIEventWithKeyState {
    public:
		enum
		{
		 	  CHAR_UNDEFINED				 = 0x0FFFF,
		 	  DOM_VK_0						 = 0x30,
		 	  DOM_VK_1						 = 0x31,
		 	  DOM_VK_2						 = 0x32,
		 	  DOM_VK_3						 = 0x33,
		 	  DOM_VK_4						 = 0x34,
		 	  DOM_VK_5						 = 0x35,
		 	  DOM_VK_6						 = 0x36,
		 	  DOM_VK_7						 = 0x37,
		 	  DOM_VK_8						 = 0x38,
		 	  DOM_VK_9						 = 0x39,
		 	  DOM_VK_A						 = 0x41,
		 	  DOM_VK_ACCEPT 				 = 0x1E,
		 	  DOM_VK_ADD					 = 0x6B,
		 	  DOM_VK_AGAIN					 = 0xFFC9,
		 	  DOM_VK_ALL_CANDIDATES 		 = 0x0100,
		 	  DOM_VK_ALPHANUMERIC			 = 0x00F0,
		 	  DOM_VK_ALT					 = 0x12,
		 	  DOM_VK_ALT_GRAPH				 = 0xFF7E,
		 	  DOM_VK_AMPERSAND				 = 0x96,
		 	  DOM_VK_ASTERISK				 = 0x97,
		 	  DOM_VK_AT 					 = 0x0200,
		 	  DOM_VK_B						 = 0x42,
		 	  DOM_VK_BACK_QUOTE 			 = 0xC0,
		 	  DOM_VK_BACK_SLASH 			 = 0x5C,
		 	  DOM_VK_BACK_SPACE 			 = 0x08,
		 	  DOM_VK_BRACELEFT				 = 0xA1,
		 	  DOM_VK_BRACERIGHT 			 = 0xA2,
		 	  DOM_VK_C						 = 0x43,
		 	  DOM_VK_CANCEL 				 = 0x03,
		 	  DOM_VK_CAPS_LOCK				 = 0x14,
		 	  DOM_VK_CIRCUMFLEX 			 = 0x0202,
		 	  DOM_VK_CLEAR					 = 0x0C,
		 	  DOM_VK_CLOSE_BRACKET			 = 0x5D,
		 	  DOM_VK_CODE_INPUT 			 = 0x0102,
		 	  DOM_VK_COLON					 = 0x0201,
		 	  DOM_VK_COMMA					 = 0x2C,
		 	  DOM_VK_COMPOSE				 = 0xFF20,
		 	  DOM_VK_CONTROL				 = 0x11,
		 	  DOM_VK_CONVERT				 = 0x1C,
		 	  DOM_VK_COPY					 = 0xFFCD,
		 	  DOM_VK_CUT					 = 0xFFD1,
		 	  DOM_VK_D						 = 0x44,
		 	  DOM_VK_DEAD_ABOVEDOT			 = 0x86,
		 	  DOM_VK_DEAD_ABOVERING 		 = 0x88,
		 	  DOM_VK_DEAD_ACUTE 			 = 0x81,
		 	  DOM_VK_DEAD_BREVE 			 = 0x85,
		 	  DOM_VK_DEAD_CARON 			 = 0x8A,
		 	  DOM_VK_DEAD_CEDILLA			 = 0x8B,
		 	  DOM_VK_DEAD_CIRCUMFLEX		 = 0x82,
		 	  DOM_VK_DEAD_DIAERESIS 		 = 0x87,
		 	  DOM_VK_DEAD_DOUBLEACUTE		 = 0x89,
		 	  DOM_VK_DEAD_GRAVE 			 = 0x80,
		 	  DOM_VK_DEAD_IOTA				 = 0x8D,
		 	  DOM_VK_DEAD_MACRON			 = 0x84,
		 	  DOM_VK_DEAD_OGONEK			 = 0x8C,
		 	  DOM_VK_DEAD_SEMIVOICED_SOUND	 = 0x8F,
		 	  DOM_VK_DEAD_TILDE 			 = 0x83,
		 	  DOM_VK_DEAD_VOICED_SOUND		 = 0x8E,
		 	  DOM_VK_DECIMAL				 = 0x6E,
		 	  DOM_VK_DELETE 				 = 0x7F,
		 	  DOM_VK_DIVIDE 				 = 0x6F,
		 	  DOM_VK_DOLLAR 				 = 0x0203,
		 	  DOM_VK_DOWN					 = 0x28,
		 	  DOM_VK_E						 = 0x45,
		 	  DOM_VK_END					 = 0x23,
		 	  DOM_VK_ENTER					 = 0x0D,
		 	  DOM_VK_EQUALS 				 = 0x3D,
		 	  DOM_VK_ESCAPE 				 = 0x1B,
		 	  DOM_VK_EURO_SIGN				 = 0x0204,
		 	  DOM_VK_EXCLAMATION_MARK		 = 0x0205,
		 	  DOM_VK_F						 = 0x46,
		 	  DOM_VK_F1 					 = 0x70,
		 	  DOM_VK_F10					 = 0x79,
		 	  DOM_VK_F11					 = 0x7A,
		 	  DOM_VK_F12					 = 0x7B,
		 	  DOM_VK_F13					 = 0xF000,
		 	  DOM_VK_F14					 = 0xF001,
		 	  DOM_VK_F15					 = 0xF002,
		 	  DOM_VK_F16					 = 0xF003,
		 	  DOM_VK_F17					 = 0xF004,
		 	  DOM_VK_F18					 = 0xF005,
		 	  DOM_VK_F19					 = 0xF006,
		 	  DOM_VK_F2 					 = 0x71,
		 	  DOM_VK_F20					 = 0xF007,
		 	  DOM_VK_F21					 = 0xF008,
		 	  DOM_VK_F22					 = 0xF009,
		 	  DOM_VK_F23					 = 0xF00A,
		 	  DOM_VK_F24					 = 0xF00B,
		 	  DOM_VK_F3 					 = 0x72,
		 	  DOM_VK_F4 					 = 0x73,
		 	  DOM_VK_F5 					 = 0x74,
		 	  DOM_VK_F6 					 = 0x75,
		 	  DOM_VK_F7 					 = 0x76,
		 	  DOM_VK_F8 					 = 0x77,
		 	  DOM_VK_F9 					 = 0x78,
		 	  DOM_VK_FINAL					 = 0x18,
		 	  DOM_VK_FIND					 = 0xFFD0,
		 	  DOM_VK_FULL_WIDTH 			 = 0x00F3,
		 	  DOM_VK_G						 = 0x47,
		 	  DOM_VK_GREATER				 = 0xA0,
		 	  DOM_VK_H						 = 0x48,
		 	  DOM_VK_HALF_WIDTH 			 = 0x00F4,
		 	  DOM_VK_HELP					 = 0x9C,
		 	  DOM_VK_HIRAGANA				 = 0x00F2,
		 	  DOM_VK_HOME					 = 0x24,
		 	  DOM_VK_I						 = 0x49,
		 	  DOM_VK_INSERT 				 = 0x9B,
		 	  DOM_VK_INVERTED_EXCLAMATION_MARK = 0x0206,
		 	  DOM_VK_J						 = 0x4A,
		 	  DOM_VK_JAPANESE_HIRAGANA		 = 0x0104,
		 	  DOM_VK_JAPANESE_KATAKANA		 = 0x0103,
		 	  DOM_VK_JAPANESE_ROMAN 		 = 0x0105,
		 	  DOM_VK_K						 = 0x4B,
		 	  DOM_VK_KANA					 = 0x15,
		 	  DOM_VK_KANJI					 = 0x19,
		 	  DOM_VK_KATAKANA				 = 0x00F1,
		 	  DOM_VK_KP_DOWN				 = 0xE1,
		 	  DOM_VK_KP_LEFT				 = 0xE2,
		 	  DOM_VK_KP_RIGHT				 = 0xE3,
		 	  DOM_VK_KP_UP					 = 0xE0,
		 	  DOM_VK_L						 = 0x4C,
		 	  DOM_VK_LEFT					 = 0x25,
		 	  DOM_VK_LEFT_PARENTHESIS		 = 0x0207,
		 	  DOM_VK_LESS					 = 0x99,
		 	  DOM_VK_M						 = 0x4D,
		 	  DOM_VK_META					 = 0x9D,
		 	  DOM_VK_MINUS					 = 0x2D,
		 	  DOM_VK_MODECHANGE 			 = 0x1F,
		 	  DOM_VK_MULTIPLY				 = 0x6A,
		 	  DOM_VK_N						 = 0x4E,
		 	  DOM_VK_NONCONVERT 			 = 0x1D,
		 	  DOM_VK_NUM_LOCK				 = 0x90,
		 	  DOM_VK_NUMBER_SIGN			 = 0x0208,
		 	  DOM_VK_NUMPAD0				 = 0x60,
		 	  DOM_VK_NUMPAD1				 = 0x61,
		 	  DOM_VK_NUMPAD2				 = 0x62,
		 	  DOM_VK_NUMPAD3				 = 0x63,
		 	  DOM_VK_NUMPAD4				 = 0x64,
		 	  DOM_VK_NUMPAD5				 = 0x65,
		 	  DOM_VK_NUMPAD6				 = 0x66,
		 	  DOM_VK_NUMPAD7				 = 0x67,
		 	  DOM_VK_NUMPAD8				 = 0x68,
		 	  DOM_VK_NUMPAD9				 = 0x69,
		 	  DOM_VK_O						 = 0x4F,
		 	  DOM_VK_OPEN_BRACKET			 = 0x5B,
		 	  DOM_VK_P						 = 0x50,
		 	  DOM_VK_PAGE_DOWN				 = 0x22,
		 	  DOM_VK_PAGE_UP				 = 0x21,
		 	  DOM_VK_PASTE					 = 0xFFCF,
		 	  DOM_VK_PAUSE					 = 0x13,
		 	  DOM_VK_PERIOD 				 = 0x2E,
		 	  DOM_VK_PLUS					 = 0x0209,
		 	  DOM_VK_PREVIOUS_CANDIDATE 	 = 0x0101,
		 	  DOM_VK_PRINTSCREEN			 = 0x9A,
		 	  DOM_VK_PROPS					 = 0xFFCA,
		 	  DOM_VK_Q						 = 0x51,
		 	  DOM_VK_QUOTE					 = 0xDE,
		 	  DOM_VK_QUOTEDBL				 = 0x98,
		 	  DOM_VK_R						 = 0x52,
		 	  DOM_VK_RIGHT					 = 0x27,
		 	  DOM_VK_RIGHT_PARENTHESIS		 = 0x020A,
		 	  DOM_VK_ROMAN_CHARACTERS		 = 0x00F5,
		 	  DOM_VK_S						 = 0x53,
		 	  DOM_VK_SCROLL_LOCK			 = 0x91,
		 	  DOM_VK_SEMICOLON				 = 0x3B,
		 	  DOM_VK_SEPARATER				 = 0x6C,
		 	  DOM_VK_SHIFT					 = 0x10,
		 	  DOM_VK_SLASH					 = 0x2F,
		 	  DOM_VK_SPACE					 = 0x20,
		 	  DOM_VK_STOP					 = 0xFFC8,
		 	  DOM_VK_SUBTRACT				 = 0x6D,
		 	  DOM_VK_T						 = 0x54,
		 	  DOM_VK_TAB					 = 0x09,
		 	  DOM_VK_U						 = 0x55,
		 	  DOM_VK_UNDEFINED				 = 0x0,
		 	  DOM_VK_UNDERSCORE 			 = 0x020B,
		 	  DOM_VK_UNDO					 = 0xFFCB,
		 	  DOM_VK_UP 					 = 0x26,
		 	  DOM_VK_V						 = 0x56,
		 	  DOM_VK_W						 = 0x57,
		 	  DOM_VK_X						 = 0x58,
		 	  DOM_VK_Y						 = 0x59,
		 	  DOM_VK_Z						 = 0x5A,

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
		
        enum KeyLocationCode {
            DOM_KEY_LOCATION_STANDARD      = 0x00,
            DOM_KEY_LOCATION_LEFT          = 0x01,
            DOM_KEY_LOCATION_RIGHT         = 0x02,
            DOM_KEY_LOCATION_NUMPAD        = 0x03
        };
        
        static PassRefPtr<KeyEvent> create()
        {
            return adoptRef(new KeyEvent);
        }
        static PassRefPtr<KeyEvent> create(const PlatformKeyboardEvent& platformEvent, AbstractView* view)
        {
            return adoptRef(new KeyEvent(platformEvent, view));
        }
        static PassRefPtr<KeyEvent> create(const AtomicString& type, bool canBubble, bool cancelable, AbstractView* view,
            const String& keyIdentifier, unsigned keyLocation,
            bool ctrlKey, bool altKey, bool shiftKey, bool metaKey, bool altGraphKey)
        {
            return adoptRef(new KeyEvent(type, canBubble, cancelable, view, keyIdentifier, keyLocation,
                ctrlKey, altKey, shiftKey, metaKey, altGraphKey));
        }
        virtual ~KeyEvent();
    
        void initKeyEvent(const AtomicString& type, bool canBubble, bool cancelable, AbstractView*,
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
        KeyEvent();
        KeyEvent(const PlatformKeyboardEvent&, AbstractView*);
        KeyEvent(const AtomicString& type, bool canBubble, bool cancelable, AbstractView*,
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

    KeyEvent* findKeyEvent(Event*);

class KeyEventDispatchMediator : public EventDispatchMediator {
public:
    static PassRefPtr<KeyEventDispatchMediator> create(PassRefPtr<KeyEvent>);
private:
    explicit KeyEventDispatchMediator(PassRefPtr<KeyEvent>);
    virtual bool dispatchEvent(EventDispatcher*) const;
};

} // namespace WebCore

#endif // KeyboardEvent_h

