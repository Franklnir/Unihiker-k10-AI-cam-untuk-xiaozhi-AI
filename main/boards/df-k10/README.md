# DFRobot UNIHIKER K10

## Hardware support

This board profile targets the DFRobot UNIHIKER K10 based on ESP32-S3.

Supported onboard resources:

- 2.8 inch 240x320 ILI9341 screen
- GC2145 2MP camera through ESP Video
- ES8311 speaker output and ES7210 dual microphone input
- A/B buttons through the onboard IO expander
- 3 onboard RGB LEDs
- AHT20 temperature and humidity sensor at I2C address `0x38`
- LTR-303ALS ambient light sensor at I2C address `0x29`
- SC7A20H accelerometer at I2C address `0x19`
- TF/MicroSD card slot over SPI
- Gravity I2C and edge connector access through the shared K10 bus

## Button controls

- A short press: interrupt or wake chat
- B short press: interrupt or wake chat
- A long press: lower volume by 10
- B long press: raise volume by 10
- During startup, A or B enters Wi-Fi configuration mode

## Xiaozhi / MCP controls

The K10 build enables the WeChat-style two-way chat UI. User messages are shown as right-aligned green bubbles, assistant replies are shown as left-aligned bubbles, and photo previews appear in the same conversation area.

After the board is flashed and connected to Xiaozhi, these device tools are exposed:

- `self.k10.get_status`: read K10 capability/status summary
- `self.k10.get_buttons`: read current A/B button state
- `self.k10.show_notification`: show text on the K10 screen
- `self.k10.set_chat_message`: show a chat message on the screen
- `self.k10.clear_chat`: clear chat messages
- `self.k10.set_emotion`: set screen emotion
- `self.k10.get_volume`: read speaker volume
- `self.k10.set_volume`: set speaker volume from 0 to 100
- `self.k10.get_environment`: read temperature and humidity
- `self.k10.get_light`: read ambient light
- `self.k10.get_accelerometer`: read acceleration in milli-g
- `self.k10.get_sensors`: read all onboard sensors at once
- `self.k10.set_screen_brightness`: set LCD brightness from 0 to 100 when supported by the panel
- `self.k10.get_sd_status`: read TF/MicroSD mount status
- `self.k10.sd_list_files`: list files on TF/MicroSD
- `self.k10.sd_read_text`: read a text file from TF/MicroSD
- `self.k10.sd_write_text`: write a text file to TF/MicroSD
- `self.k10.sd_append_log`: append one log line to TF/MicroSD
- `self.k10.sd_save_camera_photo`: capture the camera and save a JPEG to TF/MicroSD
- `self.k10.sd_save_screen_snapshot`: save the current LCD screen as a JPEG to TF/MicroSD
- `self.k10.sd_play_audio`: reports current SD audio playback status; file streaming playback is not implemented yet
- `self.camera.set_camera_flipped`: flip or unflip the camera image
- `self.led_strip.get_brightness`: read RGB LED brightness
- `self.led_strip.set_brightness`: set RGB LED brightness
- `self.led_strip.set_single_color`: set one RGB LED
- `self.led_strip.set_all_color`: set all RGB LEDs
- `self.led_strip.blink`: blink RGB LEDs
- `self.led_strip.scroll`: run RGB LED scroll effect

## Build

Set target:

```bash
idf.py set-target esp32s3
```

Select board:

```text
Xiaozhi Assistant -> Board Type -> DFRobot UNIHIKER K10
```

Required K10 options:

```text
Component config -> ESP PSRAM -> SPI RAM config -> Mode (QUAD/OCT) -> Octal Mode PSRAM
Xiaozhi Assistant -> Language -> English
Xiaozhi Assistant -> Display Style -> Enable WeChat Message Style
Xiaozhi Assistant -> Camera Configuration -> Enable software camera buffer endianness swapping
Component config -> Espressif Camera Sensors Configurations -> Camera Sensor Configuration -> Select and Set Camera Sensor -> GC2145 -> Auto detect GC2145
Component config -> Espressif Camera Sensors Configurations -> Camera Sensor Configuration -> Select and Set Camera Sensor -> GC2145 -> Select default output format for DVP interface -> RGB565 800x600 20fps, DVP 8-bit, 20M input
```

Build:

```bash
idf.py build
```
