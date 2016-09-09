/*
 * Copyright (C) 2016  173210 <root.3.173210@live.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <vita2d.h>

struct console {
	vita2d_pgf *pgf;
	int rows;
	int columns;
	int characterHeight;
	int row;
	char buffer[];
};

#define CONSOLE_SCALE 1.0f
#define CONSOLE_Y 16

static struct console *consoleInit()
{
	static const char printable[]
		= " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNO"
		  "PQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	const int screenWidth = 960;
	const int screenHeight = 544;

	vita2d_init();
	vita2d_pgf * const pgf = vita2d_load_default_pgf();

	int characterWidth
		= vita2d_pgf_text_width(pgf, CONSOLE_SCALE, printable)
		  / (sizeof(printable) - 1);

	int characterHeight
		= vita2d_pgf_text_height(pgf, CONSOLE_SCALE, printable);

	const int columns = screenWidth / characterWidth;
	const int rows = (screenHeight - CONSOLE_Y) / characterHeight;
	const int size = (columns + 1) * rows;

	struct console * const context = malloc(sizeof(struct console) + size);
	if (context == NULL) {
		vita2d_fini();
		vita2d_free_pgf(pgf);
	} else {
		context->pgf = pgf;
		context->rows = rows;
		context->columns = columns;
		context->characterHeight = characterHeight;
		context->row = 0;

		for (int index = 0; index < size; index += columns + 1)
			context->buffer[index] = 0;
	}

	return context;
}

static int consolePrintf(struct console * restrict context,
			 const char * restrict format, ...)
{
	va_list list;

	va_start(list, format);
	const int result = vsnprintf(
		context->buffer + context->row * (context->columns + 1),
		context->columns + 1, format, list);
	va_end(list);

	context->row++;
	if (context->row > context->rows)
		context->row = 0;

	return result;
}

static noreturn void consoleDraw(const struct console * restrict context)
{
	while (true) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		for (int row = 0; row < context->rows; row++)
			vita2d_pgf_draw_text(context->pgf,
				0, CONSOLE_Y + row * context->characterHeight,
				RGBA8(0,255,0,255), CONSOLE_SCALE,
				context->buffer + row * (context->columns + 1));

		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
}

int main()
{
	struct console * const console = consoleInit();
	if (console == NULL)
		return EXIT_FAILURE;

	consolePrintf(console, "PKG Decryptor (C) 2016  173210 <root.3.173210@live.com>");
	consolePrintf(console, "This program comes with ABSOLUTELY NO WARRANTY.");
	consolePrintf(console, "This is free software, and you are welcome to redistribute it");
	consolePrintf(console, "under certain conditions; see `COPYING.TXT' for details.");

	SceUID file = sceIoOpen("ux0:encrypted.pkg", SCE_O_RDONLY, 0777);
	if (file < 0) {
		consolePrintf(console, "sceIoOpen failed: 0x%08X", file);
		goto end;
	}
	sceIoClose(file);
	consolePrintf(console, "Success");
end:
	consoleDraw(console);
}
