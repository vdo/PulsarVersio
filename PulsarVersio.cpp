/**
 * PulsarVersio - Pulsar Synthesis Oscillator for Noise Engineering Versio
 *
 * A firmware implementing Curtis Roads' Pulsar Synthesis technique
 * for the Daisy-based Versio eurorack platform.
 */

#include "daisy_versio.h"
#include "PulsarEngine.hpp"
#include <cmath>

using namespace daisy;

DaisyVersio hw;
PulsarEngine pulsar;

float sampleRate;
float outputLevel = 0.8f;

// Hard sync zero-crossing detection
float prevSyncIn = 0.0f;

// Gate state for edge detection
bool prevGate = false;

// Calibration state
bool inCalibration = false;
const int CALIBRATION_MAX = 65536;
const int CALIBRATION_MIN = 63200;
uint16_t calibrationOffset = 64262;
uint16_t calibrationUnitsPerVolt = 12826;
const uint16_t CALIBRATION_THRESH = CALIBRATION_MAX - 200;

// Base frequencies for each range
const float BASE_FREQ_LOW = 4.0f;      // LFO range
const float BASE_FREQ_MID = 65.41f;    // C2
const float BASE_FREQ_HIGH = 261.63f;  // C4

// Persistence
struct Settings {
    float calibrationOffset;
    float calibrationUnitsPerVolt;
    bool operator!=(const Settings& a) {
        return a.calibrationUnitsPerVolt != calibrationUnitsPerVolt;
    }
};

PersistentStorage<Settings> storage(hw.seed.qspi);

void SaveData() {
    Settings& settings = storage.GetSettings();
    settings.calibrationOffset = calibrationOffset;
    settings.calibrationUnitsPerVolt = calibrationUnitsPerVolt;
    storage.Save();
}

void LoadData() {
    Settings& settings = storage.GetSettings();
    calibrationOffset = settings.calibrationOffset;
    calibrationUnitsPerVolt = settings.calibrationUnitsPerVolt;
}

static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out,
                          size_t size) {
    for (size_t i = 0; i < size; ++i) {
        float syncIn = IN_L[i];
        float ringIn = IN_R[i];

        // Hard sync: detect rising zero-crossing on IN_L
        if (prevSyncIn <= 0.0f && syncIn > 0.0f) {
            pulsar.Sync();
        }
        prevSyncIn = syncIn;

        // Generate pulsar sample
        float sample = pulsar.Process() * outputLevel;

        // Ring modulation on right channel
        float ringOut = sample * (1.0f + ringIn);

        OUT_L[i] = sample;
        OUT_R[i] = ringOut;
    }
}

void WaitForButton() {
    while (!hw.tap.RisingEdge()) {
        hw.tap.Debounce();
    }
    while (!hw.tap.FallingEdge()) {
        hw.tap.Debounce();
    }
    System::Delay(200);
}

void DoCalibration() {
    inCalibration = true;
    const uint8_t NUM_SAMPLES = 10;

    // Step 0: Release button
    hw.tap.Debounce();
    hw.SetLed(0, 1, 1, 1);
    hw.SetLed(1, 1, 1, 1);
    hw.SetLed(2, 1, 1, 1);
    hw.SetLed(3, 1, 1, 1);
    hw.UpdateLeds();

    while (hw.tap.RawState()) {
        hw.tap.Debounce();
    }

    // Step 1: 1V reference
    hw.SetLed(0, 0, 1, 0);
    hw.SetLed(1, 0, 0, 0);
    hw.SetLed(2, 0, 0, 0);
    hw.SetLed(3, 0, 0, 0);
    hw.UpdateLeds();
    WaitForButton();
    float oneVoltValue = hw.knobs[0].GetRawValue();

    // Step 2: 2V reference
    hw.SetLed(0, 0, 0, 1);
    hw.SetLed(1, 0, 0, 1);
    hw.SetLed(2, 0, 0, 0);
    hw.SetLed(3, 0, 0, 0);
    hw.UpdateLeds();
    WaitForButton();

    float total = 0;
    for (uint8_t x = 0; x < NUM_SAMPLES; ++x) {
        hw.knobs[0].Process();
        total += hw.knobs[0].GetRawValue();
    }
    float twoVoltValue = total / NUM_SAMPLES;

    // Step 3: 3V reference
    hw.SetLed(0, 0, 1, 1);
    hw.SetLed(1, 0, 1, 1);
    hw.SetLed(2, 0, 1, 1);
    hw.SetLed(3, 0, 0, 0);
    hw.UpdateLeds();
    WaitForButton();

    total = 0;
    for (uint8_t x = 0; x < NUM_SAMPLES; ++x) {
        hw.knobs[0].Process();
        total += hw.knobs[0].GetRawValue();
    }
    float threeVoltValue = total / NUM_SAMPLES;

    // Calculate calibration values
    float firstEstimate = oneVoltValue - twoVoltValue;
    float secondEstimate = twoVoltValue - threeVoltValue;
    float avgEstimate = (firstEstimate + secondEstimate) / 2.0f;
    uint16_t offset = oneVoltValue + avgEstimate;

    // Save
    calibrationOffset = offset;
    calibrationUnitsPerVolt = static_cast<uint16_t>(avgEstimate + 0.5f);
    SaveData();

    inCalibration = false;
}

float GetVoctFrequency(float baseFreq) {
    float rawCv = hw.knobs[hw.KNOB_0].GetRawValue();
    float volts;

    if (rawCv > CALIBRATION_MIN) {
        volts = 0.0f;
    } else {
        volts = (calibrationOffset - rawCv) / static_cast<float>(calibrationUnitsPerVolt);
        if (volts < 0.0f) volts = 0.0f;
        if (volts > 5.0f) volts = 5.0f;
    }

    return baseFreq * powf(2.0f, volts);
}

