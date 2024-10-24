import argparse

try:
    import serial
except:
    print("`pyserial` is missing! Install it with `pip`:")
    print("pip install pyserial")

try:
    import simpleaudio as sa
except:
    print("`simpleaudio` is missing! Install it with `pip`:")
    print("pip install simpleaudio")

def open_serial(port, baudrate = 115200, parity = serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS):
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = baudrate
    ser.parity = parity
    ser.stopbits = stopbits
    ser.bytesize = bytesize
    ser.timeout = 0.5
    ser.open()
    if not ser.is_open:
        raise Exception("can't open serial")
    return ser

def read_samples(ser: serial.Serial) -> bytes:
    acc = b''
    cnt = 0
    def running():
        nonlocal cnt
        TEMPLATE = '-\\|/'
        ch = TEMPLATE[cnt]
        print(ch, end='\r')
        cnt = cnt + 1 if cnt < len(TEMPLATE) - 1 else 0

    while True:
        block = ser.read(64)
        if len(block) < 1:
            print(f'received {len(acc)} bytes.')
            return acc
        acc = acc + block
        running()

def play(pcm_samples) -> None:
    if len(pcm_samples) < 1: return
    play_obj = sa.play_buffer(pcm_samples, 1, 2, 16000)
    play_obj.wait_done()

if __name__ == "__main__":

    parser = argparse.ArgumentParser("tts_demo")
    parser.add_argument("port", type=str)
    parser.add_argument("-b", type=int, default=115200)
    args = parser.parse_args()

    ser = open_serial(args.port, baudrate=args.b)
    ser.reset_input_buffer()
    ser.read_all()

    print("""
========== TTS Demo ==========
Use !speed to change speed.
For example: !1.5
==============================
""")

    while True:
        b = (input("> ") + '\n').encode('utf-8')
        if len(b) <= 1: break
        ser.write(b)
        if b[0] == ord('!'): continue
        synth_samples = read_samples(ser)
        play(synth_samples)
