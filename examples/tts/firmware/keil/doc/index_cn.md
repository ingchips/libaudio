# TTS 演示项目

[English](index.md)

重要提示：此项目需要[外部闪存](https://ingchips.github.io/blog/2024-02-05-external-flash/)。

语音样例：

> 欢迎使用 ING916 桃芯语音合成。支付宝收款 916.80 元。

<audio controls> <source src="example_data1.wav">
您的浏览器不支持 audio 元素。
</audio>

> 京 A123456789，祝您一路顺风。

<audio controls> <source src="example_data2.wav">
您的浏览器不支持 audio 元素。
</audio>

合成的波形将通过 UART 或 I2S 发送，具体由 WAVE_SINK 选择：

|WAVE_SINK  | 输出                | 备注                             |
|:---------:|:------------------:|:---------------------------------|
| 0         | UART (默认)         |Python 脚本 `tts_demo.py` 将播放波形。 |
| 1         | I2S                |需要使用带有 I2S 的音频 DAC 进行播放。 |

当选择 I2S 作为 `WAVE_SINK` 时，需要额外的带有 I2S 的音频 DAC 进行播放。以 PCM5102A 为例：

![](pcm5102a.png)

请使用 Cube 找出哪些引脚用作 I2S。

## 测试

1. 下载到连接了足够大外部闪存的 ING916 开发板；

1. 运行 [`tts_demo.py`](../../../../../tools/tts_demo.py)。假设使用 COM1：

    ```sh
    python tts_demo.py COM1
    ```
