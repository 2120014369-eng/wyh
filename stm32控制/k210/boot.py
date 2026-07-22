import gc
import sensor
import time
import lcd
import ubinascii
import sys

JPEG_QUALITY = 25
RAW_CHUNK_BYTES = 48
CAPTURE_COOLDOWN_MS = 1000
INTER_PACKET_DELAY_MS = 8

# USB CDC device for non-blocking reads
# Fallback: select-based polling
try:
    import uselect as select
except Exception:
    try:
        import select
    except Exception:
        select = None

_lcd_ok = False


def write_line(text):
    """Send a line over USB CDC (to PC)."""
    try:
        sys.stdout.write(text)
        sys.stdout.write("\r\n")
        sys.stdout.flush()
    except Exception:
        pass


def init_camera():
    """Initialize camera sensor. LCD is optional — don't hang if absent."""
    global _lcd_ok
    try:
        lcd.init(freq=15000000)
        _lcd_ok = True
    except Exception:
        _lcd_ok = False

    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)
    sensor.run(1)


def normalize_command(line):
    if not line:
        return ""
    # Strip null bytes (USB CDC padding) and whitespace before normalizing
    line = line.replace("\x00", "")
    return line.strip().upper()


def read_usb_data():
    """Read available USB CDC data without blocking. Returns string."""
    # Method 1: sys.stdin.any() (USB_VCP method, non-blocking)
    try:
        n = sys.stdin.any()
        if n and n > 0:
            data = sys.stdin.read(n)
            if isinstance(data, bytes):
                return data.decode("utf-8", errors="replace")
            return data
        return ""
    except Exception:
        pass

    # Method 2: select-based check
    if select is not None:
        try:
            ready, _, _ = select.select([sys.stdin], [], [], 0)
            if ready:
                data = sys.stdin.read()
                if data:
                    if isinstance(data, bytes):
                        return data.decode("utf-8", errors="replace")
                    return data
        except Exception:
            pass
        return ""

    # Method 3: blind poll — read 1 byte at a time
    try:
        data = sys.stdin.read(1)
        if data:
            if isinstance(data, bytes):
                return data.decode("utf-8", errors="replace")
            return data
        return ""
    except Exception:
        return ""


def drain_stdin():
    """Read and discard all available stdin bytes (clears USB CDC echo flood)."""
    try:
        while sys.stdin.any() > 0:
            sys.stdin.read(64)
    except Exception:
        pass


def split_commands(rx_buffer):
    """Split rx_buffer into commands by newline. Returns (remaining_buffer, commands_list)."""
    commands = []
    while True:
        split_positions = [pos for pos in (rx_buffer.find("\r"), rx_buffer.find("\n")) if pos >= 0]
        if not split_positions:
            break
        split_pos = min(split_positions)
        line = rx_buffer[:split_pos]
        rx_buffer = rx_buffer[split_pos + 1:]
        cmd = normalize_command(line)
        if cmd:
            commands.append(cmd)
    if len(rx_buffer) > 128:
        rx_buffer = ""
    return rx_buffer, commands


def send_snapshot(img):
    gc.collect()
    try:
        jpeg = img.to_jpeg()
    except Exception:
        jpeg = img.compressed(quality=JPEG_QUALITY)

    try:
        jpeg_bytes = jpeg.to_bytes()
    except TypeError:
        jpeg_bytes = jpeg.to_bytes(False)

    if not jpeg_bytes:
        raise RuntimeError("empty jpeg")

    write_line("IMG:BEGIN:%d" % len(jpeg_bytes))
    write_line("K210:JPEG:%d" % len(jpeg_bytes))

    for offset in range(0, len(jpeg_bytes), RAW_CHUNK_BYTES):
        raw_chunk = jpeg_bytes[offset:offset + RAW_CHUNK_BYTES]
        encoded = ubinascii.b2a_base64(raw_chunk).strip()
        try:
            encoded_text = encoded.decode()
        except Exception:
            encoded_text = str(encoded)
        write_line("IMG:DATA:" + encoded_text)
        time.sleep_ms(INTER_PACKET_DELAY_MS)

    write_line("IMG:END")
    gc.collect()


def handle_command(cmd, img, last_capture_ms):
    if cmd == "PING":
        write_line("K210:PONG")
        return last_capture_ms

    if cmd in ("SNAP", "PHOTO"):
        now = time.ticks_ms()
        if time.ticks_diff(now, last_capture_ms) < CAPTURE_COOLDOWN_MS:
            write_line("K210:BUSY")
            return last_capture_ms

        write_line("K210:CAPTURE")
        try:
            send_snapshot(img)
            # Drain echo backlog from USB CDC so main loop isn't slowed down
            drain_stdin()
            return now
        except Exception as exc:
            write_line("K210:ERR:%s" % str(exc))
            gc.collect()
            return last_capture_ms

    write_line("K210:ERR:UNKNOWN")
    return last_capture_ms


def main():
    global _lcd_ok
    init_camera()
    clock = time.clock()
    rx_buffer = ""
    last_capture_ms = time.ticks_ms() - CAPTURE_COOLDOWN_MS

    # Announce startup method
    write_line("K210:READY")
    try:
        if sys.stdin.any() is not None:
            write_line("K210:USB:READY:ANY")
    except Exception:
        if select is not None:
            write_line("K210:USB:READY:SELECT")
        else:
            write_line("K210:USB:READY:POLL")

    while True:
        clock.tick()
        img = sensor.snapshot()

        # Display on LCD only if init succeeded
        if _lcd_ok:
            try:
                lcd.display(img)
            except Exception:
                _lcd_ok = False

        # Read all available USB data (non-blocking)
        data = read_usb_data()
        if data:
            rx_buffer += data
            rx_buffer, commands = split_commands(rx_buffer)
            for cmd in commands:
                # Skip self-echo (USB CDC loopback: our stdout appears on stdin)
                if cmd.startswith("K210:") or cmd.startswith("IMG:") or cmd.startswith("OK:"):
                    continue
                last_capture_ms = handle_command(cmd, img, last_capture_ms)

        time.sleep_ms(10)


main()
