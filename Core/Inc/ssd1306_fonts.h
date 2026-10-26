#include <stdint.h>

#ifndef __SSD1306_FONTS_H__
#define __SSD1306_FONTS_H__

typedef struct {
	const uint8_t cFontWidth;    /*!< Font width in pixels */
	const uint8_t cFontHeight;   /*!< Font height in pixels */
	const uint16_t *sData; /*!< Pointer to data font data array */
} FontDef;


extern FontDef Font_6x8;
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif // __SSD1306_FONTS_H__
