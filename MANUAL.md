# PulsarVersio

## Pulsar Synthesis Oscillator for Versio

PulsarVersio is an oscillator firmware for the Noise Engineering Versio platform implementing **Pulsar Synthesis**, a technique pioneered by composer Curtis Roads. It generates trains of sonic particles called *pulsarets*, creating timbres ranging from vintage electronic sonorities to complex rhythmic textures.

---

## Overview

Pulsar synthesis generates periodic bursts of sound called **pulsars**. Each pulsar consists of a brief waveform (the **pulsaret**) followed by a silent interval. Two independent frequencies control the sound:

- **Fundamental Frequency** — The rate of pulsar emission (pitch)
- **Formant Frequency** — The reciprocal of the pulsaret duration (timbre/resonance)

By varying the ratio between these frequencies, you create timbres similar to a resonant filter swept across an impulse train—but with no actual filter in the circuit. The formant appears naturally in the spectrum based on the duty cycle.

The result is an oscillator that can produce classic electronic pulse tones, formant-like vowel sounds, rhythmic patterns, and everything in between.

---

## Controls

### Knobs

| Knob | Parameter | Description |
|------|-----------|-------------|
| **KNOB 0** | V/Oct | Pitch control with 1V/octave tracking. Sets the fundamental frequency (pulsar emission rate). |
| **KNOB 1** | Formant | Controls the duty cycle ratio. Low = short pulsaret (bright/harsh). High = long pulsaret (mellow/sine-like). |
| **KNOB 2** | Waveform | Morphs between 7 pulsaret waveforms: Sine → Triangle → Saw Up → Saw Down → Square → Pulse → Noise. |
| **KNOB 3** | Envelope | Morphs between 7 pulsaret envelopes: Rectangular → Gaussian → ExpoDec → Linear Decay → Linear Attack → Expo Attack → FOF. |
| **KNOB 4** | Burst/Prob | In Burst mode: burst count (1–8). In Stochastic mode: emission probability (0–100%). |
| **KNOB 5** | Rest/Fold | In Burst mode: rest count (0–7). In Off/Stochastic modes: wavefolding amount. |
| **KNOB 6** | Level | Output volume. |

### Switches

#### Switch 0 (Top) — Masking Mode

| Position | Mode | Description |
|----------|------|-------------|
| **LEFT** | Off | No masking. All pulsars emit. Knob 5 controls wavefolding. |
| **CENTER** | Burst | Burst masking with b:r ratio. Creates rhythmic patterns and subharmonics. |
| **RIGHT** | Stochastic | Random masking based on probability. Creates textural variations. |

#### Switch 1 (Bottom) — Frequency Range

| Position | Base Frequency | Range |
|----------|----------------|-------|
| **LEFT** | 4 Hz | LFO range (for rhythmic/modulation use) |
| **CENTER** | 65.41 Hz (C2) | Bass/sub range |
| **RIGHT** | 261.63 Hz (C4) | Mid range (middle C) |

The V/Oct input (Knob 0) tracks approximately 5 octaves from the selected base frequency.

### Button

**TAP** — Resets the phase. Use this to restart the pulsar train or sync manually to external events.

---

## Inputs & Outputs

### Audio Inputs

- **IN L** — Hard Sync input. Rising zero-crossings reset the pulsar phase, syncing it to an external oscillator. Creates classic sync timbres with the formant character of pulsar synthesis.
- **IN R** — Ring Modulation input. The pulsar output is multiplied by this signal. When unpatched, outputs dry signal. When patched, creates sidebands and metallic tones.

### Audio Outputs

- **OUT L** — Dry pulsar output
- **OUT R** — Ring modulated output (Pulsar × IN R)

### CV Inputs

Each knob has a CV input jack that sums with the knob position (0–5V range).

- **Knob 0 CV** — V/Oct pitch input (calibrated)
- **Knob 1 CV** — Formant modulation
- **Knob 2 CV** — Waveform morph modulation
- **Knob 3 CV** — Envelope morph modulation
- **Knob 4 CV** — Burst count / Probability modulation
- **Knob 5 CV** — Rest count / Fold modulation
- **Knob 6 CV** — Output Level modulation
- **FSU** — Gate input for external reset trigger. Rising edge resets phase.

---

## LED Indicators

