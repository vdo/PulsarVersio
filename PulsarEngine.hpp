#pragma once
#ifndef PULSAR_ENGINE_HPP
#define PULSAR_ENGINE_HPP

#include <cstdint>
#include <cmath>

// Maximum number of waveform table points
static constexpr int WAVETABLE_SIZE = 256;

// Pulsaret waveform types
enum class PulsaretWaveform {
    SINE = 0,
    TRIANGLE,
    SAW_UP,
    SAW_DOWN,
    SQUARE,
    PULSE,
    NOISE
};

// Pulsaret envelope types
enum class PulsaretEnvelope {
    RECTANGULAR = 0,
    GAUSSIAN,
    EXPODEC,
    LINEAR_DECAY,
    LINEAR_ATTACK,
    EXPO_ATTACK,
    FOF  // Formant synthesis style
};

// Masking mode
enum class MaskingMode {
    OFF = 0,
    BURST,
    STOCHASTIC
};

class PulsarEngine {
public:
    PulsarEngine() { Init(48000.0f); }
    ~PulsarEngine() = default;

    // Initialize with sample rate
    void Init(float sampleRate);

    // Reset phase and state
    void Reset();

    // Hard sync - reset phase immediately
    void Sync();

    // Process one sample
    float Process();

    // Set fundamental frequency (Hz) - the pulsar repetition rate
    void SetFrequency(float freq);

    // Set formant frequency (Hz) - determines duty cycle
    // If formantFreq > fundamentalFreq, duty cycle < 100%
    void SetFormantFrequency(float freq);

    // Set formant ratio (0.0 to 1.0) - alternative to SetFormantFrequency
    // 0.0 = very short duty cycle (bright), 1.0 = full duty cycle (sine-like)
    void SetFormantRatio(float ratio);

    // Set pulsaret waveform type
    void SetWaveform(PulsaretWaveform waveform);

    // Set pulsaret waveform by interpolated index (0.0 to 6.0)
    void SetWaveformMorph(float morphValue);

    // Set pulsaret envelope type
    void SetEnvelope(PulsaretEnvelope envelope);

    // Set envelope by interpolated index (0.0 to 6.0)
    void SetEnvelopeMorph(float morphValue);

    // Set wavefolding amount (0.0 to 1.0)
    void SetFold(float amount);

    // Set burst masking ratio
    // burst = number of pulsars to emit, rest = number to skip
    void SetBurstRatio(int burst, int rest);

    // Set stochastic masking probability (0.0 to 1.0)
    // 1.0 = all pulsars emit, 0.0 = no pulsars emit
    void SetMaskingProbability(float probability);

    // Set masking mode
    void SetMaskingMode(MaskingMode mode);

    // Set output amplitude (0.0 to 1.0)
    void SetAmplitude(float amp);

    // Get current phase (0.0 to 1.0)
    float GetPhase() const { return phase_; }

    // Check if currently in pulsaret (not in silent interval)
    bool IsInPulsaret() const { return inPulsaret_; }

private:
    // Generate waveform sample at given phase
    float GenerateWaveform(float phase, PulsaretWaveform waveform);

    // Generate envelope value at given phase
    float GenerateEnvelope(float phase, PulsaretEnvelope envelope);

    // Apply wavefolding
    float ApplyFold(float sample);

    // Check burst masking
    bool ShouldEmitPulsar();

    // Fast pseudo-random number generator
    float FastRandom();

    // Sample rate
    float sampleRate_;
    float invSampleRate_;

    // Phase accumulator (0.0 to 1.0 per pulsar period)
    float phase_;
    float phaseIncrement_;

    // Pulsaret phase (0.0 to 1.0 within duty cycle)
    float pulsaretPhase_;

    // Frequencies
    float fundamentalFreq_;
    float formantFreq_;

    // Duty cycle ratio (formant/fundamental)
    float dutyCycle_;

    // Waveform and envelope
    PulsaretWaveform waveform_;
    PulsaretWaveform waveformNext_;
    float waveformMorph_;

    PulsaretEnvelope envelope_;
    PulsaretEnvelope envelopeNext_;
    float envelopeMorph_;

    // Wavefolding
    float foldAmount_;

    // Masking
    MaskingMode maskingMode_;
    int burstCount_;
    int restCount_;
    int burstPosition_;
    float maskingProbability_;
    bool currentPulsarMasked_;

    // State
    bool inPulsaret_;
    float amplitude_;

    // Random state for stochastic masking and noise
    uint32_t randomSeed_;

    // Previous sample for edge smoothing
    float prevSample_;
};

#endif // PULSAR_ENGINE_HPP
