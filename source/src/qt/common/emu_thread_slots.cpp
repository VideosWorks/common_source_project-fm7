/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2016.8.26 Split from emu_thread.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [QObject SLOTs depended by VM]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "emu_thread.h"

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "menu_flags.h"

// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10


const int auto_key_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	{0x0a,	0x000 | 0x0d},	// Enter(Unix)
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x20,	0x000 | 0x20},	// ' '

	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0x32},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x36},	// '&'
	{0x27,	0x100 | 0x37},	// '''
	{0x28,	0x100 | 0x38},	// '('
	{0x29,	0x100 | 0x39},	// ')'
	{0x2a,	0x100 | 0xba},	// '*'
	{0x2b,	0x100 | 0xbb},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'

	{0x30,	0x000 | 0x30},	// '0'
	{0x31,	0x000 | 0x31},	// '1'
	{0x32,	0x000 | 0x32},	// '2'
	{0x33,	0x000 | 0x33},	// '3'
	{0x34,	0x000 | 0x34},	// '4'
	{0x35,	0x000 | 0x35},	// '5'
	{0x36,	0x000 | 0x36},	// '6'
	{0x37,	0x000 | 0x37},	// '7'
	{0x38,	0x000 | 0x38},	// '8'
	{0x39,	0x000 | 0x39},	// '9'

	{0x3a,	0x000 | 0xba},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x100 | 0xbd},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x000 | 0xc0},	// '@'

	{0x41,	0x400 | 0x41},	// 'A'
	{0x42,	0x400 | 0x42},	// 'B'
	{0x43,	0x400 | 0x43},	// 'C'
	{0x44,	0x400 | 0x44},	// 'D'
	{0x45,	0x400 | 0x45},	// 'E'
	{0x46,	0x400 | 0x46},	// 'F'
	{0x47,	0x400 | 0x47},	// 'G'
	{0x48,	0x400 | 0x48},	// 'H'
	{0x49,	0x400 | 0x49},	// 'I'
	{0x4a,	0x400 | 0x4a},	// 'J'
	{0x4b,	0x400 | 0x4b},	// 'K'
	{0x4c,	0x400 | 0x4c},	// 'L'
	{0x4d,	0x400 | 0x4d},	// 'M'
	{0x4e,	0x400 | 0x4e},	// 'N'
	{0x4f,	0x400 | 0x4f},	// 'O'
	{0x50,	0x400 | 0x50},	// 'P'
	{0x51,	0x400 | 0x51},	// 'Q'
	{0x52,	0x400 | 0x52},	// 'R'
	{0x53,	0x400 | 0x53},	// 'S'
	{0x54,	0x400 | 0x54},	// 'T'
	{0x55,	0x400 | 0x55},	// 'U'
	{0x56,	0x400 | 0x56},	// 'V'
	{0x57,	0x400 | 0x57},	// 'W'
	{0x58,	0x400 | 0x58},	// 'X'
	{0x59,	0x400 | 0x59},	// 'Y'
	{0x5a,	0x400 | 0x5a},	// 'Z'

	{0x5b,	0x000 | 0xdb},	// '['
	{0x5c,	0x000 | 0xdc},	// '\'
	{0x5d,	0x000 | 0xdd},	// ']'
	{0x5e,	0x000 | 0xde},	// '^'
	{0x5f,	0x100 | 0xe2},	// '_'
	{0x60,	0x100 | 0xc0},	// '`'

	{0x61,	0x800 | 0x41},	// 'a'
	{0x62,	0x800 | 0x42},	// 'b'
	{0x63,	0x800 | 0x43},	// 'c'
	{0x64,	0x800 | 0x44},	// 'd'
	{0x65,	0x800 | 0x45},	// 'e'
	{0x66,	0x800 | 0x46},	// 'f'
	{0x67,	0x800 | 0x47},	// 'g'
	{0x68,	0x800 | 0x48},	// 'h'
	{0x69,	0x800 | 0x49},	// 'i'
	{0x6a,	0x800 | 0x4a},	// 'j'
	{0x6b,	0x800 | 0x4b},	// 'k'
	{0x6c,	0x800 | 0x4c},	// 'l'
	{0x6d,	0x800 | 0x4d},	// 'm'
	{0x6e,	0x800 | 0x4e},	// 'n'
	{0x6f,	0x800 | 0x4f},	// 'o'
	{0x70,	0x800 | 0x50},	// 'p'
	{0x71,	0x800 | 0x51},	// 'q'
	{0x72,	0x800 | 0x52},	// 'r'
	{0x73,	0x800 | 0x53},	// 's'
	{0x74,	0x800 | 0x54},	// 't'
	{0x75,	0x800 | 0x55},	// 'u'
	{0x76,	0x800 | 0x56},	// 'v'
	{0x77,	0x800 | 0x57},	// 'w'
	{0x78,	0x800 | 0x58},	// 'x'
	{0x79,	0x800 | 0x59},	// 'y'
	{0x7a,	0x800 | 0x5a},	// 'z'

	{0x7b,	0x100 | 0xdb},	// '{'
	{0x7c,	0x100 | 0xdc},	// '|'
	{0x7d,	0x100 | 0xdd},	// '}'
	{0x7e,	0x100 | 0xde},	// '~'

	{0xa1,	0x300 | 0xbe},	// '
	{0xa2,	0x300 | 0xdb},	// '
	{0xa3,	0x300 | 0xdd},	// '
	{0xa4,	0x300 | 0xbc},	// '
	{0xa5,	0x300 | 0xbf},	// '･'
	{0xa6,	0x300 | 0x30},	// 'ｦ'
	{0xa7,	0x300 | 0x33},	// 'ｧ'
	{0xa8,	0x300 | 0x45},	// 'ｨ'
	{0xa9,	0x300 | 0x34},	// 'ｩ'
	{0xaa,	0x300 | 0x35},	// 'ｪ'
	{0xab,	0x300 | 0x36},	// 'ｫ'
	{0xac,	0x300 | 0x37},	// 'ｬ'
	{0xad,	0x300 | 0x38},	// 'ｭ'
	{0xae,	0x300 | 0x39},	// 'ｮ'
	{0xaf,	0x300 | 0x5a},	// 'ｯ'
	{0xb0,	0x200 | 0xdc},	// 'ｰ'
	{0xb1,	0x200 | 0x33},	// 'ｱ'
	{0xb2,	0x200 | 0x45},	// 'ｲ'
	{0xb3,	0x200 | 0x34},	// 'ｳ'
	{0xb4,	0x200 | 0x35},	// 'ｴ'
	{0xb5,	0x200 | 0x36},	// 'ｵ'
	{0xb6,	0x200 | 0x54},	// 'ｶ'
	{0xb7,	0x200 | 0x47},	// 'ｷ'
	{0xb8,	0x200 | 0x48},	// 'ｸ'
	{0xb9,	0x200 | 0xba},	// 'ｹ'
	{0xba,	0x200 | 0x42},	// 'ｺ'
	{0xbb,	0x200 | 0x58},	// 'ｻ'
	{0xbc,	0x200 | 0x44},	// 'ｼ'
	{0xbd,	0x200 | 0x52},	// 'ｽ'
	{0xbe,	0x200 | 0x50},	// 'ｾ'
	{0xbf,	0x200 | 0x43},	// 'ｿ'
	{0xc0,	0x200 | 0x51},	// 'ﾀ'
	{0xc1,	0x200 | 0x41},	// 'ﾁ'
	{0xc2,	0x200 | 0x5a},	// 'ﾂ'
	{0xc3,	0x200 | 0x57},	// 'ﾃ'
	{0xc4,	0x200 | 0x53},	// 'ﾄ'
	{0xc5,	0x200 | 0x55},	// 'ﾅ'
	{0xc6,	0x200 | 0x49},	// 'ﾆ'
	{0xc7,	0x200 | 0x31},	// 'ﾇ'
	{0xc8,	0x200 | 0xbc},	// 'ﾈ'
	{0xc9,	0x200 | 0x4b},	// 'ﾉ'
	{0xca,	0x200 | 0x46},	// 'ﾊ'
	{0xcb,	0x200 | 0x56},	// 'ﾋ'
	{0xcc,	0x200 | 0x32},	// 'ﾌ'
	{0xcd,	0x200 | 0xde},	// 'ﾍ'
	{0xce,	0x200 | 0xbd},	// 'ﾎ'
	{0xcf,	0x200 | 0x4a},	// 'ﾏ'
	{0xd0,	0x200 | 0x4e},	// 'ﾐ'
	{0xd1,	0x200 | 0xdd},	// 'ﾑ'
	{0xd2,	0x200 | 0xbf},	// 'ﾒ'
	{0xd3,	0x200 | 0x4d},	// 'ﾓ'
	{0xd4,	0x200 | 0x37},	// 'ﾔ'
	{0xd5,	0x200 | 0x38},	// 'ﾕ'
	{0xd6,	0x200 | 0x39},	// 'ﾖ'
	{0xd7,	0x200 | 0x4f},	// 'ﾗ'
	{0xd8,	0x200 | 0x4c},	// 'ﾘ'
	{0xd9,	0x200 | 0xbe},	// 'ﾙ'
	{0xda,	0x200 | 0xbb},	// 'ﾚ'
	{0xdb,	0x200 | 0xe2},	// 'ﾛ'
	{0xdc,	0x200 | 0x30},	// 'ﾜ'
	{0xdd,	0x200 | 0x59},	// 'ﾝ'
	{0xde,	0x200 | 0xc0},	// 'ﾞ'
	{0xdf,	0x200 | 0xdb},	// 'ﾟ'
	{-1, -1},
};