int main(void) {
    // Initialize hardware
    hw.Init();
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
    hw.StartAdc();

    sampleRate = hw.AudioSampleRate();

    // Initialize pulsar engine
    pulsar.Init(sampleRate);

    // Initialize persistent storage
    Settings defaults;
    defaults.calibrationOffset = calibrationOffset;
    defaults.calibrationUnitsPerVolt = calibrationUnitsPerVolt;
    storage.Init(defaults);
    LoadData();

    // Validate calibration data
    if (calibrationUnitsPerVolt < 400 || calibrationUnitsPerVolt > 20000) {
        storage.RestoreDefaults();
        LoadData();
    }

    // Check for calibration mode: both switches right + button held
    hw.ProcessAllControls();
    hw.tap.Debounce();
    if (hw.sw[0].Read() == hw.sw->POS_RIGHT &&
        hw.sw[1].Read() == hw.sw->POS_RIGHT) {
        if (hw.tap.RawState()) {
            DoCalibration();
        }
    }

    // Start audio
    hw.StartAudio(AudioCallback);

    // LED feedback values
    float ledPhase = 0.0f;
    float ledFormant = 0.0f;
    float ledShape = 0.0f;

    while (1) {
        hw.ProcessAllControls();
        hw.tap.Debounce();

        // Read top switch: Masking mode
        // LEFT = OFF, CENTER = BURST, RIGHT = STOCHASTIC
        MaskingMode maskMode;
        if (hw.sw[0].Read() == hw.sw->POS_LEFT) {
            maskMode = MaskingMode::OFF;
        } else if (hw.sw[0].Read() == hw.sw->POS_CENTER) {
            maskMode = MaskingMode::BURST;
        } else {
            maskMode = MaskingMode::STOCHASTIC;
        }
        pulsar.SetMaskingMode(maskMode);

        // Read bottom switch: Frequency range
        // LEFT = LO (LFO), CENTER = MID, RIGHT = HI
        float baseFreq;
        if (hw.sw[1].Read() == hw.sw->POS_LEFT) {
            baseFreq = BASE_FREQ_LOW;
        } else if (hw.sw[1].Read() == hw.sw->POS_CENTER) {
            baseFreq = BASE_FREQ_MID;
        } else {
            baseFreq = BASE_FREQ_HIGH;
        }

        // KNOB_0: V/oct pitch
        float freq = GetVoctFrequency(baseFreq);
        pulsar.SetFrequency(freq);

        // KNOB_1: Formant ratio (duty cycle)
        // 0 = short duty (bright), 1 = full duty (mellow)
        float formantRatio = hw.GetKnobValue(DaisyVersio::KNOB_1);
        formantRatio = 0.05f + formantRatio * 0.95f;
        pulsar.SetFormantRatio(formantRatio);
        ledFormant = hw.GetKnobValue(DaisyVersio::KNOB_1);

        // KNOB_2: Pulsaret waveform shape (0-6 morph)
        float waveformMorph = hw.GetKnobValue(DaisyVersio::KNOB_2) * 6.0f;
        pulsar.SetWaveformMorph(waveformMorph);
        ledShape = hw.GetKnobValue(DaisyVersio::KNOB_2);

        // KNOB_3: Pulsaret envelope type (0-6 morph)
        float envelopeMorph = hw.GetKnobValue(DaisyVersio::KNOB_3) * 6.0f;
        pulsar.SetEnvelopeMorph(envelopeMorph);

        // KNOB_4: Burst count OR Masking probability (depending on mode)
        float knob4 = hw.GetKnobValue(DaisyVersio::KNOB_4);

        // KNOB_5: Rest count OR Fold amount (depending on mode)
        float knob5 = hw.GetKnobValue(DaisyVersio::KNOB_5);

        // Apply masking parameters based on mode
        if (maskMode == MaskingMode::OFF) {
            // No masking, KNOB_5 controls fold
            pulsar.SetFold(knob5);
        } else if (maskMode == MaskingMode::BURST) {
            // Burst masking
            int burstCount = 1 + static_cast<int>(knob4 * 7.0f);  // 1-8
            int restCount = static_cast<int>(knob5 * 7.0f);       // 0-7
            pulsar.SetBurstRatio(burstCount, restCount);
            pulsar.SetFold(0.0f);
        } else {
            // Stochastic masking
            pulsar.SetMaskingProbability(knob4);
            pulsar.SetFold(knob5);
        }

        // KNOB_6: Output level
        outputLevel = hw.GetKnobValue(DaisyVersio::KNOB_6);

        // Button or Gate: Reset phase
        bool gate = hw.Gate();
        if (hw.tap.RisingEdge() || (gate && !prevGate)) {
            pulsar.Reset();
        }
        prevGate = gate;

        // Update LEDs
        if (!inCalibration) {
            // LED_0: Phase indicator (cyan pulse)
            ledPhase = pulsar.IsInPulsaret() ? 0.8f : 0.1f;
            hw.SetLed(hw.LED_0, 0, ledPhase * 0.5f, ledPhase * 0.5f);

            // LED_1: Formant (green)
            hw.SetLed(hw.LED_1, 0, ledFormant, 0);

            // LED_2: Shape (orange)
            hw.SetLed(hw.LED_2, ledShape, ledShape * 0.5f, 0);

            // LED_3: Output level (white/magenta based on mode)
            if (maskMode == MaskingMode::OFF) {
                hw.SetLed(hw.LED_3, outputLevel, outputLevel, outputLevel);
            } else {
                hw.SetLed(hw.LED_3, outputLevel, 0, outputLevel * 0.7f);
            }

            hw.UpdateLeds();
        }
    }
}
