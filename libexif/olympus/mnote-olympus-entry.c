/* mnote-olympus-entry.c
 *
 * Copyright � 2002 Lutz M�ller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "mnote-olympus-entry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libexif/exif-format.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-entry.h>
#include <libexif/i18n.h>

#define CF(format,target,v,maxlen)                              \
{                                                               \
        if (format != target) {                                 \
                snprintf (v, maxlen,	                        \
                        _("Invalid format '%s', "               \
                        "expected '%s'."),                      \
                        exif_format_get_name (format),          \
                        exif_format_get_name (target));         \
                break;                                          \
        }                                                       \
}

#define CC(number,target,v,maxlen)                                      \
{                                                                       \
        if (number != target) {                                         \
                snprintf (v, maxlen,                                    \
                        _("Invalid number of components (%i, "          \
                        "expected %i)."), (int) number, (int) target);  \
                break;                                                  \
        }                                                               \
}

#undef  MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

static struct {
	ExifTag tag;
    ExifFormat fmt;
	struct {
		int index;
		const char *string;
	} elem[8];
} items[] = {
  { MNOTE_NIKON_TAG_LENSTYPE, EXIF_FORMAT_BYTE,
    { {0, N_("AF non D Lens")},
      {1, N_("Manual")},
      {2, N_("AF-D or AF-S Lens")},
      {6, N_("AF-D G Lens")},
      {10, N_("AF-D VR Lens")},
      {0, NULL}}},
  { MNOTE_NIKON_TAG_FLASHUSED, EXIF_FORMAT_BYTE,
    { {0, N_("Flash did not fire")},
      {4, N_("Flash unit unknown")},
      {7, N_("Flash is external")},
      {9, N_("Flash is on Camera")},
      {0, NULL}}},
  { MNOTE_NIKON1_TAG_QUALITY, EXIF_FORMAT_SHORT,
    { {1, N_("VGA Basic")},
      {2, N_("VGA Normal")},
      {3, N_("VGA Fine")},
      {4, N_("SXGA Basic")},
      {5, N_("SXGA Normal")},
      {6, N_("SXGA Fine")},
      {0, NULL}}},
  { MNOTE_NIKON1_TAG_COLORMODE, EXIF_FORMAT_SHORT,
    { {1, N_("Color")},
      {2, N_("Monochrome")},
      {0, NULL}}},
  { MNOTE_NIKON1_TAG_IMAGEADJUSTMENT, EXIF_FORMAT_SHORT,
    { {0, N_("Normal")},
      {1, N_("Bright+")},
      {2, N_("Bright-")},
      {3, N_("Contrast+")},
      {4, N_("Contrast-")},
      {0, NULL}}},
  { MNOTE_NIKON1_TAG_CCDSENSITIVITY, EXIF_FORMAT_SHORT,
    { {0, N_("ISO80")},
      {2, N_("ISO160")},
      {4, N_("ISO320")},
      {5, N_("ISO100")},
      {0, NULL}}},
  { MNOTE_NIKON1_TAG_WHITEBALANCE, EXIF_FORMAT_SHORT,
    { {0, N_("Auto")},
      {1, N_("Preset")},
      {2, N_("Daylight")},
      {3, N_("Incandescense")},
      {4, N_("Fluorescence")},
      {5, N_("Cloudy")},
      {6, N_("SpeedLight")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_QUALITY, EXIF_FORMAT_SHORT,
    { {1, N_("SQ")},
      {2, N_("HQ")},
      {3, N_("SHQ")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_MACRO, EXIF_FORMAT_SHORT,
    { {0, N_("No")},
      {1, N_("Yes")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_DIGIZOOM, EXIF_FORMAT_SHORT,
    { {0, N_("1x")},
      {2, N_("2x")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_FLASHMODE, EXIF_FORMAT_SHORT,
    { {0, N_("Auto")},
      {1, N_("Red-eye reduction")},
      {2, N_("Fill")},
      {3, N_("Off")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_SHARPNESS, EXIF_FORMAT_SHORT,
    { {0, N_("Normal")},
      {1, N_("Hard")},
      {2, N_("Soft")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_CONTRAST, EXIF_FORMAT_SHORT,
    { {0, N_("Hard")},
      {1, N_("Normal")},
      {2, N_("Soft")},
      {0, NULL}}},
  { MNOTE_OLYMPUS_TAG_MANFOCUS, EXIF_FORMAT_SHORT,
    { {0, N_("No")},
      {1, N_("Yes")},
      {0, NULL}}},
  { 0, }
};

char *
mnote_olympus_entry_get_value (MnoteOlympusEntry *entry, char *v, unsigned int maxlen)
{
	char         buf[32];
	ExifLong     vl;
	ExifShort    vs;
	ExifRational vr;
	int          i, j;

	if (!entry)
		return (NULL);

	memset (v, 0, maxlen);
	maxlen--;

	if ((!entry->data) && (entry->components > 0)) return (v);

	switch (entry->tag) {
	
	/* Nikon */
        case MNOTE_NIKON_TAG_FIRMWARE:
                CF (entry->format,  EXIF_FORMAT_UNDEFINED, v, maxlen);
                CC (entry->components, 4, v, maxlen);
                vl =  exif_get_long (entry->data, entry->order);
                snprintf (v, maxlen, "0x%04lx", vl);
                break;
        case MNOTE_NIKON_TAG_ISO:
                CF (entry->format, EXIF_FORMAT_SHORT, v, maxlen);
                CC (entry->components, 2, v, maxlen);
                //vs = exif_get_short (entry->data, entry->order);
                vs = exif_get_short (entry->data + 2, entry->order);
                snprintf (v, maxlen, "ISO %hd", vs);
                break;
        case MNOTE_NIKON_TAG_ISO2:
                CF (entry->format, EXIF_FORMAT_SHORT, v, maxlen);
                CC (entry->components, 2, v, maxlen);
                //vs = exif_get_short (entry->data, entry->order);
                vs = exif_get_short (entry->data + 2, entry->order);
                snprintf (v, maxlen, "ISO2 %hd", vs);
                break;
        case MNOTE_NIKON_TAG_QUALITY:
                CF (entry->format, EXIF_FORMAT_ASCII, v, maxlen);
                //CC (entry->components, 8, v, maxlen);
                //vl =  exif_get_long (entry->data  , entry->order);
                //printf("-> 0x%04x\n",entry->data);
                //printf("-> 0x%s<\n",entry->data - 0);
                memcpy(v, entry->data ,entry->components);
                //snprintf (v, maxlen, "%s<",  ( entry->data - 9  );
                break;
        case MNOTE_NIKON_TAG_COLORMODE:
        case MNOTE_NIKON_TAG_COLORMODE1:
        case MNOTE_NIKON_TAG_WHITEBALANCE:
        case MNOTE_NIKON_TAG_SHARPENING:
        case MNOTE_NIKON_TAG_FOCUSMODE:
        case MNOTE_NIKON_TAG_FLASHSETTING:
        case MNOTE_NIKON_TAG_ISOSELECTION:
        case MNOTE_NIKON_TAG_FLASHMODE:
        case MNOTE_NIKON_TAG_IMAGEADJUSTMENT:
        case MNOTE_NIKON_TAG_ADAPTER:
                CF (entry->format, EXIF_FORMAT_ASCII, v, maxlen);
                memcpy(v, entry->data, MIN (maxlen, entry->components));
                break;
        case MNOTE_NIKON_TAG_TOTALPICTURES:
                CF (entry->format, EXIF_FORMAT_LONG, v, maxlen);
                CC (entry->components, 1, v, maxlen);
                vl =  exif_get_long (entry->data, entry->order);
                snprintf (v, maxlen, "%lu",  vl );
                break;
        case MNOTE_NIKON_TAG_WHITEBALANCEFINE:
                CF (entry->format, EXIF_FORMAT_SSHORT, v, maxlen);
                CC (entry->components, 1, v, maxlen);
                vs = exif_get_short (entry->data, entry->order);
                snprintf (v, maxlen, "%hd", vs);
                break;
        case MNOTE_NIKON_TAG_WHITEBALANCERB:
                CF (entry->format, EXIF_FORMAT_RATIONAL, v, maxlen);
                CC (entry->components, 4, v, maxlen);
                //vr = exif_get_rational (entry->data, entry->order);
                //if (vr.numerator == 0) {
                //      strncpy (v, _("Unknown"), maxlen);
                //}
                //else {
                {
                        float r,b;
                        vr = exif_get_rational (entry->data, entry->order);
                        r = (1.0*vr.numerator) / vr.denominator;
                        vr = exif_get_rational (entry->data+8, entry->order);
                        b = (1.0*vr.numerator) / vr.denominator;
                        //printf("numerator %li, denominator %li\n", vr.numerator, vr.denominator);
                        snprintf (v, maxlen, "Red Correction %f, Blue Correction %f", r,b);
                }
                break;
	case MNOTE_NIKON_TAG_AFFOCUSPOSITION:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, v, maxlen);
		CC (entry->components, 4, v, maxlen);
		switch (  *( entry->data+1)  ) {
		  	case  0: strncpy (v, "AF Position: Center", maxlen); break;
		  	case  1: strncpy (v, "AF Position: Top", maxlen); break;
		  	case  2: strncpy (v, "AF Position: Bottom", maxlen); break;
		  	case  3: strncpy (v, "AF Position: Left", maxlen); break;
		  	case  4: strncpy (v, "AF Position: Right", maxlen); break;
		  	default: strncpy (v, "Unknown AF Position", maxlen);
		}     
		break;
	case MNOTE_NIKON_TAG_LENSTYPE:
	case MNOTE_NIKON_TAG_FLASHUSED:
	case MNOTE_NIKON1_TAG_QUALITY:
	case MNOTE_NIKON1_TAG_COLORMODE:
	case MNOTE_NIKON1_TAG_IMAGEADJUSTMENT:
	case MNOTE_NIKON1_TAG_CCDSENSITIVITY:
	case MNOTE_NIKON1_TAG_WHITEBALANCE:
	case MNOTE_OLYMPUS_TAG_QUALITY:
	case MNOTE_OLYMPUS_TAG_MACRO:
	case MNOTE_OLYMPUS_TAG_DIGIZOOM:
	case MNOTE_OLYMPUS_TAG_FLASHMODE:
	case MNOTE_OLYMPUS_TAG_SHARPNESS:
	case MNOTE_OLYMPUS_TAG_CONTRAST:
	case MNOTE_OLYMPUS_TAG_MANFOCUS:
		/* search the tag */
		for (i = 0; (items[i].tag && items[i].tag != entry->tag); i++);
		if (!items[i].tag) {
		  	strncpy (v, "Internal error", maxlen);
		  	break;
		}
		CF (entry->format, items[i].fmt, v, maxlen);
		CC (entry->components, 1, v, maxlen);
		switch (entry->format) {
			case EXIF_FORMAT_BYTE:
			case EXIF_FORMAT_UNDEFINED:
				vs = entry->data[0];
				break;
			case EXIF_FORMAT_SHORT:
				vs = exif_get_short(entry->data, entry->order);
				break;
		}
		/* find the value */
		for (j = 0; items[i].elem[j].string &&
			    (items[i].elem[j].index < vs); j++);
		if (items[i].elem[j].index != vs) {
			snprintf (v, maxlen, "Unknown value %hi", vs);
			break;
		}
		strncpy (v, items[i].elem[j].string, maxlen);
		break;

	case MNOTE_NIKON_TAG_LENS:
		CF (entry->format, EXIF_FORMAT_RATIONAL, v, maxlen);
		CC (entry->components, 4, v, maxlen);
		{
			float c,d;
			unsigned long a,b;
			vr = exif_get_rational (entry->data, entry->order);
			a = vr.numerator / vr.denominator;
			vr = exif_get_rational (entry->data+8, entry->order);
			b = vr.numerator / vr.denominator;
			vr = exif_get_rational (entry->data+16, entry->order);
			c = (1.0*vr.numerator) / vr.denominator;
			vr = exif_get_rational (entry->data+24, entry->order);
			d = (1.0*vr.numerator) / vr.denominator;
                        //printf("numerator %li, denominator %li\n", vr.numerator, vr.denominator);
			snprintf (v, maxlen, "%ld-%ldmm 1:%3.1f - %3.1f",a,b,c,d);
		}
		break;

	/* Olympus */
	case MNOTE_OLYMPUS_TAG_MODE:
		CF (entry->format, EXIF_FORMAT_LONG, v, maxlen);
		CC (entry->components, 3, v, maxlen);
		vl = exif_get_long (entry->data, entry->order);
		switch (vl) {
		case 0:
			strncpy (v, _("normal"), maxlen);
			break;
		case 1:
			strncpy (v, _("unknown"), maxlen);
			break;
		case 2:
			strncpy (v, _("fast"), maxlen);
			break;
		case 3:
			strncpy (v, _("panorama"), maxlen);
			break;
		default:
			snprintf (v, maxlen, _("%li"), vl);
		}
		vl = exif_get_long (entry->data + 4, entry->order);
		snprintf (buf, sizeof (buf), "/%li/", vl);
		strncat (v, buf, maxlen - strlen (v));
		vl = exif_get_long (entry->data + 4, entry->order);
		switch (vl) {
		case 1:
			strncat (v, _("left to right"), maxlen - strlen (v));
			break;
		case 2:
			strncat (v, _("right to left"), maxlen - strlen (v));
			break;
		case 3:
			strncat (v, _("bottom to top"), maxlen - strlen (v));
			break;
		case 4:
			strncat (v, _("top to bottom"), maxlen - strlen (v));
			break;
		default:
			snprintf (buf, sizeof (buf), _("%li"), vl);
			strncat (v, buf, maxlen - strlen (v));
		}
		break;
	case MNOTE_OLYMPUS_TAG_UNKNOWN_1:
		CF (entry->format, EXIF_FORMAT_SHORT, v, maxlen);
		CC (entry->components, 1, v, maxlen);
		strncpy (v, _("Unknown tag."), maxlen);
		break;
	case MNOTE_OLYMPUS_TAG_UNKNOWN_2:
		CF (entry->format, EXIF_FORMAT_RATIONAL, v, maxlen);
		CC (entry->components, 1, v, maxlen);
		break;
	case MNOTE_OLYMPUS_TAG_UNKNOWN_3:
		CF (entry->format, EXIF_FORMAT_SSHORT, v, maxlen);
		CC (entry->components, 1, v, maxlen);
		break;
	case MNOTE_OLYMPUS_TAG_VERSION:
		CF (entry->format, EXIF_FORMAT_ASCII, v, maxlen);
		CC (entry->components, 5, v, maxlen);
		strncpy (v, entry->data, MIN (maxlen, entry->size));
		break;
	case MNOTE_OLYMPUS_TAG_INFO:
		CF (entry->format, EXIF_FORMAT_ASCII, v, maxlen);
		CC (entry->components, 52, v, maxlen);
		break;
	case MNOTE_OLYMPUS_TAG_ID:
		CF (entry->format, EXIF_FORMAT_UNDEFINED, v, maxlen);
		CC (entry->components, 32, v, maxlen);
		strncpy (v, entry->data, MIN (maxlen, entry->size));
		break;
	case MNOTE_OLYMPUS_TAG_UNKNOWN_4:
		CF (entry->format, EXIF_FORMAT_LONG, v, maxlen);
		CC (entry->components, 30, v, maxlen);
		break;
	case MNOTE_OLYMPUS_TAG_FOCUSDIST:
		CF (entry->format, EXIF_FORMAT_RATIONAL, v, maxlen);
		CC (entry->components, 1, v, maxlen);
		vr = exif_get_rational (entry->data, entry->order);
		if (vr.numerator == 0) {
			strncpy (v, _("Unknown"), maxlen);
		}
		else {
			unsigned long tmp = vr.numerator / vr.denominator;
			/* printf("numerator %li, denominator %li\n", vr.numerator, vr.denominator); */
			snprintf (v, maxlen, "%li mm", tmp);
		}
		break;
	case MNOTE_OLYMPUS_TAG_WBALANCE:
		CF (entry->format, EXIF_FORMAT_SHORT, v, maxlen);
		CC (entry->components, 2, v, maxlen);
		vs = exif_get_short (entry->data, entry->order);
		switch (vs) {
		case 1:
			strncpy (v, _("Automatic"), maxlen);
			break;
		case 2:
			{
				ExifShort v2 = exif_get_short (entry->data + 2, entry->order);
				unsigned long colorTemp = 0;
				switch (v2) {
				case 2:
					colorTemp = 3000;
					break;
				case 3:
					colorTemp = 3700;
					break;
				case 4:
					colorTemp = 4000;
					break;
				case 5:
					colorTemp = 4500;
					break;
				case 6:
					colorTemp = 5500;
					break;
				case 7:
					colorTemp = 6500;
					break;
				case 9:
					colorTemp = 7500;
					break;
				}
				if (colorTemp) {
					snprintf (v, maxlen, "Manual: %liK", colorTemp);
				}
				else {
					strncpy (v, _("Manual: Unknown"), maxlen);
				}

			}
			break;
		case 3:
			strncpy (v, _("One-touch"), maxlen);
			break;
		default:
			strncpy (v, _("Unknown"), maxlen);
			break;
		}
		break;
	default:
		switch (entry->format) {
		case EXIF_FORMAT_ASCII:
		  strncpy (v, entry->data, MIN (maxlen, entry->components));
		  break;
		case EXIF_FORMAT_SHORT:
		  vs = exif_get_short (entry->data, entry->order);
		  snprintf (v, maxlen, "%hi", vs);
		  break;
		case EXIF_FORMAT_LONG:
		  vl = exif_get_long (entry->data, entry->order);
		  snprintf (v, maxlen, "%li", vl);
		  break;
		case EXIF_FORMAT_UNDEFINED:
		default:
		  snprintf (v, maxlen, "%li bytes unknown data",
			    entry->size);
		  break;
		}
		break;
	}

	return (v);
}
