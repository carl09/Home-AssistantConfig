From https://github.com/mvturnho/esphome/tree/ili9341/esphome/components/ili9341


```yaml
font:
  - file: "fonts/OpenSans-Regular.ttf"
    id: my_font
    size: 20

display:
  - platform: ili9341
    model: TFT_2.4
    cs_pin: 14
    dc_pin: 27
    led_pin: 32
    # led_pin:
    #   number: 5
    #   inverted: yes
    reset_pin: 33
    rotation: 0
    lambda: |-
      // Draw a circle in the middle of the display
      it.fill(ili9341::BLACK);
      it.filled_circle(it.get_width() / 2, it.get_height() / 2, 30, ili9341::RED);
      it.rectangle(10,10,100,100,ili9341::BLUE);
      it.print(0, 0, id(my_font), ili9341::GREEN,"Hello World!");
```


```yaml
globals:
   - id: my_global_int
     type: int
     restore_value: no
     initial_value: '0'

font:
  - file: "fonts/OpenSans-Regular.ttf"
    id: my_font
    size: 20
  - file: "fonts/OpenSans-BoldItalic.ttf"
    id: bold_italic
    size: 30

display:
  - platform: ili9341
    model: TFT_2.4
    cs_pin: 14
    dc_pin: 27
    led_pin: 32
    reset_pin: 33
    rotation: 270
    lambda: |-
      //function
      if(id(my_global_int) < 1 ) {
        it.rectangle(0,0,318,238,ili9341::WHITE);
        it.rectangle(1,1,316,236,ili9341::WHITE);
        it.horizontal_line(2,50,316,ili9341::BLUE);
        it.horizontal_line(2,51,316,ili9341::BLUE);
        it.print(120, 10, id(bold_italic), ili9341::GREEN,"ili9341");
        it.printf(30, 100, id(my_font), ili9341::YELLOW, "Counter value:");
        it.printf(30, 170, id(my_font), ili9341::YELLOW, "Wifi connection status:");
      } else {
        it.filled_rectangle(185,100,40,30,ili9341::BLACK);
        it.printf(190, 100, id(my_font), ili9341::RED, "%1d", id(my_global_int));
      }
      if(wifi_wificomponent->is_connected()) {
        it.filled_circle(270, 185, 10, ili9341::GREEN);
      } else {
        it.filled_circle(270, 185, 10, ili9341::RED);
      }
      id(my_global_int) += 1;
```

```yaml
esphome:
  name: ttgot4
  platform: ESP32
  board: nodemcu-32s
  platformio_options:
    upload_speed: 921600
    monitor_speed: 115200

logger:
  level: DEBUG

wifi:
  ssid: !secret wifi_kelpnet_up_ssid
  password: !secret wifi_kelpnet_password
  
api:
  port: 6053
  password: 'secret'
  reboot_timeout: 0s

ota:
  safe_mode: True
  password: !secret ota_password
  port: 3286
  
time:
  - platform: homeassistant
    id: esptime
    timezone: Europe/Amsterdam

spi:
  clk_pin: 18
  mosi_pin: 23
  miso_pin: 12

globals:
   - id: my_global_int
     type: int
     restore_value: no
     initial_value: '0'
     
binary_sensor:
  - platform: gpio
    name: "BUTTON_A_PIN"
    id: BUTTON_A
    pin:
      number: 38
      inverted: True
      mode: INPUT_PULLUP
  - platform: gpio
    name: "BUTTON_B_PIN"
    id: BUTTON_B
    pin:
      number: 37
      inverted: True
      mode: INPUT_PULLUP
  - platform: gpio
    name: "BUTTON_C_PIN"
    id: BUTTON_C
    pin:
      number: 39
      inverted: True
      mode: INPUT_PULLUP

sensor:
  - platform: homeassistant
    id: inside_temperature
    entity_id: sensor.temp_2
    internal: true

text_sensor:
  - platform: homeassistant
    id: thermostat
    entity_id: climate.thermostat
    internal: true

font:
  - file: "fonts/OpenSans-Regular.ttf"
    id: my_font
    size: 20
  - file: "fonts/OpenSans-BoldItalic.ttf"
    id: bold_italic
    size: 30
  - file: "fonts/icon-works.ttf"
    id: iconworks
    size: 30
    
image:
  - file: "images/flame_color.png"
    id: flame
    resize: 50x80
    type: "RGB565"

display:
  - platform: ili9341
    id: main_display
    model: TFT_2.4
    cs_pin: 27
    dc_pin: 26
    led_pin: 4
    reset_pin: 5
    rotation: 270
    update_interval: 500ms
    lambda: |-
      if(id(my_global_int) < 1 ) {
        it.image(30, 70, id(flame));
        it.print(5, 2, id(iconworks), ili9341::WHITE,".");
        it.rectangle(0,0,318,238,ili9341::WHITE);
        it.rectangle(1,1,316,236,ili9341::WHITE);
        it.horizontal_line(2,50,316,ili9341::BLUE);
        it.horizontal_line(2,51,316,ili9341::BLUE);
        it.print(120, 5, id(bold_italic), ili9341::GREEN,"ili9341");
        it.printf(30, 170, id(my_font), ili9341::YELLOW, "Wifi connection status:");
      } else {
        it.filled_rectangle(10,200,100,30,ili9341::BLACK);
        it.printf(10, 200, id(my_font), ili9341::RED, "%1d", id(my_global_int));
        it.filled_rectangle(250,10,60,30,ili9341::BLACK);
        it.strftime(250, 10, id(my_font),ili9341::WHITE, TextAlign::TOP_LEFT, "%H:%M", id(esptime).now() );
        it.filled_rectangle(225,70,80,30,ili9341::BLACK);
        it.printf(230, 70, id(my_font), ili9341::WHITE, TextAlign::TOP_LEFT , "%.1f°C", id(inside_temperature).state );
      }
      if(wifi_wificomponent->is_connected()) {
        it.print(270, 175, id(iconworks), ili9341::GREEN,"6");
        //it.filled_circle(270, 185, 10, ili9341::GREEN);
      } else {
        it.print(270, 175, id(iconworks), ili9341::RED,"6");
        //it.filled_circle(270, 185, 10, ili9341::RED);
      }
      id(my_global_int) += 1;
```