const int auto_key_table_base_us[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	{0x0a,	0x000 | 0x0d},	// Enter(Unix)
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x20,	0x000 | 0x20},	// ' '
	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0xba},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x37},	// '&'
	{0x27,	0x000 | 0xba},	// '''
	{0x28,	0x100 | 0x39},	// '('
	{0x29,	0x100 | 0x30},	// ')'
	{0x2a,	0x100 | 0x38},	// '*'
	{0x2b,	0x100 | 0xde},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'
	
	{0x30,	0x000 | 0x30},	// '0'
	{0x31,	0x000 | 0x31},	// '1'
	{0x32,	0x000 | 0x32},	// '2'
	{0x33,	0x000 | 0x33},	// '3'
	{0x34,	0x000 | 0x34},	// '4'
	{0x35,	0x000 | 0x35},	// '5'
	{0x36,	0x000 | 0x36},	// '6'
	{0x37,	0x000 | 0x37},	// '7'
	{0x38,	0x000 | 0x38},	// '8'
	{0x39,	0x000 | 0x39},	// '9'
	
	{0x3a,	0x100 | 0xbb},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x000 | 0xde},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x100 | 0x32},	// '@'
	
	{0x41,	0x400 | 0x41},	// 'A'
	{0x42,	0x400 | 0x42},	// 'B'
	{0x43,	0x400 | 0x43},	// 'C'
	{0x44,	0x400 | 0x44},	// 'D'
	{0x45,	0x400 | 0x45},	// 'E'
	{0x46,	0x400 | 0x46},	// 'F'
	{0x47,	0x400 | 0x47},	// 'G'
	{0x48,	0x400 | 0x48},	// 'H'
	{0x49,	0x400 | 0x49},	// 'I'
	{0x4a,	0x400 | 0x4a},	// 'J'
	{0x4b,	0x400 | 0x4b},	// 'K'
	{0x4c,	0x400 | 0x4c},	// 'L'
	{0x4d,	0x400 | 0x4d},	// 'M'
	{0x4e,	0x400 | 0x4e},	// 'N'
	{0x4f,	0x400 | 0x4f},	// 'O'
	{0x50,	0x400 | 0x50},	// 'P'
	{0x51,	0x400 | 0x51},	// 'Q'
	{0x52,	0x400 | 0x52},	// 'R'
	{0x53,	0x400 | 0x53},	// 'S'
	{0x54,	0x400 | 0x54},	// 'T'
	{0x55,	0x400 | 0x55},	// 'U'
	{0x56,	0x400 | 0x56},	// 'V'
	{0x57,	0x400 | 0x57},	// 'W'
	{0x58,	0x400 | 0x58},	// 'X'
	{0x59,	0x400 | 0x59},	// 'Y'
	{0x5a,	0x400 | 0x5a},	// 'Z'

	{0x5b,	0x000 | 0xc0},	// '['
	{0x5c,	0x000 | 0xe2},	// '\'
	{0x5d,	0x000 | 0xdb},	// ']'
	{0x5e,	0x100 | 0x36},	// '^'
	{0x5f,	0x100 | 0xbd},	// '_'
	{0x60,	0x000 | 0xdd},	// '`'
	
	{0x61,	0x800 | 0x41},	// 'a'
	{0x62,	0x800 | 0x42},	// 'b'
	{0x63,	0x800 | 0x43},	// 'c'
	{0x64,	0x800 | 0x44},	// 'd'
	{0x65,	0x800 | 0x45},	// 'e'
	{0x66,	0x800 | 0x46},	// 'f'
	{0x67,	0x800 | 0x47},	// 'g'
	{0x68,	0x800 | 0x48},	// 'h'
	{0x69,	0x800 | 0x49},	// 'i'
	{0x6a,	0x800 | 0x4a},	// 'j'
	{0x6b,	0x800 | 0x4b},	// 'k'
	{0x6c,	0x800 | 0x4c},	// 'l'
	{0x6d,	0x800 | 0x4d},	// 'm'
	{0x6e,	0x800 | 0x4e},	// 'n'
	{0x6f,	0x800 | 0x4f},	// 'o'
	{0x70,	0x800 | 0x50},	// 'p'
	{0x71,	0x800 | 0x51},	// 'q'
	{0x72,	0x800 | 0x52},	// 'r'
	{0x73,	0x800 | 0x53},	// 's'
	{0x74,	0x800 | 0x54},	// 't'
	{0x75,	0x800 | 0x55},	// 'u'
	{0x76,	0x800 | 0x56},	// 'v'
	{0x77,	0x800 | 0x57},	// 'w'
	{0x78,	0x800 | 0x58},	// 'x'
	{0x79,	0x800 | 0x59},	// 'y'
	{0x7a,	0x800 | 0x5a},	// 'z'

	{0x7b,	0x100 | 0xc0},	// '{'
	{0x7c,	0x100 | 0xe2},	// '|'
	{0x7d,	0x100 | 0xdb},	// '}'
	{0x7e,	0x100 | 0xdd},	// '~'

	{0xa1,	0x300 | 0xbe},	// '
	{0xa2,	0x300 | 0xdb},	// '
	{0xa3,	0x300 | 0xdd},	// '
	{0xa4,	0x300 | 0xbc},	// '
	{0xa5,	0x300 | 0xbf},	// '･'
	{0xa6,	0x300 | 0x30},	// 'ｦ'
	{0xa7,	0x300 | 0x33},	// 'ｧ'
	{0xa8,	0x300 | 0x45},	// 'ｨ'
	{0xa9,	0x300 | 0x34},	// 'ｩ'
	{0xaa,	0x300 | 0x35},	// 'ｪ'
	{0xab,	0x300 | 0x36},	// 'ｫ'
	{0xac,	0x300 | 0x37},	// 'ｬ'
	{0xad,	0x300 | 0x38},	// 'ｭ'
	{0xae,	0x300 | 0x39},	// 'ｮ'
	{0xaf,	0x300 | 0x5a},	// 'ｯ'
	{0xb0,	0x200 | 0xdc},	// 'ｰ'
	{0xb1,	0x200 | 0x33},	// 'ｱ'
	{0xb2,	0x200 | 0x45},	// 'ｲ'
	{0xb3,	0x200 | 0x34},	// 'ｳ'
	{0xb4,	0x200 | 0x35},	// 'ｴ'
	{0xb5,	0x200 | 0x36},	// 'ｵ'
	{0xb6,	0x200 | 0x54},	// 'ｶ'
	{0xb7,	0x200 | 0x47},	// 'ｷ'
	{0xb8,	0x200 | 0x48},	// 'ｸ'
	{0xb9,	0x200 | 0xba},	// 'ｹ'
	{0xba,	0x200 | 0x42},	// 'ｺ'
	{0xbb,	0x200 | 0x58},	// 'ｻ'
	{0xbc,	0x200 | 0x44},	// 'ｼ'
	{0xbd,	0x200 | 0x52},	// 'ｽ'
	{0xbe,	0x200 | 0x50},	// 'ｾ'
	{0xbf,	0x200 | 0x43},	// 'ｿ'
	{0xc0,	0x200 | 0x51},	// 'ﾀ'
	{0xc1,	0x200 | 0x41},	// 'ﾁ'
	{0xc2,	0x200 | 0x5a},	// 'ﾂ'
	{0xc3,	0x200 | 0x57},	// 'ﾃ'
	{0xc4,	0x200 | 0x53},	// 'ﾄ'
	{0xc5,	0x200 | 0x55},	// 'ﾅ'
	{0xc6,	0x200 | 0x49},	// 'ﾆ'
	{0xc7,	0x200 | 0x31},	// 'ﾇ'
	{0xc8,	0x200 | 0xbc},	// 'ﾈ'
	{0xc9,	0x200 | 0x4b},	// 'ﾉ'
	{0xca,	0x200 | 0x46},	// 'ﾊ'
	{0xcb,	0x200 | 0x56},	// 'ﾋ'
	{0xcc,	0x200 | 0x32},	// 'ﾌ'
	{0xcd,	0x200 | 0xde},	// 'ﾍ'
	{0xce,	0x200 | 0xbd},	// 'ﾎ'
	{0xcf,	0x200 | 0x4a},	// 'ﾏ'
	{0xd0,	0x200 | 0x4e},	// 'ﾐ'
	{0xd1,	0x200 | 0xdd},	// 'ﾑ'
	{0xd2,	0x200 | 0xbf},	// 'ﾒ'
	{0xd3,	0x200 | 0x4d},	// 'ﾓ'
	{0xd4,	0x200 | 0x37},	// 'ﾔ'
	{0xd5,	0x200 | 0x38},	// 'ﾕ'
	{0xd6,	0x200 | 0x39},	// 'ﾖ'
	{0xd7,	0x200 | 0x4f},	// 'ﾗ'
	{0xd8,	0x200 | 0x4c},	// 'ﾘ'
	{0xd9,	0x200 | 0xbe},	// 'ﾙ'
	{0xda,	0x200 | 0xbb},	// 'ﾚ'
	{0xdb,	0x200 | 0xe2},	// 'ﾛ'
	{0xdc,	0x200 | 0x30},	// 'ﾜ'
	{0xdd,	0x200 | 0x59},	// 'ﾝ'
	{0xde,	0x200 | 0xc0},	// 'ﾞ'
	{0xdf,	0x200 | 0xdb},	// 'ﾟ'
	{-1, -1},
};