| LED | Color | Indicates |
|-----|-------|-----------|
| **LED 0** | Cyan | Pulsaret activity. Bright when pulsaret is sounding, dim during silent interval. |
| **LED 1** | Green | Formant amount. Brightness reflects Knob 1 value. |
| **LED 2** | Orange | Waveform position. Brightness reflects Knob 2 value. |
| **LED 3** | White/Magenta | Output level. White when masking is off, Magenta when masking is active. |

---

## V/Oct Calibration

PulsarVersio supports precise 1V/octave pitch tracking with a calibration routine.

### To Calibrate:

1. Set **both switches to the RIGHT position**
2. Hold the **TAP button** while powering on the module
3. All LEDs will turn white to indicate calibration mode
4. Apply **1V** to the Knob 0 CV input, press TAP
5. Apply **2V** to the Knob 0 CV input, press TAP
6. Apply **3V** to the Knob 0 CV input, press TAP
7. Calibration values are saved to flash memory and persist across power cycles

### Default Calibration

If calibration data becomes corrupted or falls outside valid ranges, the module automatically restores factory defaults.

---

## Algorithm Details

### How Pulsar Synthesis Works

1. Each **pulsar** consists of a **pulsaret** waveform followed by silence
2. The **fundamental frequency** (fp) determines how often pulsars repeat
3. The **formant frequency** (fd) determines the pulsaret duration (duty cycle)
4. The **duty cycle** ratio is fd/fp—when fd > fp, the pulsaret is shorter than the period
5. The spectrum contains a **formant peak** at fd, resembling a bandpass-filtered impulse train

### Pulsaret Waveforms

| Waveform | Character |
|----------|-----------|
| **Sine** | Pure, smooth, classic electronic |
| **Triangle** | Softer harmonics, slightly hollow |
| **Saw Up** | Bright, buzzy, rich harmonics |
| **Saw Down** | Similar to Saw Up with different phase |
| **Square** | Hollow, odd harmonics only |
| **Pulse** | Nasal, narrow duty, clavinet-like |
| **Noise** | Unpitched, textural, percussive |

### Pulsaret Envelopes

The envelope shapes the pulsaret amplitude over its duration, strongly affecting the spectrum:

| Envelope | Spectrum Effect |
|----------|-----------------|
| **Rectangular** | Broad spectrum with strong peaks and nulls (sinc function) |
| **Gaussian** | Narrow formant band, concentrated energy |
| **ExpoDec** | Smooth spectrum, percussive attack |
| **Linear Decay** | Moderate smoothing |
| **Linear Attack** | Reversed percussive character |
| **Expo Attack** | Swelling, reversed percussive |
| **FOF** | Classic formant synthesis envelope (sharp attack, exponential decay) |

### Masking Modes

**Burst Masking**
- Creates patterns like "play 4, skip 2" (b:r = 4:2)
- When fundamental is audio-rate, creates **subharmonics** dividing the pitch
- Ratio of b:(b+r) determines the subharmonic division

**Stochastic Masking**
- Random probability of each pulsar emitting
- High probability (>90%): subtle analog-like intermittency
- Medium probability (50–80%): textural variation
- Low probability (<50%): sparse, pointillistic

### Wavefolding

West-coast style wavefolding adds harmonics by "folding" the waveform back on itself when it exceeds a threshold. Creates bright, complex timbres especially effective with sine pulsarets.

---

## Patch Ideas

### Classic Pulse Tone
- Frequency Range: Mid (right switch)
- Formant: 10–12 o'clock (moderate duty)
- Waveform: Sine
- Envelope: Rectangular
- Masking: Off
- Result: Classic electronic music pulse tone, similar to vintage equipment

