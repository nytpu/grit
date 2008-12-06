//
//! \file cldib_adjust.cpp
//!   color adjustment
//! \date 20050823 - 20080304
//! \author cearn
//
// === NOTES === 

#include <math.h>

#include "cldib_core.h"
#include "cldib_tools.h"

// === CONSTANTS ======================================================
// === CLASSES ========================================================
// === GLOBALS ========================================================
// === PROTOTYPES =====================================================


// --------------------------------------------------------------------
// FUNCTIONS
// --------------------------------------------------------------------


//! Color adjustment by LUT (all bpp; IP).
/*!	\param dib Work bitmap.
*	\param lut Adjustment LUT. Assumed to be 256 entries long.
*	\param cce Color channel to adjust
*/
bool dib_adjust(CLDIB *dib, BYTE lut[], enum eClrChannel cce)
{
	if(dib == NULL || lut == NULL)
		return false;

	int ii;
	int dibB= dib_get_bpp(dib);
	switch(dibB)
	{
	case 1:		case 4:		case 8:
		data_adjust((BYTE*)dib_get_pal(dib), lut, cce, 
			dib_get_nclrs(dib), RGB_SIZE);
		break;
	case 24:	case 32:
		{
			int ofs= dibB>>8;
			int dibW= dib_get_width(dib);
			int dibH= dib_get_height(dib);
			int dibP= dib_get_pitch(dib);
			BYTE *dibL= dib_get_img(dib);
			for(ii=0; ii<dibH; ii++)
			{
				data_adjust(dibL, lut, cce, dibW, ofs);
				dibL += dibP;
			}
		}
	default:
		return false;
	}
	return true;
}


//! Data adjustment by LUT; internal for dib_adjust
/*!	\param data Work buffer.
*	\param lut Adjustment LUT. Assumed to be 256 entries long.
*	\param cce Color channel to adjust
*	\param nn Number of data entries
*	\param ofs Offset between entries
*/
bool data_adjust(BYTE *data, BYTE lut[], enum eClrChannel cce, int nn, int ofs)
{
	int ii, jj;
	BYTE *line;
	nn *= ofs;
	for(ii=0; ii<4; ii++)	// cycle through BGRA
	{
		if(~cce & (1<<ii))
			continue;
		line= &data[ii];
		for(jj=0; jj<nn; jj += ofs)
			line[jj]= lut[line[jj]];
	}
	return true;
}


//!	Inverts the bitmap (all bpp; IP).
/*!	\param dib Work bitmap.
*/
bool dib_invert(CLDIB *dib)
{
	if(dib == NULL)
		return false;

	int ii, nn;
	int dibB= dib_get_bpp(dib);
	// I need the mask to keep the reserved bits of RGBQUAD.
	DWORD mask= 0xFFFFFFFF, *buf4;

	if(dibB <= 8)	// invert palette
	{
		mask >>= 8;
		buf4= (DWORD*)dib_get_pal(dib);
		nn= dib_get_nclrs(dib);
	}
	else
	{
		if(dibB==32)
			mask >>= 8;
		buf4= (DWORD*)dib_get_img(dib);
		nn= dib_get_pitch(dib)*dib_get_height(dib)/4;
	}

	for(ii=0; ii<nn; ii++)
		buf4[ii] ^= mask;
	return true;
}


// --- LUT INITIALISERS -----------------------------------------------


//! Brightness LUT (perc=[-100,+100]). b= perc/100 <br> f(x)= x + b
bool dib_lut_brightness(BYTE lut[], double perc)
{	return dib_lut_linear(lut, 1.0, perc/100.0);				}


//! Contrast LUT (perc=[-100,+100]). c= perc/100 <br> f(x)= (c+1)x - 