void EmuThreadClass::do_start_auto_key(QString ctext)
{
#ifdef USE_AUTO_KEY
	if(using_flags->is_use_auto_key()) {
		QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
		QByteArray array;
		clipBoardText = ctext;
		array = codec->fromUnicode(clipBoardText);
		if(clipBoardText.size() > 0) {
			static int auto_key_table[256];
			static bool initialized = false;
			if(!initialized) {
				memset(auto_key_table, 0, sizeof(auto_key_table));
				if(using_flags->is_use_auto_key_us()) {
					for(int i = 0;; i++) {
						if(auto_key_table_base_us[i][0] == -1) {
							break;
						}
						auto_key_table[auto_key_table_base_us[i][0]] = auto_key_table_base_us[i][1];
					}
				} else {
					for(int i = 0;; i++) {
						if(auto_key_table_base[i][0] == -1) {
							break;
						}
						auto_key_table[auto_key_table_base[i][0]] = auto_key_table_base[i][1];
					}
				}
				if(using_flags->is_use_vm_auto_key_table()) {
					if(using_flags->is_use_auto_key_us()) {
						for(int i = 0;; i++) {
							if(auto_key_table_base_us[i][0] == -1) {
								break;
							}
							auto_key_table[auto_key_table_base_us[i][0]] = auto_key_table_base_us[i][1];
						}
					} else {
						for(int i = 0;; i++) {
							if(auto_key_table_base[i][0] == -1) {
								break;
							}
							auto_key_table[auto_key_table_base[i][0]] = auto_key_table_base[i][1];
						}
					}
				}
				initialized = true;
			}
			
			FIFO* auto_key_buffer = emu->get_auto_key_buffer();
			auto_key_buffer->clear();
			
			int size = strlen(array.constData()), prev_kana = 0;
			const char *buf = (char *)(array.constData());
			
			for(int i = 0; i < size; i++) {
				int code = buf[i] & 0xff;
				if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
					i++;	// kanji ?
					continue;
				}
				// Effect [Enter] even Unix etc.(0x0a should not be ignored). 
				//else if(code == 0xa) { 
				//continue;	// cr-lf
				//}
				if((code = auto_key_table[code]) != 0) {
					int kana = code & 0x200;
					if(prev_kana != kana) {
						auto_key_buffer->write(0xf2);
					}
					prev_kana = kana;
					if(using_flags->is_use_auto_key_no_caps()) {
						if((code & 0x100) && !(code & (0x400 | 0x800))) {
							auto_key_buffer->write((code & 0xff) | 0x100);
						} else {
							auto_key_buffer->write(code & 0xff);
						}
					} else if(using_flags->is_use_auto_key_caps()) {
						if(code & (0x100 | 0x800)) {
							auto_key_buffer->write((code & 0xff) | 0x100);
						} else {
							auto_key_buffer->write(code & 0xff);
						}
					} else if(code & (0x100 | 0x400)) {
						auto_key_buffer->write((code & 0xff) | 0x100);
					} else {
						auto_key_buffer->write(code & 0xff);
					}
				}
			}
	   		
			if(prev_kana) {
				auto_key_buffer->write(0xf2);
			}
			p_emu->stop_auto_key();
			p_emu->start_auto_key();
		}
	}
