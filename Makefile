# Copyright (C) 2016  173210 <root.3.173210@live.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

LIBS := -lvita2d -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub \
	-lSceSysmodule_stub -lSceCtrl_stub -lScePgf_stub \
	-lSceCommonDialog_stub -lfreetype -lpng -ljpeg -lz -lm -lc

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -O2 -Wall -Wextra -Wl,-q -flto -pedantic -std=c11
LDFLAGS = $(CFLAGS) -fwhole-program

pkg.vpk: eboot.bin param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $@

eboot.bin: pkg.velf
	vita-make-fself -s $< $@

%.velf: %.elf
	vita-elf-create $< $@

pkg.elf: main.o
	$(LINK.o) $^ $(LIBS) $(OUTPUT_OPTION)

param.sfo:
	vita-mksfoex -s TITLE_ID=PKGDCRYPT "PKG Decryptor" $@

.PHONY: clean
clean:
	$(RM) eboot.bin main.o param.sfo pkg.elf pkg.velf pkg.vpk
