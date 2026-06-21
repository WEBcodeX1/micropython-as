#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "ssd1306.h"

#define I2C_NUM I2C_NUM_0

#define I2C_MASTER_FREQ_HZ 400000
#define I2C_TICKS_TO_WAIT 100


void i2c_master_init(SSD1306_t* dev, int16_t sda, int16_t scl, int16_t reset)
{
	i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.i2c_port = I2C_NUM,
		.scl_io_num = scl,
		.sda_io_num = sda,
		.flags.enable_internal_pullup = true,
	};

	i2c_master_bus_handle_t i2c_bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = I2C_ADDRESS,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};

	i2c_master_dev_handle_t i2c_dev_handle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

	if (reset >= 0) {
		gpio_reset_pin(reset);
		gpio_set_direction(reset, GPIO_MODE_OUTPUT);
		gpio_set_level(reset, 0);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		gpio_set_level(reset, 1);
	}

	dev->_address = I2C_ADDRESS;
	dev->_i2c_num = I2C_NUM;
	dev->_i2c_bus_handle = i2c_bus_handle;
	dev->_i2c_dev_handle = i2c_dev_handle;
}

void i2c_device_add(SSD1306_t* dev, i2c_port_t i2c_num, int16_t reset, uint16_t i2c_address)
{
	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = i2c_address,
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
	};

	i2c_master_dev_handle_t i2c_dev_handle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(dev->_i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

	if (reset >= 0) {
		gpio_reset_pin(reset);
		gpio_set_direction(reset, GPIO_MODE_OUTPUT);
		gpio_set_level(reset, 0);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		gpio_set_level(reset, 1);
	}

	dev->_address = i2c_address;
	dev->_i2c_num = i2c_num;
	dev->_i2c_dev_handle = i2c_dev_handle;
}

void i2c_init(SSD1306_t* dev, int width, int height)
{
	dev->_width = width;
	dev->_height = height;
	dev->_pages = 8;

	if (dev->_height == 32) dev->_pages = 4;

	uint8_t out_buf[27];
	int out_index = 0;

	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
	out_buf[out_index++] = OLED_CMD_DISPLAY_OFF;				// AE
	out_buf[out_index++] = OLED_CMD_SET_MUX_RATIO;				// A8

	if (dev->_height == 64) out_buf[out_index++] = 0x3F;
	if (dev->_height == 32) out_buf[out_index++] = 0x1F;

	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_OFFSET;			// D3
	out_buf[out_index++] = 0x00;
	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_START_LINE;		// 40

	out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1;		// A1

	out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_MODE;			// C8
	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_CLK_DIV;		// D5
	out_buf[out_index++] = 0x80;
	out_buf[out_index++] = OLED_CMD_SET_COM_PIN_MAP;			// DA
	if (dev->_height == 64) out_buf[out_index++] = 0x12;
	if (dev->_height == 32) out_buf[out_index++] = 0x02;
	out_buf[out_index++] = OLED_CMD_SET_CONTRAST;				// 81
	out_buf[out_index++] = 0xFF;
	out_buf[out_index++] = OLED_CMD_DISPLAY_RAM;				// A4
	out_buf[out_index++] = OLED_CMD_SET_VCOMH_DESELCT;			// DB
	out_buf[out_index++] = 0x40;
	out_buf[out_index++] = OLED_CMD_SET_MEMORY_ADDR_MODE;		// 20
	out_buf[out_index++] = OLED_CMD_SET_PAGE_ADDR_MODE;			// 02
	out_buf[out_index++] = 0x00;
	out_buf[out_index++] = 0x10;
	out_buf[out_index++] = OLED_CMD_SET_CHARGE_PUMP;			// 8D
	out_buf[out_index++] = 0x14;
	out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL;			// 2E
	out_buf[out_index++] = OLED_CMD_DISPLAY_NORMAL;				// A6
	out_buf[out_index++] = OLED_CMD_DISPLAY_ON;					// AF

	esp_err_t res;
	i2c_master_transmit(dev->_i2c_dev_handle, out_buf, out_index, I2C_TICKS_TO_WAIT);
}

void i2c_display_image(SSD1306_t* dev, int page, int seg, const uint8_t * images, int width)
{
	if (page >= dev->_pages) return;
	if (seg >= dev->_width) return;

	//int _seg = seg + CONFIG_OFFSETX;
	int _seg = seg + 0;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;

	uint8_t *out_buf;
	out_buf = malloc(width < 4 ? 4 : width + 1);
	if (out_buf == NULL) {
		return;
	}

	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
	// Set Lower Column Start Address for Page Addressing Mode
	out_buf[out_index++] = (0x00 + columLow);
	// Set Higher Column Start Address for Page Addressing Mode
	out_buf[out_index++] = (0x10 + columHigh);
	// Set Page Start Address for Page Addressing Mode
	out_buf[out_index++] = 0xB0 | _page;

	esp_err_t res;
	i2c_master_transmit(dev->_i2c_dev_handle, out_buf, out_index, I2C_TICKS_TO_WAIT);

	out_buf[0] = OLED_CONTROL_BYTE_DATA_STREAM;
	memcpy(&out_buf[1], images, width);

	i2c_master_transmit(dev->_i2c_dev_handle, out_buf, width + 1, I2C_TICKS_TO_WAIT);
	free(out_buf);
}

void i2c_contrast(SSD1306_t* dev, int contrast)
{
	uint8_t _contrast = contrast;

	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	uint8_t out_buf[3];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM; // 00
	out_buf[out_index++] = OLED_CMD_SET_CONTRAST; // 81
	out_buf[out_index++] = _contrast;

	i2c_master_transmit(dev->_i2c_dev_handle, out_buf, 3, I2C_TICKS_TO_WAIT);
}