#endif	
}

void EmuThreadClass::do_stop_auto_key(void)
{
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,
	//					  "AutoKey: stop\n");
#if defined(USE_AUTO_KEY)   
	p_emu->stop_auto_key();
#endif   
}

void EmuThreadClass::do_write_protect_disk(int drv, bool flag)
{
#ifdef USE_FLOPPY_DISK
	p_emu->is_floppy_disk_protected(drv, flag);
#endif	
}

void EmuThreadClass::do_close_disk(int drv)
{
#ifdef USE_FLOPPY_DISK
	p_emu->close_floppy_disk(drv);
	p_emu->d88_file[drv].bank_num = 0;
	p_emu->d88_file[drv].cur_bank = -1;
#endif	
}

void EmuThreadClass::do_open_disk(int drv, QString path, int bank)
{
#ifdef USE_FLOPPY_DISK
	QByteArray localPath = path.toLocal8Bit();
   
	//p_emu->d88_file[drv].bank_num = 0;
	//p_emu->d88_file[drv].cur_bank = -1;
	
	if(check_file_extension(localPath.constData(), ".d88") || check_file_extension(localPath.constData(), ".d77")) {
		
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 17, 1);
					memset(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], tmp, 127, 17);
					
					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					p_emu->d88_file[drv].bank_num++;
				}
				strcpy(emu->d88_file[drv].path, path.toUtf8().constData());
				if(bank >= p_emu->d88_file[drv].bank_num) bank = p_emu->d88_file[drv].bank_num - 1;
				if(bank < 0) bank = 0;
				p_emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->d88_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_floppy_disk(drv, localPath.constData(), bank);
	emit sig_update_recent_disk(drv);
