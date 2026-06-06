#include "wifi_board.h"
#include "k10_audio_codec.h"
#include "display/lcd_display.h"
#include "esp_lcd_ili9341.h"
#include "led_control.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "esp_video.h"
#include "mcp_server.h"
#include "settings.h"
#include "backlight.h"
#include "ssid_manager.h"

#include "led/circular_strip.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_timer.h>
#include <esp_vfs_fat.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <driver/sdspi_host.h>
#include <sdmmc_cmd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cmath>
#include <cstdio>

#include "esp_io_expander_tca95xx_16bit.h"

#define TAG "DF-K10"

class Ili9341RegisterBacklight : public Backlight {
public:
    explicit Ili9341RegisterBacklight(esp_lcd_panel_io_handle_t panel_io) : panel_io_(panel_io) {
        uint8_t ctrl = 0x2c;
        esp_lcd_panel_io_tx_param(panel_io_, 0x53, &ctrl, 1);
    }

private:
    esp_lcd_panel_io_handle_t panel_io_;

    void SetBrightnessImpl(uint8_t brightness) override {
        uint8_t value = static_cast<uint8_t>(brightness * 255 / 100);
        esp_lcd_panel_io_tx_param(panel_io_, 0x51, &value, 1);
    }
};

class Df_K10Board : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_dev_handle_t aht20_ = nullptr;
    i2c_master_dev_handle_t ltr303_ = nullptr;
    i2c_master_dev_handle_t sc7a20_ = nullptr;
    esp_io_expander_handle_t io_expander;
    LcdDisplay *display_;
    Ili9341RegisterBacklight* backlight_ = nullptr;
    button_handle_t btn_a;
    button_handle_t btn_b;
    EspVideo* camera_;
    sdmmc_card_t* sd_card_ = nullptr;
    bool sd_mounted_ = false;

    button_driver_t* btn_a_driver_ = nullptr;
    button_driver_t* btn_b_driver_ = nullptr;

    CircularStrip* led_strip_;
    bool camera_flipped_ = false;
    bool wifi_reconfigure_pending_ = false;

    static Df_K10Board* instance_;

    static constexpr uint8_t AHT20_ADDR = 0x38;
    static constexpr uint8_t LTR303_ADDR = 0x29;
    static constexpr uint8_t SC7A20_ADDR = 0x19;
    static constexpr const char* SD_MOUNT_POINT = "/sdcard";

    bool AddI2cDevice(uint8_t addr, i2c_master_dev_handle_t* dev, uint32_t speed_hz = 400000) {
        i2c_device_config_t cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = speed_hz,
            .scl_wait_us = 0,
            .flags = {
                .disable_ack_check = 0,
            },
        };
        auto ret = i2c_master_bus_add_device(i2c_bus_, &cfg, dev);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "I2C device 0x%02x unavailable: %s", addr, esp_err_to_name(ret));
            *dev = nullptr;
            return false;
        }
        return true;
    }

    esp_err_t I2cWrite(i2c_master_dev_handle_t dev, const uint8_t* data, size_t len) {
        if (dev == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }
        return i2c_master_transmit(dev, data, len, 100);
    }

    esp_err_t I2cReadReg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* data, size_t len) {
        if (dev == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }
        return i2c_master_transmit_receive(dev, &reg, 1, data, len, 100);
    }

    esp_err_t I2cWriteReg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t value) {
        uint8_t buffer[2] = {reg, value};
        return I2cWrite(dev, buffer, sizeof(buffer));
    }

    std::string MakeSdPath(const std::string& requested_path) {
        std::string path = requested_path.empty() ? "/" : requested_path;
        if (path.find("..") != std::string::npos) {
            return "";
        }
        if (path.rfind(SD_MOUNT_POINT, 0) == 0) {
            return path;
        }
        if (path[0] != '/') {
            path = "/" + path;
        }
        return std::string(SD_MOUNT_POINT) + path;
    }

    bool EnsureSdMounted() {
        return sd_mounted_;
    }

    bool WriteBinaryFile(const std::string& path, const std::string& data) {
        FILE* file = fopen(path.c_str(), "wb");
        if (file == nullptr) {
            ESP_LOGE(TAG, "Failed to open %s", path.c_str());
            return false;
        }
        size_t written = fwrite(data.data(), 1, data.size(), file);
        fclose(file);
        return written == data.size();
    }

    bool AppendTextFile(const std::string& path, const std::string& text) {
        FILE* file = fopen(path.c_str(), "a");
        if (file == nullptr) {
            ESP_LOGE(TAG, "Failed to open %s", path.c_str());
            return false;
        }
        size_t written = fwrite(text.data(), 1, text.size(), file);
        fwrite("\n", 1, 1, file);
        fclose(file);
        return written == text.size();
    }

    std::string ReadTextFile(const std::string& path, size_t max_bytes) {
        FILE* file = fopen(path.c_str(), "rb");
        if (file == nullptr) {
            return "";
        }
        std::string data;
        data.resize(max_bytes);
        size_t read = fread(data.data(), 1, max_bytes, file);
        fclose(file);
        data.resize(read);
        return data;
    }

    cJSON* GetSdStatusJson() {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddBoolToObject(root, "mounted", sd_mounted_);
        cJSON_AddStringToObject(root, "mount_point", SD_MOUNT_POINT);
        if (sd_mounted_ && sd_card_ != nullptr) {
            cJSON_AddNumberToObject(root, "sector_size", sd_card_->csd.sector_size);
            cJSON_AddNumberToObject(root, "capacity_mb",
                static_cast<double>(sd_card_->csd.capacity) * sd_card_->csd.sector_size / (1024.0 * 1024.0));
            cJSON_AddNumberToObject(root, "spi_mosi_gpio", SD_SPI_GPIO_MOSI);
            cJSON_AddNumberToObject(root, "spi_miso_gpio", SD_SPI_GPIO_MISO);
            cJSON_AddNumberToObject(root, "spi_sclk_gpio", SD_SPI_GPIO_SCLK);
            cJSON_AddNumberToObject(root, "spi_cs_gpio", SD_SPI_GPIO_CS);
        }
        return root;
    }

    void InitializeI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
                .i2c_port = (i2c_port_t)1,
                .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
                .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .intr_priority = 0,
                .trans_queue_depth = 0,
                .flags = {
                                .enable_internal_pullup = 1,
                        },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = GPIO_NUM_21;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = GPIO_NUM_12;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeSdCard() {
        spi_bus_config_t bus_cfg = {};
        bus_cfg.mosi_io_num = SD_SPI_GPIO_MOSI;
        bus_cfg.miso_io_num = SD_SPI_GPIO_MISO;
        bus_cfg.sclk_io_num = SD_SPI_GPIO_SCLK;
        bus_cfg.quadwp_io_num = GPIO_NUM_NC;
        bus_cfg.quadhd_io_num = GPIO_NUM_NC;
        bus_cfg.max_transfer_sz = 4096;

        esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "Failed to initialize SD SPI bus: %s", esp_err_to_name(ret));
            return;
        }

        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        host.slot = SPI2_HOST;
        host.max_freq_khz = SDMMC_FREQ_PROBING;

        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.gpio_cs = SD_SPI_GPIO_CS;
        slot_config.host_id = SPI2_HOST;

        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024,
            .disk_status_check_enable = false,
            .use_one_fat = false,
        };

        ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &sd_card_);
        if (ret != ESP_OK) {
            sd_mounted_ = false;
            sd_card_ = nullptr;
            ESP_LOGW(TAG, "SD card mount failed: %s", esp_err_to_name(ret));
            return;
        }

        sd_mounted_ = true;
        ESP_LOGI(TAG, "SD card mounted at %s", SD_MOUNT_POINT);
    }

    esp_err_t IoExpanderSetLevel(uint16_t pin_mask, uint8_t level) {
        return esp_io_expander_set_level(io_expander, pin_mask, level);
    }

    uint8_t IoExpanderGetLevel(uint16_t pin_mask) {
        uint32_t pin_val = 0;
        esp_io_expander_get_level(io_expander, DRV_IO_EXP_INPUT_MASK, &pin_val);
        pin_mask &= DRV_IO_EXP_INPUT_MASK;
        return (uint8_t)((pin_val & pin_mask) ? 1 : 0);
    }

    void InitializeIoExpander() {
        esp_io_expander_new_i2c_tca95xx_16bit(
                i2c_bus_, ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_000, &io_expander);

        esp_err_t ret;
        ret = esp_io_expander_print_state(io_expander);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Print state failed: %s", esp_err_to_name(ret));
        }

        ret = esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1, IO_EXPANDER_OUTPUT);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Set direction failed: %s", esp_err_to_name(ret));
        }
        ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1, 0);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Set level failed: %s", esp_err_to_name(ret));
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
        ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1, 1);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Set level failed: %s", esp_err_to_name(ret));
        }
        ret = esp_io_expander_set_dir(
                io_expander, DRV_IO_EXP_INPUT_MASK,
                IO_EXPANDER_INPUT);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Set direction failed: %s", esp_err_to_name(ret));
        }
    }

    void InitializeButtons() {
        instance_ = this;

        // Button A
        button_config_t btn_a_config = {
            .long_press_time = 1000,
            .short_press_time = 0
        };
        btn_a_driver_ = (button_driver_t*)calloc(1, sizeof(button_driver_t));
        btn_a_driver_->enable_power_save = false;
        btn_a_driver_->get_key_level = [](button_driver_t *button_driver) -> uint8_t {
            return !instance_->IoExpanderGetLevel(IO_EXPANDER_PIN_NUM_2);
        };
        ESP_ERROR_CHECK(iot_button_create(&btn_a_config, btn_a_driver_, &btn_a));
        iot_button_register_cb(btn_a, BUTTON_SINGLE_CLICK, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<Df_K10Board*>(usr_data);
            self->display_->ScrollChat(-80);
        }, this);
        iot_button_register_cb(btn_a, BUTTON_DOUBLE_CLICK, nullptr, [](void* button_handle, void* usr_data) {
            auto& app = Application::GetInstance();
            app.ToggleChatState();
        }, this);
        iot_button_register_cb(btn_a, BUTTON_LONG_PRESS_START, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<Df_K10Board*>(usr_data);
            auto codec = self->GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);
            self->GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        }, this);

        // Button B
        button_config_t btn_b_config = {
            .long_press_time = 1000,
            .short_press_time = 0
        };
        btn_b_driver_ = (button_driver_t*)calloc(1, sizeof(button_driver_t));
        btn_b_driver_->enable_power_save = false;
        btn_b_driver_->get_key_level = [](button_driver_t *button_driver) -> uint8_t {
            return !instance_->IoExpanderGetLevel(IO_EXPANDER_PIN_NUM_12);
        };
        ESP_ERROR_CHECK(iot_button_create(&btn_b_config, btn_b_driver_, &btn_b));
        iot_button_register_cb(btn_b, BUTTON_SINGLE_CLICK, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<Df_K10Board*>(usr_data);
            self->display_->ScrollChat(80);
        }, this);
        static button_event_args_t btn_b_triple_click_args = { .multiple_clicks = { .clicks = 3 } };
        iot_button_register_cb(btn_b, BUTTON_MULTIPLE_CLICK, &btn_b_triple_click_args, [](void* button_handle, void* usr_data) {
            auto self = static_cast<Df_K10Board*>(usr_data);
            self->GetDisplay()->ShowNotification("Entering Wi-Fi configuration mode", 5000);
            SsidManager::GetInstance().Clear();
            self->EnterWifiConfigMode();
        }, this);
        iot_button_register_cb(btn_b, BUTTON_LONG_PRESS_START, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<Df_K10Board*>(usr_data);
            auto codec = self->GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
            self->GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        }, this);
    }

    void InitializeCamera() {
        ESP_LOGI(TAG, "Initializing K10 GC2145 DVP camera");
        static esp_cam_ctlr_dvp_pin_config_t dvp_pin_config = {
            .data_width = CAM_CTLR_DATA_WIDTH_8,
            .data_io = {
                [0] = CAMERA_PIN_D2,
                [1] = CAMERA_PIN_D3,
                [2] = CAMERA_PIN_D4,
                [3] = CAMERA_PIN_D5,
                [4] = CAMERA_PIN_D6,
                [5] = CAMERA_PIN_D7,
                [6] = CAMERA_PIN_D8,
                [7] = CAMERA_PIN_D9,
            },
            .vsync_io = CAMERA_PIN_VSYNC,
            .de_io = CAMERA_PIN_HREF,
            .pclk_io = CAMERA_PIN_PCLK,
            .xclk_io = CAMERA_PIN_XCLK,
        };

        esp_video_init_sccb_config_t sccb_config = {
            .init_sccb = false,
            .i2c_handle = i2c_bus_,
            .freq = 100000,
        };

        esp_video_init_dvp_config_t dvp_config = {
            .sccb_config = sccb_config,
            .reset_pin = CAMERA_PIN_RESET,
            .pwdn_pin = CAMERA_PIN_PWDN,
            .dvp_pin = dvp_pin_config,
            .xclk_freq = XCLK_FREQ_HZ,
        };

        esp_video_init_config_t video_config = {
            .dvp = &dvp_config,
        };

        camera_ = new EspVideo(video_config);

        Settings settings("df-k10", false);
        camera_flipped_ = static_cast<bool>(settings.GetInt("camera-flipped", 0));
        camera_->SetHMirror(camera_flipped_);
        camera_->SetVFlip(camera_flipped_);
        ESP_LOGI(TAG, "K10 camera init result: ready=%s flipped=%s",
            camera_->IsReady() ? "true" : "false",
            camera_flipped_ ? "true" : "false");
    }

    void InitializeIli9341Display() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        // 液晶屏控制IO初始化
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = GPIO_NUM_14;
        io_config.dc_gpio_num = GPIO_NUM_13;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io));

        // 初始化液晶屏驱动芯片
        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.bits_per_pixel = 16;
        panel_config.color_space = ESP_LCD_COLOR_SPACE_BGR;

        ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(panel_io, &panel_config, &panel));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, DISPLAY_BACKLIGHT_OUTPUT_INVERT));
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));

        display_ = new SpiLcdDisplay(panel_io, panel,
                                DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY);
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        led_strip_ = new CircularStrip(BUILTIN_LED_GPIO, 3);
        // Keep the LED object available locally, but avoid registering six extra
        // MCP tools because Xiaozhi limits the advertised tool count.
    }

    void InitializeSensors() {
        if (AddI2cDevice(AHT20_ADDR, &aht20_)) {
            uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
            I2cWrite(aht20_, init_cmd, sizeof(init_cmd));
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        if (AddI2cDevice(LTR303_ADDR, &ltr303_)) {
            I2cWriteReg(ltr303_, 0x80, 0x01);  // active mode
            I2cWriteReg(ltr303_, 0x85, 0x03);  // 100 ms integration, 50 ms measurement
        }

        if (AddI2cDevice(SC7A20_ADDR, &sc7a20_)) {
            I2cWriteReg(sc7a20_, 0x20, 0x57);  // 100 Hz, XYZ enabled
            I2cWriteReg(sc7a20_, 0x23, 0x00);  // +/-2g, high-resolution disabled
        }
    }

    cJSON* ReadEnvironmentJson() {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "sensor", "AHT20");
        cJSON_AddNumberToObject(root, "address", AHT20_ADDR);

        uint8_t cmd[3] = {0xAC, 0x33, 0x00};
        esp_err_t ret = I2cWrite(aht20_, cmd, sizeof(cmd));
        if (ret == ESP_OK) {
            vTaskDelay(80 / portTICK_PERIOD_MS);
            uint8_t data[6] = {};
            ret = i2c_master_receive(aht20_, data, sizeof(data), 100);
            if (ret == ESP_OK && (data[0] & 0x80) == 0) {
                uint32_t humi_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
                uint32_t temp_raw = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
                float humidity = humi_raw * 100.0f / 1048576.0f;
                float temperature = temp_raw * 200.0f / 1048576.0f - 50.0f;
                cJSON_AddBoolToObject(root, "available", true);
                cJSON_AddNumberToObject(root, "temperature_c", std::round(temperature * 10.0f) / 10.0f);
                cJSON_AddNumberToObject(root, "humidity_percent", std::round(humidity * 10.0f) / 10.0f);
                return root;
            }
        }

        cJSON_AddBoolToObject(root, "available", false);
        cJSON_AddStringToObject(root, "error", esp_err_to_name(ret));
        return root;
    }

    cJSON* ReadLightJson() {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "sensor", "LTR-303ALS");
        cJSON_AddNumberToObject(root, "address", LTR303_ADDR);

        uint8_t data[4] = {};
        auto ret = I2cReadReg(ltr303_, 0x88 | 0x80, data, sizeof(data));
        if (ret != ESP_OK) {
            cJSON_AddBoolToObject(root, "available", false);
            cJSON_AddStringToObject(root, "error", esp_err_to_name(ret));
            return root;
        }

        uint16_t ch1 = ((uint16_t)data[1] << 8) | data[0];
        uint16_t ch0 = ((uint16_t)data[3] << 8) | data[2];
        float ratio = (ch0 + ch1) == 0 ? 0.0f : static_cast<float>(ch1) / (ch0 + ch1);
        float lux = 0.0f;
        if (ratio < 0.45f) {
            lux = 1.7743f * ch0 + 1.1059f * ch1;
        } else if (ratio < 0.64f) {
            lux = 4.2785f * ch0 - 1.9548f * ch1;
        } else if (ratio < 0.85f) {
            lux = 0.5926f * ch0 + 0.1185f * ch1;
        }

        cJSON_AddBoolToObject(root, "available", true);
        cJSON_AddNumberToObject(root, "ch0", ch0);
        cJSON_AddNumberToObject(root, "ch1", ch1);
        cJSON_AddNumberToObject(root, "lux", std::round(lux));
        return root;
    }

    cJSON* ReadAccelerometerJson() {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "sensor", "SC7A20H");
        cJSON_AddNumberToObject(root, "address", SC7A20_ADDR);

        uint8_t data[6] = {};
        auto ret = I2cReadReg(sc7a20_, 0x28 | 0x80, data, sizeof(data));
        if (ret != ESP_OK) {
            cJSON_AddBoolToObject(root, "available", false);
            cJSON_AddStringToObject(root, "error", esp_err_to_name(ret));
            return root;
        }

        int16_t x = static_cast<int16_t>((data[1] << 8) | data[0]) >> 4;
        int16_t y = static_cast<int16_t>((data[3] << 8) | data[2]) >> 4;
        int16_t z = static_cast<int16_t>((data[5] << 8) | data[4]) >> 4;
        int magnitude = static_cast<int>(std::round(std::sqrt(x * x + y * y + z * z)));

        cJSON_AddBoolToObject(root, "available", true);
        cJSON_AddNumberToObject(root, "x_mg", x);
        cJSON_AddNumberToObject(root, "y_mg", y);
        cJSON_AddNumberToObject(root, "z_mg", z);
        cJSON_AddNumberToObject(root, "magnitude_mg", magnitude);
        return root;
    }

    cJSON* ListSdFilesJson(const std::string& requested_path) {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddBoolToObject(root, "mounted", sd_mounted_);
        if (!EnsureSdMounted()) {
            cJSON_AddStringToObject(root, "error", "SD card is not mounted");
            return root;
        }

        std::string path = MakeSdPath(requested_path);
        if (path.empty()) {
            cJSON_AddStringToObject(root, "error", "Invalid path");
            return root;
        }

        DIR* dir = opendir(path.c_str());
        if (dir == nullptr) {
            cJSON_AddStringToObject(root, "error", "Failed to open directory");
            cJSON_AddStringToObject(root, "path", path.c_str());
            return root;
        }

        cJSON_AddStringToObject(root, "path", path.c_str());
        cJSON* files = cJSON_AddArrayToObject(root, "files");
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "name", entry->d_name);
            std::string child_path = path + "/" + entry->d_name;
            struct stat st = {};
            if (stat(child_path.c_str(), &st) == 0) {
                cJSON_AddBoolToObject(item, "directory", S_ISDIR(st.st_mode));
                cJSON_AddNumberToObject(item, "size", static_cast<double>(st.st_size));
            }
            cJSON_AddItemToArray(files, item);
        }
        closedir(dir);
        return root;
    }

    std::string DefaultSdFilename(const char* prefix, const char* extension) {
        int64_t now_ms = esp_timer_get_time() / 1000;
        char buffer[96];
        snprintf(buffer, sizeof(buffer), "%s/%s_%lld.%s", SD_MOUNT_POINT, prefix,
            static_cast<long long>(now_ms), extension);
        return std::string(buffer);
    }

    cJSON* SaveCameraPhotoJson(const std::string& requested_path, int quality) {
        cJSON* root = cJSON_CreateObject();
        if (!EnsureSdMounted()) {
            cJSON_AddBoolToObject(root, "success", false);
            cJSON_AddStringToObject(root, "error", "SD card is not mounted");
            return root;
        }
        std::string path = requested_path.empty() ? DefaultSdFilename("camera", "jpg") : MakeSdPath(requested_path);
        if (path.empty()) {
            cJSON_AddBoolToObject(root, "success", false);
            cJSON_AddStringToObject(root, "error", "Invalid path");
            return root;
        }
        bool ok = camera_ != nullptr && camera_->Capture() && camera_->SaveJpegToFile(path, quality);
        cJSON_AddBoolToObject(root, "success", ok);
        cJSON_AddStringToObject(root, "path", path.c_str());
        return root;
    }

    cJSON* SaveScreenSnapshotJson(const std::string& requested_path, int quality) {
        cJSON* root = cJSON_CreateObject();
        if (!EnsureSdMounted()) {
            cJSON_AddBoolToObject(root, "success", false);
            cJSON_AddStringToObject(root, "error", "SD card is not mounted");
            return root;
        }
        std::string path = requested_path.empty() ? DefaultSdFilename("screen", "jpg") : MakeSdPath(requested_path);
        if (path.empty()) {
            cJSON_AddBoolToObject(root, "success", false);
            cJSON_AddStringToObject(root, "error", "Invalid path");
            return root;
        }
        std::string jpeg_data;
        bool ok = display_->SnapshotToJpeg(jpeg_data, quality) && WriteBinaryFile(path, jpeg_data);
        cJSON_AddBoolToObject(root, "success", ok);
        cJSON_AddStringToObject(root, "path", path.c_str());
        cJSON_AddNumberToObject(root, "bytes", static_cast<double>(jpeg_data.size()));
        return root;
    }

    void InitializeMcpTools() {
        auto& mcp_server = McpServer::GetInstance();

        mcp_server.AddTool("self.k10.get_status",
            "Get DFRobot UNIHIKER K10 capabilities and current controllable state.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                cJSON_AddStringToObject(root, "board", "DFRobot UNIHIKER K10");
                cJSON_AddStringToObject(root, "mcu", "ESP32-S3");
                cJSON_AddNumberToObject(root, "screen_width", DISPLAY_WIDTH);
                cJSON_AddNumberToObject(root, "screen_height", DISPLAY_HEIGHT);
                cJSON_AddNumberToObject(root, "rgb_led_count", 3);
                cJSON_AddBoolToObject(root, "led_light_detected", led_strip_ != nullptr);
                cJSON_AddStringToObject(root, "led_light_tool", "self.light.set_rgb");
                cJSON_AddBoolToObject(root, "memory_card_slot_detected", true);
                cJSON_AddBoolToObject(root, "memory_card_mounted", sd_mounted_);
                cJSON_AddStringToObject(root, "memory_card_tool", "self.k10.memory_card");
                cJSON_AddBoolToObject(root, "camera", camera_ != nullptr);
                cJSON_AddBoolToObject(root, "camera_ready", camera_ != nullptr && camera_->IsReady());
                cJSON_AddBoolToObject(root, "microphones", true);
                cJSON_AddBoolToObject(root, "speaker", true);
                cJSON_AddBoolToObject(root, "temperature_humidity_sensor", aht20_ != nullptr);
                cJSON_AddBoolToObject(root, "light_sensor", ltr303_ != nullptr);
                cJSON_AddBoolToObject(root, "accelerometer", sc7a20_ != nullptr);
                cJSON_AddBoolToObject(root, "sd_card_mounted", sd_mounted_);
                cJSON_AddBoolToObject(root, "gravity_i2c_port", true);
                cJSON_AddBoolToObject(root, "edge_connector", true);
                cJSON_AddBoolToObject(root, "camera_flipped", camera_flipped_);
                cJSON_AddNumberToObject(root, "screen_brightness", backlight_ ? backlight_->brightness() : -1);
                cJSON_AddNumberToObject(root, "volume", GetAudioCodec()->output_volume());
                return root;
            });

        auto set_led_color = [this](const PropertyList& properties) -> ReturnValue {
            if (led_strip_ == nullptr) {
                return false;
            }
            int index = properties["index"].value<int>();
            StripColor color = {
                static_cast<uint8_t>(properties["red"].value<int>()),
                static_cast<uint8_t>(properties["green"].value<int>()),
                static_cast<uint8_t>(properties["blue"].value<int>())
            };
            if (index < 0) {
                led_strip_->SetAllColor(color);
            } else {
                led_strip_->SetSingleColor(static_cast<uint8_t>(index), color);
            }
            return true;
        };

        #if 0
        mcp_server.AddTool("self.k10.get_buttons",
            "Read current DFRobot UNIHIKER K10 A/B button state.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                cJSON_AddBoolToObject(root, "a_pressed", !IoExpanderGetLevel(IO_EXPANDER_PIN_NUM_2));
                cJSON_AddBoolToObject(root, "b_pressed", !IoExpanderGetLevel(IO_EXPANDER_PIN_NUM_12));
                return root;
            });

        mcp_server.AddTool("self.k10.show_notification",
            "Show a short notification on the K10 screen.",
            PropertyList({
                Property("message", kPropertyTypeString),
                Property("duration_ms", kPropertyTypeInteger, 3000, 500, 30000)
            }), [this](const PropertyList& properties) -> ReturnValue {
                display_->ShowNotification(properties["message"].value<std::string>(),
                    properties["duration_ms"].value<int>());
                return true;
            });

        mcp_server.AddTool("self.k10.set_chat_message",
            "Show a chat message on the K10 screen.",
            PropertyList({
                Property("role", kPropertyTypeString, std::string("assistant")),
                Property("content", kPropertyTypeString)
            }), [this](const PropertyList& properties) -> ReturnValue {
                display_->SetChatMessage(properties["role"].value<std::string>().c_str(),
                    properties["content"].value<std::string>().c_str());
                return true;
            });

        mcp_server.AddTool("self.k10.clear_chat",
            "Clear chat messages from the K10 screen.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                display_->ClearChatMessages();
                return true;
            });

        mcp_server.AddTool("self.k10.set_emotion",
            "Set the K10 screen emotion. Common values: neutral, happy, sad, angry, surprised, thinking.",
            PropertyList({
                Property("emotion", kPropertyTypeString)
            }), [this](const PropertyList& properties) -> ReturnValue {
                display_->SetEmotion(properties["emotion"].value<std::string>().c_str());
                return true;
            });

        mcp_server.AddTool("self.k10.get_volume",
            "Get K10 speaker volume from 0 to 100.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return GetAudioCodec()->output_volume();
            });

        mcp_server.AddTool("self.k10.set_volume",
            "Set K10 speaker volume from 0 to 100.",
            PropertyList({
                Property("volume", kPropertyTypeInteger, 0, 100)
            }), [this](const PropertyList& properties) -> ReturnValue {
                int volume = properties["volume"].value<int>();
                GetAudioCodec()->SetOutputVolume(volume);
                display_->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
                return true;
            });

        mcp_server.AddTool("self.k10.get_environment",
            "Read K10 onboard AHT20 temperature and humidity sensor.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return ReadEnvironmentJson();
            });

        mcp_server.AddTool("self.k10.get_light",
            "Read K10 onboard LTR-303ALS ambient light sensor.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return ReadLightJson();
            });

        mcp_server.AddTool("self.k10.get_accelerometer",
            "Read K10 onboard SC7A20H accelerometer in milli-g.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return ReadAccelerometerJson();
            });
        #endif

        mcp_server.AddTool("self.k10.get_sensors",
            "Read all onboard K10 sensors: temperature, humidity, ambient light, and accelerometer.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                cJSON_AddItemToObject(root, "environment", ReadEnvironmentJson());
                cJSON_AddItemToObject(root, "light", ReadLightJson());
                cJSON_AddItemToObject(root, "accelerometer", ReadAccelerometerJson());
                return root;
            });

        mcp_server.AddTool("self.k10.set_rgb_led",
            "Set one or all K10 RGB LEDs / light / lamp. Use index -1 for all LEDs, or 0 to 2 for a single LED.",
            PropertyList({
                Property("index", kPropertyTypeInteger, -1, -1, 2),
                Property("red", kPropertyTypeInteger, 0, 0, 255),
                Property("green", kPropertyTypeInteger, 0, 0, 255),
                Property("blue", kPropertyTypeInteger, 0, 0, 255)
            }), set_led_color);

        mcp_server.AddTool("self.light.set_rgb",
            "Set the K10 built-in RGB light color. This controls the onboard lamp/LED. Use index -1 for all lights, or 0 to 2 for one light.",
            PropertyList({
                Property("index", kPropertyTypeInteger, -1, -1, 2),
                Property("red", kPropertyTypeInteger, 0, 0, 255),
                Property("green", kPropertyTypeInteger, 0, 0, 255),
                Property("blue", kPropertyTypeInteger, 0, 0, 255)
            }), set_led_color);

        mcp_server.AddTool("self.camera.get_status",
            "Get K10 camera driver status.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                cJSON_AddBoolToObject(root, "present", camera_ != nullptr);
                cJSON_AddBoolToObject(root, "ready", camera_ != nullptr && camera_->IsReady());
                cJSON_AddBoolToObject(root, "flipped", camera_flipped_);
                cJSON_AddStringToObject(root, "sensor", "GC2145");
                cJSON_AddStringToObject(root, "interface", "DVP");
                return root;
            });

        mcp_server.AddTool("self.camera.test_capture",
            "Test K10 camera capture and return whether it succeeds.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                bool ready = camera_ != nullptr && camera_->IsReady();
                bool captured = ready && camera_->Capture();
                ESP_LOGI(TAG, "Camera test capture: ready=%s captured=%s",
                    ready ? "true" : "false",
                    captured ? "true" : "false");
                cJSON_AddBoolToObject(root, "ready", ready);
                cJSON_AddBoolToObject(root, "captured", captured);
                return root;
            });

        #if 0
        mcp_server.AddTool("self.k10.set_screen_brightness",
            "Set K10 screen brightness from 0 to 100. Uses the LCD brightness register when supported by the panel.",
            PropertyList({
                Property("brightness", kPropertyTypeInteger, 0, 100)
            }), [this](const PropertyList& properties) -> ReturnValue {
                int brightness = properties["brightness"].value<int>();
                if (backlight_ == nullptr) {
                    return false;
                }
                backlight_->SetBrightness(static_cast<uint8_t>(brightness), true);
                return true;
            });

        mcp_server.AddTool("self.k10.get_sd_status",
            "Get K10 TF/MicroSD card status and SPI pin mapping.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                return GetSdStatusJson();
            });
        #endif

        mcp_server.AddTool("self.k10.sd_mount",
            "Mount or re-mount the K10 TF/MicroSD memory card.",
            PropertyList(), [this](const PropertyList&) -> ReturnValue {
                if (!sd_mounted_) {
                    InitializeSdCard();
                }
                return GetSdStatusJson();
            });

        mcp_server.AddTool("self.k10.memory_card",
            "Control the K10 memory card / SD card. action can be status, mount, or list. Status and list will try to mount the card first.",
            PropertyList({
                Property("action", kPropertyTypeString, std::string("mount")),
                Property("path", kPropertyTypeString, std::string("/"))
            }), [this](const PropertyList& properties) -> ReturnValue {
                std::string action = properties["action"].value<std::string>();
                if (!sd_mounted_) {
                    InitializeSdCard();
                }
                if (action == "mount" || action == "status") {
                    cJSON* root = GetSdStatusJson();
                    cJSON_AddBoolToObject(root, "memory_card_slot_detected", true);
                    cJSON_AddBoolToObject(root, "memory_card_detected", sd_mounted_);
                    if (!sd_mounted_) {
                        cJSON_AddStringToObject(root, "message", "Memory card slot is available, but no FAT-formatted card is mounted. Insert a card, then call mount again.");
                    }
                    return root;
                }
                if (action == "list") {
                    if (!sd_mounted_) {
                        return GetSdStatusJson();
                    }
                    return ListSdFilesJson(properties["path"].value<std::string>());
                }
                return GetSdStatusJson();
            });

        mcp_server.AddTool("self.wifi.reconfigure",
            "Request Wi-Fi reset, delete saved SSID, change Wi-Fi, change SSID, or enter Wi-Fi configuration mode. This is destructive and must be confirmed: first call with confirm=false to ask the user to say yes, then call with confirm=true only after the user says yes. Use action 'configure' to enter Wi-Fi config mode, or 'reset'/'clear'/'delete' to clear saved SSIDs before config mode.",
            PropertyList({
                Property("action", kPropertyTypeString, std::string("configure")),
                Property("confirm", kPropertyTypeBoolean, false)
            }), [this](const PropertyList& properties) -> ReturnValue {
                std::string action = properties["action"].value<std::string>();
                bool confirm = properties["confirm"].value<bool>();

                if (!confirm) {
                    wifi_reconfigure_pending_ = true;
                    display_->ShowNotification("Confirm Wi-Fi reconfiguration by saying yes", 10000);
                    cJSON* root = cJSON_CreateObject();
                    cJSON_AddBoolToObject(root, "confirmation_required", true);
                    cJSON_AddStringToObject(root, "message", "Ask the user to say yes to enter Wi-Fi configuration mode, or no to cancel.");
                    cJSON_AddStringToObject(root, "action", action.c_str());
                    return root;
                }

                if (!wifi_reconfigure_pending_) {
                    cJSON* root = cJSON_CreateObject();
                    cJSON_AddBoolToObject(root, "started", false);
                    cJSON_AddStringToObject(root, "error", "Confirmation was not requested first. Ask the user for confirmation, then call again with confirm=true.");
                    return root;
                }

                wifi_reconfigure_pending_ = false;
                bool clear_saved = action == "reset" || action == "clear" || action == "delete" || action == "erase";
                if (clear_saved) {
                    SsidManager::GetInstance().Clear();
                }

                display_->ShowNotification("Entering Wi-Fi configuration mode", 10000);
                EnterWifiConfigMode();

                cJSON* root = cJSON_CreateObject();
                cJSON_AddBoolToObject(root, "started", true);
                cJSON_AddBoolToObject(root, "saved_ssids_cleared", clear_saved);
                cJSON_AddStringToObject(root, "message", "Wi-Fi configuration mode is starting.");
                return root;
            });

        mcp_server.AddTool("self.k10.sd_list_files",
            "List files on the K10 TF/MicroSD memory card.",
            PropertyList({
                Property("path", kPropertyTypeString, std::string("/"))
            }), [this](const PropertyList& properties) -> ReturnValue {
                return ListSdFilesJson(properties["path"].value<std::string>());
            });

        mcp_server.AddTool("self.k10.sd_read_text",
            "Read a text file from the K10 TF/MicroSD memory card. Binary files are not supported by this tool.",
            PropertyList({
                Property("path", kPropertyTypeString),
                Property("max_bytes", kPropertyTypeInteger, 4096, 1, 16384)
            }), [this](const PropertyList& properties) -> ReturnValue {
                if (!EnsureSdMounted()) {
                    return std::string("SD card is not mounted");
                }
                std::string path = MakeSdPath(properties["path"].value<std::string>());
                if (path.empty()) {
                    return std::string("Invalid path");
                }
                return ReadTextFile(path, static_cast<size_t>(properties["max_bytes"].value<int>()));
            });

        mcp_server.AddTool("self.k10.sd_write_text",
            "Write or replace a text file on the K10 TF/MicroSD memory card.",
            PropertyList({
                Property("path", kPropertyTypeString),
                Property("content", kPropertyTypeString)
            }), [this](const PropertyList& properties) -> ReturnValue {
                if (!EnsureSdMounted()) {
                    return false;
                }
                std::string path = MakeSdPath(properties["path"].value<std::string>());
                if (path.empty()) {
                    return false;
                }
                return WriteBinaryFile(path, properties["content"].value<std::string>());
            });

        mcp_server.AddTool("self.k10.sd_save_camera_photo",
            "Capture a K10 camera photo and save it as a JPEG on the TF/MicroSD memory card.",
            PropertyList({
                Property("path", kPropertyTypeString, std::string("")),
                Property("quality", kPropertyTypeInteger, 80, 30, 95)
            }), [this](const PropertyList& properties) -> ReturnValue {
                return SaveCameraPhotoJson(properties["path"].value<std::string>(),
                    properties["quality"].value<int>());
            });

        #if 0
        mcp_server.AddTool("self.k10.sd_append_log",
            "Append one text line to a log file on the K10 TF/MicroSD card.",
            PropertyList({
                Property("path", kPropertyTypeString, std::string("/k10.log")),
                Property("line", kPropertyTypeString)
            }), [this](const PropertyList& properties) -> ReturnValue {
                if (!EnsureSdMounted()) {
                    return false;
                }
                std::string path = MakeSdPath(properties["path"].value<std::string>());
                if (path.empty()) {
                    return false;
                }
                return AppendTextFile(path, properties["line"].value<std::string>());
            });

        mcp_server.AddTool("self.k10.sd_save_camera_photo",
            "Capture a K10 camera photo and save it as a JPEG on the TF/MicroSD card.",
            PropertyList({
                Property("path", kPropertyTypeString, std::string("")),
                Property("quality", kPropertyTypeInteger, 80, 30, 95)
            }), [this](const PropertyList& properties) -> ReturnValue {
                return SaveCameraPhotoJson(properties["path"].value<std::string>(),
                    properties["quality"].value<int>());
            });

        mcp_server.AddTool("self.k10.sd_save_screen_snapshot",
            "Save the current K10 screen as a JPEG on the TF/MicroSD card.",
            PropertyList({
                Property("path", kPropertyTypeString, std::string("")),
                Property("quality", kPropertyTypeInteger, 80, 30, 95)
            }), [this](const PropertyList& properties) -> ReturnValue {
                return SaveScreenSnapshotJson(properties["path"].value<std::string>(),
                    properties["quality"].value<int>());
            });

        mcp_server.AddTool("self.k10.sd_play_audio",
            "Report SD audio playback support. This firmware can store and read files, but SD file streaming into the voice decoder is not implemented yet.",
            PropertyList({
                Property("path", kPropertyTypeString)
            }), [this](const PropertyList& properties) -> ReturnValue {
                cJSON* root = cJSON_CreateObject();
                cJSON_AddBoolToObject(root, "success", false);
                cJSON_AddStringToObject(root, "path", properties["path"].value<std::string>().c_str());
                cJSON_AddStringToObject(root, "error", "SD audio playback is not implemented in the current Xiaozhi audio pipeline");
                return root;
            });
        #endif

        mcp_server.AddTool("self.camera.set_camera_flipped",
            "Flip or unflip K10 camera image horizontally and vertically.",
            PropertyList({
                Property("flipped", kPropertyTypeBoolean)
            }), [this](const PropertyList& properties) -> ReturnValue {
                camera_flipped_ = properties["flipped"].value<bool>();
                camera_->SetHMirror(camera_flipped_);
                camera_->SetVFlip(camera_flipped_);
                Settings settings("df-k10", true);
                settings.SetInt("camera-flipped", camera_flipped_ ? 1 : 0);
                return true;
            });
    }

