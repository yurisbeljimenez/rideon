#include "SoundController.h"
#include <Arduino.h>
#include "driver/i2s.h"

// --- I2S Configuration Constants ---
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 22050
#define BUFFER_SIZE 256

// --- Engine Sound Tuning ---
#define IDLE_FREQ 80.0f
#define MAX_FREQ 600.0f
#define SPEED_SMOOTHING 0.01f // How quickly the sound pitch changes
#define LFO_FREQ 30.0f
#define MIN_MOD_DEPTH 1.0f
#define MAX_MOD_DEPTH 15.0f

int16_t i2s_buffer[BUFFER_SIZE];

SoundController::SoundController(int bclkPin, int lrcPin, int dinPin, Logger* logger) {
  _bclkPin = bclkPin;
  _lrcPin = lrcPin;
  _dinPin = dinPin;
  _logger = logger;
}

void SoundController::setup() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true
  };
  i2s_pin_config_t pin_config = {
      .bck_io_num = _bclkPin,
      .ws_io_num = _lrcPin,
      .data_out_num = _dinPin,
      .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
  if (_logger) _logger->log("Initialized");
}

void SoundController::update() {
  fillBuffer();
}

// Sets the target motor speed from the main loop.
void SoundController::setMotorCommand(int speed) {
  _targetSpeed = constrain(speed, -255, 255);
}

void SoundController::fillBuffer() {
  // Smoothly move the sound's current speed towards the target speed.
  _currentSpeed = _currentSpeed + (_targetSpeed - _currentSpeed) * SPEED_SMOOTHING;
  
  // The "RPM" is based on the absolute value of the smoothed speed.
  float rpm = abs(_currentSpeed);
  float base_freq = map(rpm, 0, 255, IDLE_FREQ, MAX_FREQ);
  
  // The "Engine Load" is calculated by how quickly the speed is increasing.
  float load = (_currentSpeed - _previousSpeed);
  if (load < 0) load = 0; // Only apply load effect on acceleration
  _previousSpeed = _currentSpeed;
  
  // Map the load to the aggressiveness of the sound.
  float mod_depth = map(load, 0, 5, MIN_MOD_DEPTH, MAX_MOD_DEPTH); // A small change in speed creates a big change in sound
  mod_depth = constrain(mod_depth, MIN_MOD_DEPTH, MAX_MOD_DEPTH);

  float lfo_phase_step = (LFO_FREQ * 2 * PI) / SAMPLE_RATE;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    float freq_offset = sin(_lfoPhase) * mod_depth;
    float final_freq = base_freq + freq_offset;
    float phase_step = (final_freq * 2 * PI) / SAMPLE_RATE;

    int16_t sample = 5000 * sin(_phase);
    i2s_buffer[i] = (sample << 16) | (sample & 0xffff);
    
    _phase += phase_step;
    if (_phase >= 2 * PI) _phase -= 2 * PI;

    _lfoPhase += lfo_phase_step;
    if (_lfoPhase >= 2 * PI) _lfoPhase -= 2 * PI;
  }

  size_t bytes_written = 0;
  i2s_write(I2S_PORT, i2s_buffer, sizeof(i2s_buffer), &bytes_written, portMAX_DELAY);
}
