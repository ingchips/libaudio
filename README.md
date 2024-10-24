# Audio Library

An audio library for ING916, which contains:

* [ADPCM](https://en.wikipedia.org/wiki/Adaptive_differential_pulse-code_modulation) codec;
* De-noise module;
* [SBC](https://en.wikipedia.org/wiki/SBC_(codec)) codec;
* [Opus](https://en.wikipedia.org/wiki/Opus_(audio_format)) encoder;
* [AMR-WB](https://en.wikipedia.org/wiki/AMR-WB) codec;
* Text-to-Speech (TTS) engine;
* Speech stretch.

For more information, please checkout [Application Note](https://ingchips.github.io/application-notes/an_libaudio_cn/).

## Acknowledgement

These libraries, codes, and/or data from third parties are used:

* [libsbc](https://github.com/google/libsbc) ([License](SBC-LICENSE))

* [Opus](https://opus-codec.org/) ([License](OPUS-COPYING))

* AMR-WB codec

    * [TS 26.173](https://portal.3gpp.org/desktopmodules/Specifications/SpecificationDetails.aspx?specificationId=1421)
    * PacketVideo AMR-WB decoder ([License](AMR-WB-DEC-LICENSE))
    * VisualOn AMR-WB encoder ([License](AMR-WB-ENC-LICENSE))

* [Audio Stretch](https://github.com/dbry/audio-stretch) ([License](STRETCH-LICENSE))

* [IK Analysis](https://github.com/infinilabs/analysis-ik) ([License](IK-LICENSE))