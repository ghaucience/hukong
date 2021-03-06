/* utility definitions for RTD Serial Recovery (rtdsr)
 *
 * copyright (c) 2011 Pete B. <xtreamerdev@gmail.com>
 *
 * based on util.h from Realtek bootloader set_pll, not copyrighted
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#define NULL                    ((void *)0)
#define REG8( addr )            (*(volatile UINT8 *) (addr))
#define REG16( addr )           (*(volatile UINT16 *)(addr))
#define REG32( addr )           (*(volatile UINT32 *)(addr))
#define REG64( addr )           (*(volatile UINT64 *)(addr))

typedef unsigned char           UINT8;
typedef signed char             INT8;
typedef unsigned short          UINT16;
typedef signed short            INT16;
typedef unsigned int            UINT32;
typedef signed int              INT32;
typedef unsigned long long      UINT64;
typedef signed long long        INT64;
typedef UINT8                   bool;

/************************************************************************
 *  Public function
 ************************************************************************/
extern void _putchar(int c);
extern void _putchars(unsigned char *pchs, int nLen);
extern int  _getchar(int timeout);
extern void _flush_read();
#endif
