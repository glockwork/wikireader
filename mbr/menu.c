/*
    e07 bootloader suite - boot menu
    Copyright (c) 2009 Christopher Hall <hsw@openmoko.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define APPLICATION_TITLE "boot menu"

#include "application.h"
#include "eeprom.h"

#define MAXIMUM_BLOCKS 8
#define HEADER_MAGIC  0x4f4d4153

struct {
	u32 magic;
	char name[32];
} header;


// main() must be first as the loader executes from the first program address
int main(void)
{
	int i = 0;
	int k = 0;
	u8 valid[MAXIMUM_BLOCKS] = {0};
	APPLICATION_INITIALISE();

	print("\n\nBoot Menu\n\n");

	// not zero since this program should be in block zero
	for (i = 1; i < MAXIMUM_BLOCKS; ++i) {
		eeprom_load((i << 13), (void *)&header, sizeof(header));

		if (HEADER_MAGIC == header.magic) {
			print_char(('A' - 1) + i);
			print(". ");
			for (k = 0; k < sizeof(header.name); ++k) {
				if ('\0' == header.name[k]) {
					break;
				}
				print_char(header.name[k]);
			}
			print_char('\n');
			valid[i] = 1;
		}
	}
	print("\nEnter selection: ");
	for (;;) {
		k = serial_input_char();
		if ('A' <= k && 'Z' >= k) {
			k += 'a' - 'A';
		}
		i = k - 'a' + 1;
		if (0 < i && MAXIMUM_BLOCKS > i) {
			if (valid[i]) {
				print_char(k);
				print_char('\n');
				break;
			}
		}
	}
	// next program
	APPLICATION_FINALISE(i);
}