#endif	
}
void EmuThreadClass::do_play_tape(int drv, QString name)
{
#if defined(USE_TAPE)
	p_emu->play_tape(drv, name.toLocal8Bit().constData());
#endif
}

void EmuThreadClass::do_rec_tape(int drv, QString name)
{
#if defined(USE_TAPE)
	p_emu->rec_tape(drv, name.toLocal8Bit().constData());
#endif
}

void EmuThreadClass::do_close_tape(int drv)
{
#if defined(USE_TAPE)
	p_emu->close_tape(drv);
#endif
}

void EmuThreadClass::do_cmt_push_play(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_play(drv);
#endif
}

void EmuThreadClass::do_cmt_push_stop(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_stop(drv);
#endif
}

void EmuThreadClass::do_cmt_push_fast_forward(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_fast_forward(drv);
#endif
}

void EmuThreadClass::do_cmt_push_fast_rewind(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_fast_rewind(drv);
#endif
}

void EmuThreadClass::do_cmt_push_apss_forward(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_apss_forward(drv);
#endif
}

void EmuThreadClass::do_cmt_push_apss_rewind(int drv)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_apss_rewind(drv);
#endif	
}


void EmuThreadClass::do_write_protect_quickdisk(int drv, bool flag)
{
#ifdef USE_QUICK_DISK
	//p_emu->write_protect_Qd(drv, flag);
#endif	
}

