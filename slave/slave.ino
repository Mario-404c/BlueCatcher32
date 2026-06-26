// Change the name of your headphones and upload this code on earphones 1 and 2

#include <BluetoothA2DPSource.h>
#include <driver/i2s.h>
#include <freertos/ringbuf.h>
#include <math.h>

#define I2S_BCLK    26
#define I2S_WCLK    25
#define I2S_DIN     22
#define BUFFER_SIZE 128  
#define LED_CONNESSO  2
#define LED_DISCONNESSO  14
#define NAME "Insert name of your headphones"

BluetoothA2DPSource a2dp_source;
RingbufHandle_t ring_buf;

int32_t get_audio_data(Frame *frame, int32_t num_frames) {
    int frames_filled = 0;

    while (frames_filled < num_frames) {
        size_t item_size;
        int16_t *samples = (int16_t *)xRingbufferReceive(ring_buf, &item_size, pdMS_TO_TICKS(5));
        if (samples != NULL) {
            int available_frames = item_size / 4;
            int to_copy = min(available_frames, num_frames - frames_filled);
            for (int i = 0; i < to_copy; i++) {
                frame[frames_filled + i].channel1 = samples[i * 2];
                frame[frames_filled + i].channel2 = samples[i * 2 + 1];
            }
            frames_filled += to_copy;
            vRingbufferReturnItem(ring_buf, samples);
        } else {
            for (int i = frames_filled; i < num_frames; i++) {
                frame[i].channel1 = 0;
                frame[i].channel2 = 0;
            }
            frames_filled = num_frames;
        }
    }
    return num_frames;
}

void i2s_reader_task(void *param) {
    int16_t buffer[BUFFER_SIZE];
    size_t bytes_read;
    uint32_t count = 0;
    while (true) {
        i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
        count++;
        if (count % 200 == 0) {
            Serial.printf("I2S letti: %d blocchi\n", count);
        }
        xRingbufferSend(ring_buf, buffer, bytes_read, pdMS_TO_TICKS(5));
    }
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr) {
    switch(state) {
        case ESP_A2D_CONNECTION_STATE_CONNECTED:
            Serial.println("Connected");
            digitalWrite(LED_CONNESSO, HIGH);
            digitalWrite(LED_DISCONNESSO, LOW);
            break;
        case ESP_A2D_CONNECTION_STATE_DISCONNECTED:
        case ESP_A2D_CONNECTION_STATE_CONNECTING:
            Serial.println("Disconnected/Connecting");
            digitalWrite(LED_CONNESSO, LOW);
            digitalWrite(LED_DISCONNESSO, HIGH);
            break;
        default:
            break;
    }
}

void audio_state_changed(esp_a2d_audio_state_t state, void *ptr) {
    Serial.print("Audio: ");
    switch(state) {
        case ESP_A2D_AUDIO_STATE_STARTED: Serial.println("Started"); break;
        case ESP_A2D_AUDIO_STATE_STOPPED: Serial.println("Stopped"); break;
        default: Serial.println("Unknown"); break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_CONNESSO, OUTPUT);
    pinMode(LED_DISCONNESSO, OUTPUT);
    digitalWrite(LED_CONNESSO, LOW);
    digitalWrite(LED_DISCONNESSO, HIGH);

    i2s_config_t i2s_config = {
        .mode                 = (i2s_mode_t)(I2S_MODE_SLAVE | I2S_MODE_RX),
        .sample_rate          = 44100,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 16, 
        .dma_buf_len          = 32,  
        .use_apll             = true
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num   = I2S_BCLK,
        .ws_io_num    = I2S_WCLK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = I2S_DIN
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);

    ring_buf = xRingbufferCreate(1024 * 32, RINGBUF_TYPE_BYTEBUF);

    xTaskCreatePinnedToCore(i2s_reader_task, "i2s_reader", 8192, NULL, configMAX_PRIORITIES - 1, NULL, 1);

    a2dp_source.set_on_connection_state_changed(connection_state_changed);
    a2dp_source.set_on_audio_state_changed(audio_state_changed);

    a2dp_source.start(NAME, get_audio_data);
}

void loop() {
    delay(1000);
}