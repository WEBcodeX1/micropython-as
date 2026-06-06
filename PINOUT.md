# Pinout / Cabling

The following hardware / peripherals will be cabled like described in this document.

- I2C SSD1306 OLED Display
- 3 Color LED
- 1500 mAh 3.7V Battery

Detailed hardware specs at: https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/

## SSD1306 OLED

An I2C based SSD1306 128x64 pixel 2 color OLED display will be used.

### Cabling

```bash
        SSD1306 Display
----------------------------------
        GND  VCC  SCL  SDA
         x    x    x    x
         |    |    |    |
         |    |    |    |
         |    |    |    |
         x    x    x    x
        GND  3V3  D05  D04
----------------------------------
            ESP32-C3
```

## 3 Color LED

A standard IDUINO 3 color LED (including correct resistor) will be used.

### Cabling

```bash
        3 Color LED
----------------------------------
       P1R   P2G   P3B   GND
        x     x     x     x
        |     |     |     |
        |     |     |     |
        |     |     |     |
        x     x     x     x
       D00   D01   D02   GND
       GPIO2 GPIO3 GPIO4
----------------------------------
            ESP32-C3
```
