#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#define INCL_RXFUNC
#include <rexxsaa.h>

#define RX_OK  0;
#define RX_ERR 1;

#define trace_args(msg, ...) printf("%s[%s:%ld]: "msg, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define trace(msg, ...) printf("%s[%s:%ld]: "msg, __FUNCTION__, __FILE__, __LINE__)

enum error_type {
    OK = 0,
    BAD_STR,
    BAD_LEN,
    BAD_HEX,
};

RexxFunctionHandler rexxInitWindow;
RexxFunctionHandler rexxCloseWindow;
RexxFunctionHandler rexxWindowShouldClose;

RexxFunctionHandler rexxBeginDrawing;
RexxFunctionHandler rexxEndDrawing;
RexxFunctionHandler rexxClearBackground;
RexxFunctionHandler rexxDrawText;
RexxFunctionHandler rexxDrawRectangle;

RexxFunctionHandler rexxGetFrameTime;

RexxFunctionHandler rexxIsKeyDown;

// converts rexx string containing number to long
// NOTICE: does not fail if the string doesn't actually have a number, just returns 0
enum error_type rxstr_to_long(RXSTRING rxstr, long* value) {
	// abort if string is invalid
	if (!RXVALIDSTRING(rxstr)) {
		return BAD_STR;
	}

	*value = strtol(RXSTRPTR(rxstr), NULL, 10);
	return OK;
}

// converts rexx string (with no NUL chars) to null terminated char* in cstr_ret, must be freed after use
enum error_type rxstr_to_cstr(RXSTRING rxstr, char** cstr_ret) {
	// abort if its not a valid string
	if (!RXVALIDSTRING(rxstr)) {
		return BAD_STR;
	}

	// allocate buffer and copy over data from rexx string
	char* cstr = malloc(sizeof(char) * RXSTRLEN(rxstr) + 1);
	memcpy(cstr, RXSTRPTR(rxstr), RXSTRLEN(rxstr));
	cstr[RXSTRLEN(rxstr)] = '\0';

	*cstr_ret = cstr;
	return OK;
}

// converts rexx string containing a hexcode (format #RRGGBBAA) into a raylib Color pointed to by color
enum error_type rxstr_to_color(RXSTRING rxstr, Color* color) {
	// abort if rexx string is not valid
	if (!RXVALIDSTRING(rxstr)) {
		return BAD_STR;
	}

	// abort if the length of the string is wrong (9 = 8 digits from hex + # at the start)
	if (RXSTRLEN(rxstr) != 9) {
		return BAD_LEN;
	}

	// convert hex code to long
	char* endptr = NULL;
	long hex = strtol(RXSTRPTR(rxstr) + 1, &endptr, 16);

	// abort if not all digits from hex code could be read
	if (*endptr) {
		return BAD_HEX;
	}

	color->r = (hex >> 8*3) & 0xFF;
	color->g = (hex >> 8*2) & 0xFF;
	color->b = (hex >> 8*1) & 0xFF;
	color->a = (hex >> 8*0) & 0xFF;

	return OK;
}

