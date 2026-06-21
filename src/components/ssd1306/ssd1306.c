#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1306.h"
#include "font8x8_basic.h"


void ssd1306_init(SSD1306_t* dev, int width, int height)
{
	i2c_init(dev, width, height);
	// Initialize internal buffer
	for (int i=0;i<dev->_pages;i++) {
		memset(dev->_page[i]._segs, 0, 128);
	}
}

int ssd1306_get_width(SSD1306_t* dev)
{
	return dev->_width;
}

int ssd1306_get_height(SSD1306_t* dev)
{
	return dev->_height;
}

int ssd1306_get_pages(SSD1306_t* dev)
{
	return dev->_pages;
}

void ssd1306_show_buffer(SSD1306_t* dev)
{
	for (int page=0; page<dev->_pages;page++) {
		i2c_display_image(dev, page, 0, dev->_page[page]._segs, dev->_width);
	}
}

void ssd1306_set_buffer(SSD1306_t* dev, const uint8_t* buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&dev->_page[page]._segs, &buffer[index], 128);
		index = index + 128;
	}
}

void ssd1306_get_buffer(SSD1306_t* dev, uint8_t* buffer)
{
	int index = 0;
	for (int page=0; page<dev->_pages;page++) {
		memcpy(&buffer[index], &dev->_page[page]._segs, 128);
		index = index + 128;
	}
}

void ssd1306_set_page(SSD1306_t* dev, int page, const uint8_t* buffer)
{
	memcpy(&dev->_page[page]._segs, buffer, 128);
}

void ssd1306_get_page(SSD1306_t* dev, int page, uint8_t* buffer)
{
	memcpy(buffer, &dev->_page[page]._segs, 128);
}

void ssd1306_display_image(SSD1306_t* dev, int page, int seg, const uint8_t* images, int width)
{
	i2c_display_image(dev, page, seg, images, width);
	// Set to internal buffer
	memcpy(&dev->_page[page]._segs[seg], images, width);
}

void ssd1306_display_text(SSD1306_t* dev, int page, const char* text, int text_len, bool invert)
{
	if (page >= dev->_pages) return;

	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;

	int seg = 0;
	uint8_t image[8];

	for (int i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) ssd1306_invert(image, 8);
		ssd1306_display_image(dev, page, seg, image, 8);
		seg = seg + 8;
	}
}

void ssd1306_clear_screen(SSD1306_t* dev, bool invert)
{
	char space[16];
	memset(space, 0x1f, sizeof(space));
	for (int page = 0; page < dev->_pages; page++) {
		ssd1306_display_text(dev, page, space, sizeof(space), invert);
	}
}

void ssd1306_clear_line(SSD1306_t* dev, int page, bool invert)
{
	char space[16];
	memset(space, 0x1f, sizeof(space));
	ssd1306_display_text(dev, page, space, sizeof(space), invert);
}

void ssd1306_contrast(SSD1306_t* dev, int contrast)
{
	i2c_contrast(dev, contrast);
}

void ssd1306_bitmaps(SSD1306_t* dev, int xpos, int ypos, const uint8_t* bitmap, int width, int height, bool invert)
{
	_ssd1306_bitmaps(dev, xpos, ypos, bitmap, width, height, invert);
	
	// Calculate the range of pages and segments to update
	int start_page = ypos / 8;
	int end_page = (ypos + height - 1) / 8;
	int start_seg = xpos;
	int end_seg = xpos + width - 1;

	// Update only the modified pages and segments
	for (int page = start_page; page <= end_page; page++) {
		int seg_start = (page == start_page) ? start_seg : 0;
		int seg_end = (page == end_page) ? end_seg : 127;
		int seg_width = seg_end - seg_start + 1;
		ssd1306_display_image(dev, page, seg_start, &dev->_page[page]._segs[seg_start], seg_width);
	}
}

// Set pixel to internal buffer. Not show it.
void _ssd1306_pixel(SSD1306_t* dev, int xpos, int ypos, bool invert)
{
	uint8_t _page = (ypos / 8);
	uint8_t _bits = (ypos % 8);
	uint8_t _seg = xpos;
	uint8_t wk0 = dev->_page[_page]._segs[_seg];
	uint8_t wk1 = 1 << _bits;
	if (invert) {
		wk0 = wk0 & ~wk1;
	} else {
		wk0 = wk0 | wk1;
	}
	dev->_page[_page]._segs[_seg] = wk0;
}

// Set line to internal buffer. Not show it.
void _ssd1306_line(SSD1306_t* dev, int x1, int y1, int x2, int y2,  bool invert)
{
	int i;
	int dx,dy;
	int sx,sy;
	int E;

	/* distance between two points */
	dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
	dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = ( x2 > x1 ) ? 1 : -1;
	sy = ( y2 > y1 ) ? 1 : -1;

	/* inclination < 1 */
	if ( dx > dy ) {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) {
			_ssd1306_pixel(dev, x1, y1, invert);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) {
			y1 += sy;
			E -= 2 * dx;
		}
	}

	/* inclination >= 1 */
	} else {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) {
			_ssd1306_pixel(dev, x1, y1, invert);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}

void _ssd1306_cursor(SSD1306_t* dev, int x0, int y0, int r, bool invert)
{
	_ssd1306_line(dev, x0-r, y0, x0+r, y0, invert);
	_ssd1306_line(dev, x0, y0-r, x0, y0+r, invert);
}

void ssd1306_invert(uint8_t* buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

uint8_t ssd1306_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits)
{
	uint8_t smask = 0x01 << srcBits;
	uint8_t dmask = 0x01 << dstBits;
	uint8_t _src = src & smask;
	uint8_t _dst;
	if (_src != 0) {
		_dst = dst | dmask; // set bit
	} else {
		_dst = dst & ~(dmask); // clear bit
	}
	return _dst;
}