public:
    Df_K10Board() {
        InitializeI2c();
        InitializeIoExpander();
        InitializeSensors();
        InitializeSpi();
        InitializeIli9341Display();
        ESP_LOGI(TAG, "Skipping SD auto-mount during boot; use self.k10.sd_mount after startup");
        InitializeButtons();
        InitializeIot();
        InitializeCamera();
        InitializeMcpTools();
    }

    virtual Led* GetLed() override {
        return led_strip_;
    }

    virtual AudioCodec *GetAudioCodec() override {
        static K10AudioCodec audio_codec(
                    i2c_bus_,
                    AUDIO_INPUT_SAMPLE_RATE,
                    AUDIO_OUTPUT_SAMPLE_RATE,
                    AUDIO_I2S_GPIO_MCLK,
                    AUDIO_I2S_GPIO_BCLK,
                    AUDIO_I2S_GPIO_WS,
                    AUDIO_I2S_GPIO_DOUT,
                    AUDIO_I2S_GPIO_DIN,
                    AUDIO_CODEC_PA_PIN,
                    AUDIO_CODEC_ES8311_ADDR,
                    AUDIO_CODEC_ES7210_ADDR,
                    AUDIO_INPUT_REFERENCE);
        return &audio_codec;
    }

    virtual Camera* GetCamera() override {
        return camera_;
    }

    virtual Display *GetDisplay() override {
        return display_;
    }

    virtual Backlight* GetBacklight() override {
        return backlight_;
    }
};

DECLARE_BOARD(Df_K10Board);

Df_K10Board* Df_K10Board::instance_ = nullptr;