// expects width and height (numbers) and window title (string without NUL chars) arguments, creates raylib window, returns nothing
APIRET APIENTRY rexxInitWindow(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	// abort if less than three arguments were supplied
	if (argc < 3) {
		trace_args("ERROR: expected 3 arguments, got %ld\n", argc);
		return RX_ERR;
	}

	// convert size arguments to longs
	long width, height;

	if (rxstr_to_long(argv[0], &width)) {
    		trace("ERROR: width is not a valid string (must contain non-empty characters)\n");
    		return RX_ERR;
	}

	if (rxstr_to_long(argv[1], &height)) {
    		trace("ERROR: height is not a valid string (must contain non-empty characters)\n");
    		return RX_ERR;
	}

	// convert title argument to a proper c string
	char* title = NULL;
	if (rxstr_to_cstr(argv[2], &title)) {
		trace("ERROR: window title is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	// initializing window
	InitWindow(width, height, title);

	// freeing
	free(title);

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects no arguments, closes raylib window, returns nothing
APIRET APIENTRY rexxCloseWindow(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	CloseWindow();

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects no arguments, returns wether or not the raylib window should close (1 or 0)
APIRET APIENTRY rexxWindowShouldClose(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	sprintf(RXSTRPTR(*returnstr), "%d", WindowShouldClose());
	returnstr->strlength = 1;

	return RX_OK;
}

// expects no arguments, begins drawing on the raylib window, returns nothing
APIRET APIENTRY rexxBeginDrawing(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	BeginDrawing();

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects no arguments, ends drawing on the raylib window, returns nothing
APIRET APIENTRY rexxEndDrawing(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	EndDrawing();

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects one argument containing a color hex (format #RRGGBBAA), clears the window to the provided color, returns nothing
APIRET APIENTRY rexxClearBackground(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	// abort if no arguments were supplied
	if (argc < 1) {
		trace_args("ERROR: expected 1 argument, got %ld\n", argc);
	}

	// convert color hex to raylib Color
	Color color;
	enum error_type err;
	if ((err = rxstr_to_color(argv[0], &color))) {
		switch (err) {
			case BAD_STR:
				trace("ERROR: color hex string is invalid (must contain non-empty characters)\n");
				break;
			case BAD_LEN:
				trace("ERROR: color hex string is the wrong length, expected format: #RRGGBBAA\n");
				break;
			case BAD_HEX:
				trace("ERROR: failed to read color hex, expected format: #RRGGBBAA\n");
				break;
			default: break;
		}

		return RX_ERR;
	}

	// clear background
	ClearBackground(color);

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects the following arguments:
// - the text to be drawn (string with no NUL chars)
// - the x and y position to draw the text at (numbers)
// - the font size (number)
// - the font color (color hex with format #RRGGBBAA)
// draws the requested text to the window in the provided position with the specified font size and color, returns nothing
APIRET APIENTRY rexxDrawText(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	// abort if less than five arguments were supplied
	if (argc < 5) {
		trace_args("ERROR: expected 5 arguments, got %ld\n", argc);
		return RX_ERR;
	}

	// convert text to c string
	char* text = NULL;
	if (rxstr_to_cstr(argv[0], &text)) {
		trace("ERROR: supplied text to draw is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	// convert position arguments and font size to longs
	long x, y, font_size;

	if (rxstr_to_long(argv[1], &x)) {
		trace("ERROR: x position is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	if (rxstr_to_long(argv[2], &y)) {
		trace("ERROR: y position is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	if (rxstr_to_long(argv[3], &font_size)) {
		trace("ERROR: font size is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	// convert color hex into raylib Color
	Color color;
	enum error_type err;
	if ((err = rxstr_to_color(argv[4], &color))) {
		switch (err) {
			case BAD_STR:
				trace("ERROR: color hex string is invalid (must contain non-empty characters)\n");
				break;
			case BAD_LEN:
				trace("ERROR: color hex string is the wrong length, expected format: #RRGGBBAA\n");
				break;
			case BAD_HEX:
				trace("ERROR: failed to read color hex, expected format: #RRGGBBAA\n");
				break;
			default: break;
		}

		return RX_ERR;
	}

	// drawing
	DrawText(text, x, y, font_size, color);

	// freeing
	free(text);

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects the following arguments:
// - the x and y position to draw the rectangle (numbers)
// - the width and height of the rectangle (numbers)
// - the color to fill the rectangle (color hex with the format #RRGGBBAA)
// draws a filled rectangle at the specified position with the provided size and color, returns nothing
APIRET APIENTRY rexxDrawRectangle(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	// abort if less than 5 arguments were supplied
	if (argc < 5) {
		trace_args("ERROR: expected 5 arguments, got %d\n", argc);
		return RX_ERR;
	}

	// convert x, y, width and height arguments to longs
	long x, y, w, h;

	if (rxstr_to_long(argv[0], &x)) {
		trace("ERROR: x position is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	if (rxstr_to_long(argv[1], &y)) {
		trace("ERROR: y position is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	if (rxstr_to_long(argv[2], &w)) {
		trace("ERROR: width is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	if (rxstr_to_long(argv[3], &h)) {
		trace("ERROR: height is not a valid string (must contain non-empty characters)\n");
		return RX_ERR;
	}

	// convert color hex to raylib Color
	Color color;
	enum error_type err;
	if ((err = rxstr_to_color(argv[4], &color))) {
		switch (err) {
			case BAD_STR:
				trace("ERROR: color hex string is invalid (must contain non-empty characters)\n");
				break;
			case BAD_LEN:
				trace("ERROR: color hex string is the wrong length, expected format: #RRGGBBAA\n");
				break;
			case BAD_HEX:
				trace("ERROR: failed to read color hex, expected format: #RRGGBBAA\n");
				break;
			default: break;
		}

		return RX_ERR;
	}

	// draw rectangle

	DrawRectangle(x, y, w, h, color);

	*returnstr = (RXSTRING){.strptr = NULL, .strlength = 0};
	return RX_OK;
}

// expects no arguments, returns the elapsed time since the last frame in seconds
// WARNING: this is probably extremely unsafe
APIRET APIENTRY rexxGetFrameTime(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
    sprintf(RXSTRPTR(*returnstr), "%f", GetFrameTime());
    returnstr->strlength = strlen(RXSTRPTR(*returnstr));

    return RX_OK;
}

// expects a keycode as an argument (number, possible values in raylib.h), returns wether or not that key is being pressed
APIRET APIENTRY rexxIsKeyDown(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queuename, PRXSTRING returnstr) {
	// abort if no arguments were supplied
	if (argc < 1) {
		trace_args("ERROR: expected 1 argument, got %d\n", argc);
		return RX_ERR;
	}

	// convert keycode argument to long
	long keycode;
	if (rxstr_to_long(argv[0], &keycode)) {
		trace("ERROR: keycode is not a valid string (must contain non-empty characters)\n");
	}

	// return to returnstr
	sprintf(RXSTRPTR(*returnstr), "%d", IsKeyDown(keycode));
	returnstr->strlength = 1;
	
	return RX_OK;
}