### Vocal Formants
- Frequency Range: Mid (right switch)
- Formant: Modulate with slow LFO (9–3 o'clock sweep)
- Waveform: Sine
- Envelope: FOF or Gaussian
- Masking: Off
- Result: Vowel-like sounds, speech-adjacent timbres. Modulating formant creates "talking" effects

### Rhythmic Sub-Bass
- Frequency Range: LFO (left switch) at ~8 Hz
- Formant: Low (bright, punchy)
- Waveform: Sine or Square
- Envelope: ExpoDec
- Masking: Off
- Wavefolding: Moderate
- Result: Rhythmic pulse pattern, great for techno kicks and bass

### Subharmonic Generator
- Frequency Range: Mid or High
- Masking: Burst (center switch)
- Burst (Knob 4): 2–3
- Rest (Knob 5): 1–2
- Result: Pitch divided by (burst+rest). B:R of 2:1 = 1/3 subharmonic. Creates organ-like sub-octaves

### Harsh Digital Texture
- Frequency Range: High (right switch)
- Formant: Low (short duty, 8–9 o'clock)
- Waveform: Noise or Pulse
- Envelope: Rectangular
- Masking: Stochastic, probability ~70%
- Result: Aggressive digital noise, glitchy textures

### Evolving Pad
- Frequency Range: Mid
- Formant: CV modulated with slow random/S&H
- Waveform: Morph slowly between Sine and Triangle
- Envelope: Gaussian
- Masking: Stochastic, probability ~95%
- Result: Subtly shifting, organic pad sound

### Resonant Sweep
- Frequency Range: Mid or High
- Formant: CV controlled by envelope or LFO (full range sweep)
- Waveform: Sine
- Envelope: Gaussian or ExpoDec
- Masking: Off
- Result: Filter-sweep-like effect without a filter. The formant peak moves through the spectrum

### Burst Rhythm Pattern
- Frequency Range: LFO (left switch) at ~4–6 Hz
- Masking: Burst (center switch)
- Burst: 3
- Rest: 1
- Waveform: Square
- Envelope: ExpoDec
- Result: Rhythmic pattern of 3 hits then 1 rest, synced to fundamental

### Metallic Ring Mod
- Frequency Range: High
- Formant: Medium
- Waveform: Sine
- Envelope: Gaussian
- Masking: Off
- Patch: Send a different oscillator into IN R
- Result: Complex sidebands, bell-like tones, metallic textures

### Sync Lead
- Frequency Range: Mid
- Formant: Medium-Low (brighter)
- Waveform: Saw Up
- Envelope: ExpoDec
- Patch: Send master oscillator into IN L (hard sync)
- Result: Classic sync lead sound with formant character. Sweep master pitch for tearing sync effect

### Pointillistic Texture
- Frequency Range: High
- Masking: Stochastic, probability ~20–40%
- Formant: Random CV modulation
- Waveform: Various (modulate)
- Envelope: Various (modulate)
- Result: Sparse, scattered sonic particles. Excellent for experimental/electroacoustic music

### LFO Modulation Source
- Frequency Range: LFO (left switch)
- Fundamental: ~1–10 Hz
- Formant: High (full duty = smoother LFO)
- Waveform: Choose shape for LFO waveshape
- Masking: Off
- Output: Use OUT L as complex LFO
- Result: Waveshaped LFO with adjustable duty cycle

---

## Tips & Techniques

### Formant vs Fundamental Relationship
- **Formant > Fundamental**: Short duty cycle, bright/harsh timbre
- **Formant = Fundamental**: 100% duty cycle, continuous waveform (like a normal oscillator)
- **Formant < Fundamental**: Pulsaret gets cut off, creating edge artifacts

### Creating Movement
- Modulate **Formant** (Knob 1) with an LFO for filter-sweep-like effects
- Modulate **Waveform** (Knob 2) slowly for evolving timbre
- Modulate **Envelope** (Knob 3) for changing attack/decay character
- Use **Stochastic masking** at high probability for subtle analog-like variation

### Subharmonic Math
In Burst mode, the effective pitch is divided by (burst + rest):
- B:R = 1:1 → Pitch ÷ 2 (one octave down)
- B:R = 2:1 → Pitch ÷ 3
- B:R = 3:1 → Pitch ÷ 4 (two octaves down)
- B:R = 1:2 → Pitch ÷ 3 (different phase pattern)

---

## Specifications

| Parameter | Value |
|-----------|-------|
| Sample Rate | 48 kHz |
| Waveform Types | 7 (with morphing) |
| Envelope Types | 7 (with morphing) |
| Frequency Range | ~0.5 Hz – 10 kHz |
| V/Oct Tracking | ~5 octaves |
| Audio Output | Stereo (L=dry, R=ring mod) |
| Flash Usage | ~90 KB (68%) |
| RAM Usage | ~16 KB (3%) |

---

## Credits

PulsarVersio implements **Pulsar Synthesis** as described by **Curtis Roads** in his paper "Sound Composition with Pulsars" (2001) and book "Microsound" (2001). The technique has roots in the filtered impulse generation methods of the classic electronic music studios of Karlheinz Stockhausen and Gottfried Michael Koenig.

Firmware developed for the **Noise Engineering Versio** platform.

---

*"Pulsar synthesis melds established principles within a new paradigm... it generates electronic pulses and pitched tones similar to those produced by analog instruments such as the Ondioline and the Hohner Elektronium."*
— Curtis Roads
