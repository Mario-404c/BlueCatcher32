#include <BluetoothA2DPSink.h>

BluetoothA2DPSink a2dp_sink;

#define LED_CONNESSO  2
#define LED_DISCONNESSO  4

void stato_conn_cambiato(esp_a2d_connection_state_t state, void *ptr) {
    Serial.print("Connessione: ");
    Serial.println(a2dp_sink.to_str(state));

    if(state == ESP_A2D_CONNECTION_STATE_CONNECTED){
        digitalWrite(LED_CONNESSO, HIGH);
        digitalWrite(LED_DISCONNESSO, LOW);
    }else{
        digitalWrite(LED_CONNESSO, LOW);
        digitalWrite(LED_DISCONNESSO, HIGH);
    }
}

void stato_audio_cambiato(esp_a2d_audio_state_t state, void *ptr) {
    Serial.print("Audio: ");
    Serial.println(a2dp_sink.to_str(state));
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_CONNESSO, OUTPUT);
    pinMode(LED_DISCONNESSO, OUTPUT);
    digitalWrite(LED_CONNESSO, LOW);
    digitalWrite(LED_DISCONNESSO, HIGH);  

    i2s_pin_config_t pin_config = {
        .bck_io_num   = 26,
        .ws_io_num    = 25,
        .data_out_num = 22,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };

    a2dp_sink.set_pin_config(pin_config);
    a2dp_sink.set_on_connection_state_changed(stato_conn_cambiato);
    a2dp_sink.set_on_audio_state_changed(stato_audio_cambiato);
    a2dp_sink.start("ESP32-Audio_in");
}

void loop() {
    delay(1000);
}