void EmuThreadClass::do_close_quickdisk(int drv)
{
#ifdef USE_QUICK_DISK
	p_emu->close_quick_disk(drv);
#endif	
}

void EmuThreadClass::do_open_quickdisk(int drv, QString path)
{
#ifdef USE_QUICK_DISK
	p_emu->open_quick_disk(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_open_cdrom(int drv, QString path)
{
#ifdef USE_COMPACT_DISC
	p_emu->open_compact_disc(drv, path.toLocal8Bit().constData());
#endif	
}
void EmuThreadClass::do_eject_cdrom(int drv)
{
#ifdef USE_COMPACT_DISC
	p_emu->close_compact_disc(drv);
#endif	
}

void EmuThreadClass::do_close_hard_disk(int drv)
{
#ifdef USE_HARD_DISK
	p_emu->close_hard_disk(drv);
#endif	
}

void EmuThreadClass::do_open_hard_disk(int drv, QString path)
{
#ifdef USE_HARD_DISK
	p_emu->open_hard_disk(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_close_cart(int drv)
{
#ifdef USE_CART
	p_emu->close_cart(drv);
#endif	
}

void EmuThreadClass::do_open_cart(int drv, QString path)
{
#ifdef USE_CART
	p_emu->open_cart(drv, path.toLocal8Bit().constData());
#endif	
}


void EmuThreadClass::do_close_laser_disc(int drv)
{
#ifdef USE_LASER_DISC
	p_emu->close_laser_disc(drv);
#endif
}

void EmuThreadClass::do_open_laser_disc(int drv, QString path)
{
#ifdef USE_LASER_DISC
	p_emu->open_laser_disc(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_load_binary(int drv, QString path)
{
#ifdef USE_BINARY_FILE
	p_emu->load_binary(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_save_binary(int drv, QString path)
{
#ifdef USE_BINARY_FILE
	p_emu->save_binary(drv, path.toLocal8Bit().constData());
#endif	
}


void EmuThreadClass::do_write_protect_bubble_casette(int drv, bool flag)
{
#ifdef USE_BUBBLE
	p_emu->is_bubble_casette_protected(drv, flag);
#endif	
}

void EmuThreadClass::do_close_bubble_casette(int drv)
{
#ifdef USE_BUBBLE
	p_emu->close_bubble_casette(drv);
	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;
#endif	
}

void EmuThreadClass::do_open_bubble_casette(int drv, QString path, int bank)
{
#ifdef USE_BUBBLE
	QByteArray localPath = path.toLocal8Bit();
   
	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;
	
	if(check_file_extension(localPath.constData(), ".b77")) {
		
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->b77_file[drv].bank_num < MAX_B77_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 16, 1);
					memset(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], tmp, 127, 17);
					
					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					p_emu->b77_file[drv].bank_num++;
				}
				strcpy(p_emu->b77_file[drv].path, path.toUtf8().constData());
				if(bank >= p_emu->b77_file[drv].bank_num) bank = p_emu->b77_file[drv].bank_num - 1;
				if(bank < 0) bank = 0;
				p_emu->b77_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->b77_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_bubble_casette(drv, localPath.constData(), bank);
	emit sig_update_recent_bubble(drv);
#endif	
}

// Debugger

void EmuThreadClass::do_close_debugger(void)
{
#if defined(USE_DEBUGGER)
	emit sig_quit_debugger();
#endif
}

bool EmuThreadClass::now_debugging() {
#if defined(USE_DEBUGGER)
	return p_emu->now_debugging;
#else
	return false;
#endif
}

