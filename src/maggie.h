/**
 * maggie.h
 *
 * Maggie3D shared library
 *
 * @author Fabrice Labrador <fabrice.labrador@gmail.com>
 * @version 1.2 March 2024 (updated: 10/03/2024)
 */

#ifndef _MAGGIE_H_
#define _MAGGIE_H_

// 68080 CPU bit
#ifndef AFB_68080
#define AFB_68080             10
#endif
#ifndef AFF_68080
#define AFF_68080             (1<<AFB_68080)
#endif

// Vampire card
#define M3D_VCARD             0xdff3fc
#define M3D_V600              1
#define M3D_V500              2
#define M3D_V4500             3
#define M3D_V41200            4
#define M3D_V4SA              5
#define M3D_V1200             6
#define M3D_V4600             7

#endif
