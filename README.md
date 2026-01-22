# PulsarVersio

**Pulsar Synthesis Oscillator for Noise Engineering Versio**

A firmware implementing Curtis Roads' Pulsar Synthesis technique. Generates trains of sonic particles (pulsarets) with independent control over fundamental frequency (pitch) and formant frequency (timbre).

## Features

- **7 pulsaret waveforms** with smooth morphing (sine → noise)
- **7 envelope types** (rectangular, gaussian, FOF, etc.)
- **Burst masking** for subharmonics and rhythmic patterns
- **Stochastic masking** for textural variation
- **West-coast wavefolding**
- **Hard sync** and **ring modulation** inputs
- **V/Oct tracking** with calibration

## Controls

```
KNOB 0: V/Oct Pitch          KNOB 4: Burst Count / Probability
KNOB 1: Formant (duty cycle) KNOB 5: Rest Count / Fold
KNOB 2: Waveform Shape       KNOB 6: Output Level
KNOB 3: Envelope Type

SW 0: Masking Mode (Off / Burst / Stochastic)
SW 1: Frequency Range (LFO / Low / Mid)

IN L: Hard Sync    OUT L: Dry Output
IN R: Ring Mod     OUT R: Ring Mod Output
FSU: Reset Gate
```

## Building

```bash
make
make program-dfu  # Flash via USB (module in bootloader mode)
```

See [BUILD.md](BUILD.md) for prerequisites and detailed instructions.

## Documentation

- [MANUAL.md](MANUAL.md) — Full user guide with patch ideas
- [BUILD.md](BUILD.md) — Build and flashing instructions

## Credits

Based on Pulsar Synthesis by **Curtis Roads** ("Sound Composition with Pulsars", 2001).

Built for the **Noise Engineering Versio** platform using libDaisy.

---

*"Much can be gained from the harvest of synthetic waveforms. Of special interest are those hybrids that crossbreed the richness of familiar sounds with unusual overtones."*
— Curtis Roads
