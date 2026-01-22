#include "PulsarEngine.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static constexpr float TWO_PI = 2.0f * M_PI;

void PulsarEngine::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    invSampleRate_ = 1.0f / sampleRate;

    phase_ = 0.0f;
    pulsaretPhase_ = 0.0f;
    phaseIncrement_ = 0.0f;

    fundamentalFreq_ = 220.0f;
    formantFreq_ = 440.0f;
    dutyCycle_ = 0.5f;

    waveform_ = PulsaretWaveform::SINE;
    waveformNext_ = PulsaretWaveform::SINE;
    waveformMorph_ = 0.0f;

    envelope_ = PulsaretEnvelope::GAUSSIAN;
    envelopeNext_ = PulsaretEnvelope::GAUSSIAN;
    envelopeMorph_ = 0.0f;

    foldAmount_ = 0.0f;

    maskingMode_ = MaskingMode::OFF;
    burstCount_ = 4;
    restCount_ = 0;
    burstPosition_ = 0;
    maskingProbability_ = 1.0f;
    currentPulsarMasked_ = false;

    inPulsaret_ = true;
    amplitude_ = 1.0f;

    randomSeed_ = 12345;
    prevSample_ = 0.0f;

    SetFrequency(fundamentalFreq_);
}

void PulsarEngine::Reset() {
    phase_ = 0.0f;
    pulsaretPhase_ = 0.0f;
    burstPosition_ = 0;
    currentPulsarMasked_ = false;
    inPulsaret_ = true;
    prevSample_ = 0.0f;
}

void PulsarEngine::Sync() {
    phase_ = 0.0f;
    pulsaretPhase_ = 0.0f;
    inPulsaret_ = true;
    // Check masking for new pulsar
    currentPulsarMasked_ = !ShouldEmitPulsar();
}

float PulsarEngine::Process() {
    float sample = 0.0f;

    // Calculate duty cycle threshold
    // dutyCycle_ represents the fraction of the period that is the pulsaret
    float dutyThreshold = dutyCycle_;

    // Are we in the pulsaret portion of the period?
    inPulsaret_ = (phase_ < dutyThreshold);

    if (inPulsaret_ && !currentPulsarMasked_) {
        // Calculate pulsaret phase (0 to 1 within the duty cycle)
        pulsaretPhase_ = phase_ / dutyThreshold;

        // Generate waveform with morphing
        float waveA = GenerateWaveform(pulsaretPhase_, waveform_);
        float waveB = GenerateWaveform(pulsaretPhase_, waveformNext_);
        float waveformSample = waveA + (waveB - waveA) * waveformMorph_;

        // Generate envelope with morphing
        float envA = GenerateEnvelope(pulsaretPhase_, envelope_);
        float envB = GenerateEnvelope(pulsaretPhase_, envelopeNext_);
        float envelopeSample = envA + (envB - envA) * envelopeMorph_;

        // Apply envelope to waveform
        sample = waveformSample * envelopeSample;

        // Apply wavefolding
        if (foldAmount_ > 0.001f) {
            sample = ApplyFold(sample);
        }

        // Apply amplitude
        sample *= amplitude_;
    }

    // Advance phase
    float prevPhase = phase_;
    phase_ += phaseIncrement_;

    // Check for period wrap
    if (phase_ >= 1.0f) {
        phase_ -= 1.0f;

        // Update burst position for masking
        burstPosition_++;
        if (burstPosition_ >= (burstCount_ + restCount_)) {
            burstPosition_ = 0;
        }

        // Check masking for new pulsar
        currentPulsarMasked_ = !ShouldEmitPulsar();
    }

    // Smooth transitions at pulsaret boundaries to reduce clicks
    if (prevPhase < dutyThreshold && phase_ >= dutyThreshold) {
        // Transitioning from pulsaret to silence - apply small fade
        sample = prevSample_ * 0.5f;
    }

    prevSample_ = sample;
    return sample;
}

void PulsarEngine::SetFrequency(float freq) {
    fundamentalFreq_ = fmaxf(0.1f, fminf(freq, sampleRate_ * 0.45f));
    phaseIncrement_ = fundamentalFreq_ * invSampleRate_;

    // Update duty cycle based on formant/fundamental ratio
    if (formantFreq_ > 0.1f) {
        dutyCycle_ = fminf(1.0f, fundamentalFreq_ / formantFreq_);
    }
}

void PulsarEngine::SetFormantFrequency(float freq) {
    formantFreq_ = fmaxf(0.1f, freq);

    // Update duty cycle: duty = fundamental / formant
    // Higher formant = shorter duty cycle = brighter sound
    if (formantFreq_ > 0.1f) {
        dutyCycle_ = fminf(1.0f, fundamentalFreq_ / formantFreq_);
    }
}

void PulsarEngine::SetFormantRatio(float ratio) {
    // ratio 0.0 = very short duty (high formant), 1.0 = full duty (low formant)
    ratio = fmaxf(0.01f, fminf(1.0f, ratio));
    dutyCycle_ = ratio;

    // Calculate equivalent formant frequency
    if (ratio > 0.01f) {
        formantFreq_ = fundamentalFreq_ / ratio;
    }
}

void PulsarEngine::SetWaveform(PulsaretWaveform waveform) {
    waveform_ = waveform;
    waveformNext_ = waveform;
    waveformMorph_ = 0.0f;
}

void PulsarEngine::SetWaveformMorph(float morphValue) {
    morphValue = fmaxf(0.0f, fminf(6.0f, morphValue));

    int idx = static_cast<int>(morphValue);
    waveformMorph_ = morphValue - static_cast<float>(idx);

    waveform_ = static_cast<PulsaretWaveform>(idx);
    waveformNext_ = static_cast<PulsaretWaveform>(fminf(6.0f, static_cast<float>(idx + 1)));
}

void PulsarEngine::SetEnvelope(PulsaretEnvelope envelope) {
    envelope_ = envelope;
    envelopeNext_ = envelope;
    envelopeMorph_ = 0.0f;
}

void PulsarEngine::SetEnvelopeMorph(float morphValue) {
    morphValue = fmaxf(0.0f, fminf(6.0f, morphValue));

    int idx = static_cast<int>(morphValue);
    envelopeMorph_ = morphValue - static_cast<float>(idx);

    envelope_ = static_cast<PulsaretEnvelope>(idx);
    envelopeNext_ = static_cast<PulsaretEnvelope>(fminf(6.0f, static_cast<float>(idx + 1)));
}

void PulsarEngine::SetFold(float amount) {
    foldAmount_ = fmaxf(0.0f, fminf(1.0f, amount));
}

void PulsarEngine::SetBurstRatio(int burst, int rest) {
    burstCount_ = (burst < 1) ? 1 : ((burst > 16) ? 16 : burst);
    restCount_ = (rest < 0) ? 0 : ((rest > 16) ? 16 : rest);
}

void PulsarEngine::SetMaskingProbability(float probability) {
    maskingProbability_ = fmaxf(0.0f, fminf(1.0f, probability));
}

void PulsarEngine::SetMaskingMode(MaskingMode mode) {
    maskingMode_ = mode;
}

void PulsarEngine::SetAmplitude(float amp) {
    amplitude_ = fmaxf(0.0f, fminf(1.0f, amp));
}

float PulsarEngine::GenerateWaveform(float phase, PulsaretWaveform waveform) {
    float sample = 0.0f;

    switch (waveform) {
        case PulsaretWaveform::SINE:
            sample = sinf(phase * TWO_PI);
            break;

        case PulsaretWaveform::TRIANGLE:
            if (phase < 0.25f) {
                sample = phase * 4.0f;
            } else if (phase < 0.75f) {
                sample = 1.0f - (phase - 0.25f) * 4.0f;
            } else {
                sample = (phase - 0.75f) * 4.0f - 1.0f;
            }
            break;

        case PulsaretWaveform::SAW_UP:
            sample = 2.0f * phase - 1.0f;
            break;

        case PulsaretWaveform::SAW_DOWN:
            sample = 1.0f - 2.0f * phase;
            break;

        case PulsaretWaveform::SQUARE:
            sample = (phase < 0.5f) ? 1.0f : -1.0f;
            break;

        case PulsaretWaveform::PULSE:
            // Narrow pulse (25% duty)
            sample = (phase < 0.25f) ? 1.0f : -0.33f;
            break;

        case PulsaretWaveform::NOISE:
            sample = FastRandom() * 2.0f - 1.0f;
            break;
    }

    return sample;
}

float PulsarEngine::GenerateEnvelope(float phase, PulsaretEnvelope envelope) {
    float env = 1.0f;

    switch (envelope) {
        case PulsaretEnvelope::RECTANGULAR:
            env = 1.0f;
            break;

        case PulsaretEnvelope::GAUSSIAN: {
            // Gaussian centered at 0.5
            float x = (phase - 0.5f) * 3.0f;
            env = expf(-x * x);
            break;
        }

        case PulsaretEnvelope::EXPODEC: {
            // Exponential decay
            float decay = 4.0f;
            env = expf(-phase * decay);
            break;
        }

        case PulsaretEnvelope::LINEAR_DECAY:
            env = 1.0f - phase;
            break;

        case PulsaretEnvelope::LINEAR_ATTACK:
            env = phase;
            break;

        case PulsaretEnvelope::EXPO_ATTACK: {
            // Exponential attack
            float attack = 4.0f;
            env = 1.0f - expf(-(phase) * attack);
            break;
        }

        case PulsaretEnvelope::FOF: {
            // FOF-style: sharp attack, exponential decay
            float attackTime = 0.1f;
            if (phase < attackTime) {
                env = phase / attackTime;
            } else {
                float decay = 3.0f;
                env = expf(-(phase - attackTime) * decay);
            }
            break;
        }
    }

    return env;
}

float PulsarEngine::ApplyFold(float sample) {
    // West-coast style wavefolding
    float gain = 1.0f + foldAmount_ * 8.0f;
    sample *= gain;

    // Fold using sine function for smooth folding
    while (sample > 1.0f || sample < -1.0f) {
        if (sample > 1.0f) {
            sample = 2.0f - sample;
        }
        if (sample < -1.0f) {
            sample = -2.0f - sample;
        }
    }

    return sample;
}

bool PulsarEngine::ShouldEmitPulsar() {
    switch (maskingMode_) {
        case MaskingMode::OFF:
            return true;

        case MaskingMode::BURST:
            // Emit if we're in the burst portion of the cycle
            return (burstPosition_ < burstCount_);

        case MaskingMode::STOCHASTIC:
            return (FastRandom() < maskingProbability_);
    }

    return true;
}

float PulsarEngine::FastRandom() {
    // Linear congruential generator
    randomSeed_ = randomSeed_ * 1664525u + 1013904223u;
    return static_cast<float>(randomSeed_) / 4294967296.0f;
}
