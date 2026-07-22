#!/usr/bin/env python3
"""
@file    test_serial.py
@brief   全硬件模拟串口测试脚本 — 不需要真实硬件。

支持两种模式：
  - TCP 模式：连接 virtual_stm32.py，验证命令响应 + 电机引脚状态
  - 串口模式：连接真实 STM32 硬件，验证命令响应

TCP 模式下会通过状态端口（port+1）验证电机 GPIO 引脚状态，
实现完整的固件行为测试，无需任何真实硬件。

Usage:
  # TCP 虚拟测试（推荐，无需硬件）
  python test_serial.py --tcp

  # 串口硬件测试
  python test_serial.py COM3

  # 列出串口
  python test_serial.py --list

Requirements:
  pip install pyserial
"""

import sys
import time
import argparse
import socket

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("ERROR: pyserial not installed. Run: pip install pyserial")
    sys.exit(1)


# =========================================================================
# Expected motor pin states for each command
# (PB8, PB9, PB12, PB13, PB14, PB15) — matching mycar.c logic
# =========================================================================
MOTOR_EXPECTED = {
    b"go\n":   (1, 1, 0, 1, 0, 1),
    b"stop\n": (0, 0, 0, 0, 0, 0),
    b"F\n":    (1, 1, 0, 1, 0, 1),
    b"B\n":    (1, 1, 1, 0, 1, 0),
    b"L\n":    (1, 1, 1, 0, 0, 1),
    b"R\n":    (1, 1, 0, 1, 1, 0),
    b"S\n":    (0, 0, 0, 0, 0, 0),
    b"F":      (1, 1, 0, 1, 0, 1),
    b"B":      (1, 1, 1, 0, 1, 0),
    b"L":      (1, 1, 1, 0, 0, 1),
    b"R":      (1, 1, 0, 1, 1, 0),
    b"S":      (0, 0, 0, 0, 0, 0),
}

MOTOR_PIN_NAMES = ["PB8", "PB9", "PB12", "PB13", "PB14", "PB15"]


# =========================================================================
# Test cases: (label, command, expected_response_prefix, timeout_ms)
# =========================================================================
USB_TEST_CASES = [
    ("Forward (go)",     b"go\n",   b"OK: go",   500),
    ("Stop (stop)",      b"stop\n", b"OK: stop", 500),
    ("Forward (F)",      b"F\n",    b"OK: F",    500),
    ("Backward (B)",     b"B\n",    b"OK: B",    500),
    ("Left (L)",         b"L\n",    b"OK: L",    500),
    ("Right (R)",        b"R\n",    b"OK: R",    500),
    ("Stop (S)",         b"S\n",    b"OK: S",    500),
    ("Unknown cmd",      b"xyz\n",  b"Error",    500),
]

USB_BT_STYLE = [
    ("USB:BT-F", b"F\n",  b"OK: F",  500),
    ("USB:BT-B", b"B\n",  b"OK: B",  500),
    ("USB:BT-L", b"L\n",  b"OK: L",  500),
    ("USB:BT-R", b"R\n",  b"OK: R",  500),
    ("USB:BT-S", b"S\n",  b"OK: S",  500),
]

RAW_DIRECTION_CASES = [
    ("Raw Forward (F)",  b"F", b"OK: F", 500),
    ("Raw Backward (B)", b"B", b"OK: B", 500),
    ("Raw Left (L)",     b"L", b"OK: L", 500),
    ("Raw Right (R)",    b"R", b"OK: R", 500),
    ("Raw Stop (S)",     b"S", b"OK: S", 500),
]

EXTENDED_USB_CASES = [
    ("Track on",            b"track\n",     [b"MODE:TRACKING", b"OK: track", b"SPEED:", b"DIST:", b"TRACK:"],      {"mode": "TRACKING"},   600),
    ("Track off",           b"track_off\n", [b"MODE:MANUAL", b"OK: track off"],    {"mode": "MANUAL"},     600),
    ("Hold",                b"hold\n",      [b"MODE:WAIT_USER", b"OK: hold"],      {"mode": "WAIT_USER"},  600),
    ("Continue from wait",  b"continue\n",  [b"MODE:TRACKING", b"OK: continue", b"SPEED:", b"DIST:", b"TRACK:"],   {"mode": "TRACKING"},   600),
    ("Status query",        b"status\n",    [b"MODE:", b"SPEED:", b"DIST:", b"TRACK:", b"BT_RX_OVF:", b"OK: status"], None,    600),
    ("Set speed",           b"V45\n",       [b"MODE:MANUAL", b"OK: V45"],          {"mode": "MANUAL", "speed": 45}, 600),
    ("Bad speed",           b"V200\n",      [b"Error: speed 0-100"],               None,                    600),
]

SNAP_TEST_CASES = [
    ("K210 ping obsolete",  b"kping\n",     [b"Error: K210 uses USB"], 800),
    ("Snapshot obsolete",   b"snap\n",      [b"Error: K210 uses USB"], 800),
    ("Obstacle event",      b"obs\n",       [b"MODE:OBSTACLE_STOP", b"OBS:120mm"], 800),
]

NEGATIVE_USB_CASES = [
    ("Continue without wait", b"continue\n", [b"Error: no wait state"], 600),
]


# =========================================================================
# TCP serial adapter (duck-typed for serial.Serial)
# =========================================================================
class TcpSerial:
    """Duck-typed serial port that connects over TCP.

    Provides the same ``read()``/``write()``/``timeout`` interface as
    ``serial.Serial``, so all test functions work unchanged over TCP.
    """

    def __init__(self, host="127.0.0.1", port=54321, timeout=0.5):
        self.host = host
        self.port = port
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.settimeout(timeout)
        self._socket.connect((host, port))
        self._timeout = timeout
        self._read_buf = b""

    @property
    def timeout(self):
        return self._timeout

    @timeout.setter
    def timeout(self, value):
        self._timeout = value
        self._socket.settimeout(value)

    def write(self, data):
        self._socket.sendall(data)

    def read(self, size=1):
        if self._read_buf:
            chunk = self._read_buf[:size]
            self._read_buf = self._read_buf[size:]
            return chunk
        try:
            return self._socket.recv(size)
        except socket.timeout:
            return b""
        except (ConnectionError, OSError):
            return b""

    def close(self):
        try:
            self._socket.close()
        except OSError:
            pass


def connect_tcp(addr):
    """Parse ``host:port`` and return a TcpSerial instance."""
    if ":" in addr:
        host, port_str = addr.rsplit(":", 1)
        port = int(port_str)
    else:
        host = addr
        port = 54321
    print(f"Connecting to TCP {host}:{port}...")
    try:
        tcp = TcpSerial(host, port, timeout=0.5)
        print(f"Connected to {host}:{port} (virtual STM32)")
        return tcp
    except (ConnectionRefusedError, OSError) as e:
        print(f"ERROR: Cannot connect to {host}:{port}: {e}")
        print("  Is virtual_stm32.py running?")
        print("  Start it: python virtual_stm32.py")
        sys.exit(1)


# =========================================================================
# Motor state verification (TCP mode only)
# =========================================================================

def get_motor_state(tcp_host="127.0.0.1", tcp_port=54321):
    """Query the virtual STM32's motor state via the state port (port+1).

    Returns a dict like {"PB8": 1, "PB9": 0, ...} or None on failure.
    """
    state_port = tcp_port + 1
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(3.0)
        s.connect((tcp_host, state_port))
        data = s.recv(4096)
        s.close()
    except (ConnectionRefusedError, OSError, socket.timeout):
        return None

    text = data.decode("utf-8", errors="replace")
    result = {}
    for line in text.split("\n"):
        line = line.strip()
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()
        try:
            result[key] = int(value)
        except ValueError:
            result[key] = value
    return result


def verify_motor_state(state, expected):
    """Compare motor state dict with expected tuple (PB8..PB15).

    Returns (ok: bool, details: str).
    """
    if state is None:
        return False, "  [MOTOR] !! state port unreachable"

    all_ok = True
    details = []
    for i, name in enumerate(MOTOR_PIN_NAMES):
        actual = state.get(name)
        exp = expected[i]
        if actual != exp:
            all_ok = False
            details.append(f"{name}={actual}≠{exp}")
        else:
            details.append(f"{name}={actual}")

    pin_str = " ".join(details)
    if all_ok:
        return True, f"  [MOTOR] OK {pin_str}"
    else:
        return False, f"  [MOTOR] MISMATCH MISMATCH: {pin_str}"


def verify_virtual_fields(state, expected_fields):
    if not expected_fields:
        return True, ""
    if state is None:
        return False, "  [STATE] !! state port unreachable"

    failures = []
    details = []
    for key, expected in expected_fields.items():
        actual = state.get(key)
        details.append(f"{key}={actual}")
        if actual != expected:
            failures.append(f"{key}={actual}≠{expected}")

    if failures:
        return False, "  [STATE] MISMATCH " + " ".join(failures)
    return True, "  [STATE] OK " + " ".join(details)


# =========================================================================
# Serial helpers
# =========================================================================

def find_serial_port():
    """Auto-detect the STM32 serial port."""
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None

    # First, try STM32 or USB-serial specific ports
    for p in ports:
        desc = (p.description + " " + (p.manufacturer or "") + " " +
                (p.product or "")).lower()
        if any(kw in desc for kw in ["stm32", "st link", "usb serial",
                                      "ch340", "cp210", "ft232", "arduino"]):
            return p.device

    # Fallback: any non-Bluetooth port
    for p in ports:
        if "bluetooth" not in p.description.lower():
            return p.device

    return ports[0].device


def list_ports():
    """List all available serial ports."""
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return

    print(f"{'Port':<12} {'Description':<40} {'HWID':<30}")
    print("-" * 82)
    for p in ports:
        print(f"{p.device:<12} {p.description:<40} {p.hwid:<30}")


def clear_buffer(ser, timeout=0.2):
    """Read any pending data."""
    ser.timeout = timeout
    while True:
        data = ser.read(256)
        if not data:
            break


def read_response_window(ser, total_timeout_ms=800, idle_timeout_ms=120):
    deadline = time.time() + total_timeout_ms / 1000.0
    last_data_time = None
    chunks = []
    ser.timeout = max(0.05, idle_timeout_ms / 1000.0)

    while time.time() < deadline:
        data = ser.read(1024)
        now = time.time()
        if data:
            chunks.append(data)
            last_data_time = now
            continue
        if last_data_time is not None and (now - last_data_time) * 1000.0 >= idle_timeout_ms:
            break

    return b"".join(chunks)


def run_test(ser, label, cmd, expected_prefix, timeout_ms):
    """Run a single test case, return (ok, response_str)."""
    clear_buffer(ser, 0.1)

    ser.write(cmd)
    time.sleep(min(timeout_ms, 150) / 1000.0)

    response = read_response_window(ser, total_timeout_ms=timeout_ms)

    resp_str = response.decode('utf-8', errors='replace').strip()
    if expected_prefix in response:
        return True, resp_str
    else:
        return False, resp_str


def run_contains_test(ser, cmd, expected_fragments, timeout_ms):
    clear_buffer(ser, 0.1)
    ser.write(cmd)
    time.sleep(min(timeout_ms, 150) / 1000.0)
    response = read_response_window(ser, total_timeout_ms=timeout_ms)
    resp_str = response.decode('utf-8', errors='replace').strip()
    ok = all(fragment in response for fragment in expected_fragments)
    return ok, resp_str


def run_test_with_motor(ser, label, cmd, expected_prefix, timeout_ms,
                        tcp_host=None, tcp_port=None):
    """Run test and also verify motor state (TCP mode only)."""
    # Test response
    ok_resp, resp = run_test(ser, label, cmd, expected_prefix, timeout_ms)

    # Test motor state if TCP mode and we have expected values
    motor_ok = True
    motor_detail = ""
    if tcp_host is not None and cmd in MOTOR_EXPECTED:
        state = get_motor_state(tcp_host, tcp_port)
        motor_ok, motor_detail = verify_motor_state(
            state, MOTOR_EXPECTED[cmd])

    return ok_resp and motor_ok, resp, motor_detail


def run_extended_case(ser, cmd, expected_fragments, timeout_ms, expected_state=None,
                      tcp_host=None, tcp_port=None):
    ok_resp, resp = run_contains_test(ser, cmd, expected_fragments, timeout_ms)
    state_info = ""
    state_ok = True
    if tcp_host is not None and expected_state is not None:
        state = get_motor_state(tcp_host, tcp_port)
        state_ok, state_info = verify_virtual_fields(state, expected_state)
    return ok_resp and state_ok, resp, state_info


# =========================================================================
# Test runner
# =========================================================================

def run_all_tests(ser, is_tcp=False, tcp_host=None, tcp_port=None):
    """Run the full test suite. Returns True if all passed."""
    passed = 0
    failed = 0

    print("\n" + "=" * 60)
    print(" USB Serial Command Tests")
    print("=" * 60)

    for label, cmd, expected, timeout in USB_TEST_CASES:
        if is_tcp:
            ok, resp, motor_info = run_test_with_motor(
                ser, label, cmd, expected, timeout, tcp_host, tcp_port)
        else:
            ok, resp = run_test(ser, label, cmd, expected, timeout)
            motor_info = ""

        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<8}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\" (expected {expected!r})")
        if motor_info:
            print(motor_info)

    print("\n" + "=" * 60)
    print(" USB Bluetooth-Style Commands (via USB)")
    print("=" * 60)

    for label, cmd, expected, timeout in USB_BT_STYLE:
        if is_tcp:
            ok, resp, motor_info = run_test_with_motor(
                ser, label, cmd, expected, timeout, tcp_host, tcp_port)
        else:
            ok, resp = run_test(ser, label, cmd, expected, timeout)
            motor_info = ""

        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<8}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\" (expected {expected!r})")
        if motor_info:
            print(motor_info)

    print("\n" + "=" * 60)
    print(" Raw Single-Byte Direction Commands")
    print("=" * 60)

    for label, cmd, expected, timeout in RAW_DIRECTION_CASES:
        if is_tcp:
            ok, resp, motor_info = run_test_with_motor(
                ser, label, cmd, expected, timeout, tcp_host, tcp_port)
        else:
            ok, resp = run_test(ser, label, cmd, expected, timeout)
            motor_info = ""

        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<8}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\" (expected {expected!r})")
        if motor_info:
            print(motor_info)

    print("\n" + "=" * 60)
    print(" Extended Protocol Commands")
    print("=" * 60)

    for label, cmd, expected_parts, expected_state, timeout in EXTENDED_USB_CASES:
        if is_tcp:
            ok, resp, state_info = run_extended_case(
                ser, cmd, expected_parts, timeout, expected_state, tcp_host, tcp_port)
        else:
            ok, resp = run_contains_test(ser, cmd, expected_parts, timeout)
            state_info = ""

        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<12}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\"")
        if state_info:
            print(state_info)

    print("\n" + "=" * 60)
    print(" Negative Protocol Checks")
    print("=" * 60)

    for label, cmd, expected_parts, timeout in NEGATIVE_USB_CASES:
        ok, resp = run_contains_test(ser, cmd, expected_parts, timeout)
        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<12}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\"")

    print("\n" + "=" * 60)
    print(" Snapshot Flow")
    print("=" * 60)

    for label, cmd, expected_parts, timeout in SNAP_TEST_CASES:
        ok, resp = run_contains_test(ser, cmd, expected_parts, timeout)
        status = "PASS" if ok else "FAIL"
        if ok:
            passed += 1
        else:
            failed += 1
        print(f"  [{status}] {label:<25} cmd={cmd!r:<12}", end="")
        if ok:
            print(f"  → {resp}")
        else:
            print(f"  → got \"{resp}\"")

    print("\n" + "=" * 60)
    print(f" Results: {passed} passed, {failed} failed"
          f"  (out of {passed + failed} tests)")
    print("=" * 60)
    if is_tcp:
        print(" Motor state: OK verified via state port")
    else:
        print(" Motor state: ⏺ verify with --tcp mode for GPIO checks")

    return failed == 0


# =========================================================================
# Interactive monitor mode
# =========================================================================

def monitor_mode(ser, is_tcp=False, tcp_host=None, tcp_port=None):
    """Interactive monitor mode."""
    print("\n" + "=" * 60)
    print(" Interactive Monitor Mode")
    print(" Type commands to send to STM32.")
    if is_tcp:
        print(" Motor state will be shown after each command.")
    print(" Special commands:")
    print("   /q        - quit")
    print("   /t 500    - set delay between sends (ms)")
    print("   /auto F B L R S  - auto-sequence commands")
    print("   /state    - show motor state (TCP mode only)")
    print("   /h        - help")
    print("=" * 60)

    delay_ms = 200
    ser.timeout = 0.5

    while True:
        try:
            cmd = input(f"\nSTM32 [{delay_ms}ms] > ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break

        if not cmd:
            continue

        if cmd == "/q":
            break
        elif cmd == "/h":
            print("  Commands: go, stop, F, B, L, R, S, xyz (unknown)")
            print("            track, track_off, hold, continue, V45, obs")
            print("  /q        - quit")
            print("  /t <ms>   - set delay")
            print("  /auto ... - run sequence")
            print("  /state    - show motor pins")
            continue
        elif cmd == "/state":
            if is_tcp:
                state = get_motor_state(tcp_host, tcp_port)
                if state:
                    print("  Motor state:")
                    for name in MOTOR_PIN_NAMES:
                        print(f"    {name} = {state.get(name, '?')}")
                    print(f"    mode = {state.get('mode', '?')}")
                    print(f"    speed = {state.get('speed', '?')}")
                else:
                    print("  (state port unreachable)")
            else:
                print("  (only available in --tcp mode)")
            continue
        elif cmd.startswith("/t "):
            try:
                delay_ms = int(cmd.split()[1])
                print(f"  Delay set to {delay_ms}ms")
            except (IndexError, ValueError):
                print("  Usage: /t <milliseconds>")
            continue
        elif cmd.startswith("/auto "):
            seq = cmd.split()[1:]
            if not seq:
                seq = ["F", "B", "L", "R", "S"]
            for c in seq:
                ser.write((c + "\n").encode())
                time.sleep(delay_ms / 1000.0)
                ser.timeout = 0.3
                resp = ser.read(256)
                r = resp.decode('utf-8', errors='replace').strip()
                extra = ""
                if is_tcp and c + "\n" in MOTOR_EXPECTED:
                    state = get_motor_state(tcp_host, tcp_port)
                    _, info = verify_motor_state(
                        state, MOTOR_EXPECTED[(c + "\n").encode()])
                    extra = f"\n  {info}"
                print(f"  {c} → {r}{extra}")
            continue

        # Send single command
        ser.write((cmd + "\n").encode())
        time.sleep(delay_ms / 1000.0)

        ser.timeout = 0.3
        resp = ser.read(256)
        if resp:
            r = resp.decode('utf-8', errors='replace').strip()
            print(f"  → {r}")
        else:
            print("  → (no response)")

        # Show motor state if TCP mode
        if is_tcp and (cmd + "\n").encode() in MOTOR_EXPECTED:
            state = get_motor_state(tcp_host, tcp_port)
            _, info = verify_motor_state(
                state, MOTOR_EXPECTED[(cmd + "\n").encode()])
            print(f"  {info}")


# =========================================================================
# Main
# =========================================================================

def main():
    parser = argparse.ArgumentParser(
        description="STM32 Motor Firmware — Serial Test Script")
    parser.add_argument("port", nargs="?", default=None,
                        help="Serial port (e.g. COM3). Auto-detects if omitted.")
    parser.add_argument("--baud", type=int, default=115200,
                        help="Baud rate (default: 115200)")
    parser.add_argument("--tcp", nargs="?", const="127.0.0.1:54321", default=None,
                        help="Connect to virtual_stm32.py over TCP "
                             "(default: 127.0.0.1:54321)")
    parser.add_argument("--list", action="store_true",
                        help="List available serial ports")
    parser.add_argument("--monitor", action="store_true",
                        help="Interactive monitor mode")
    parser.add_argument("--test-only", action="store_true",
                        help="Run automated tests only, no monitor")

    args = parser.parse_args()

    if args.list:
        list_ports()
        return

    # Connection
    is_tcp = False
    tcp_host = None
    tcp_port = None

    if args.tcp:
        is_tcp = True
        if ":" in args.tcp:
            tcp_host, port_str = args.tcp.rsplit(":", 1)
            tcp_port = int(port_str)
        else:
            tcp_host = args.tcp
            tcp_port = 54321
        ser = connect_tcp(args.tcp)
    else:
        port = args.port or find_serial_port()
        if not port:
            print("ERROR: No serial port found. Use --tcp for virtual test, "
                  "or specify a port.")
            print("  python test_serial.py --tcp      # virtual test")
            print("  python test_serial.py COM3       # hardware test")
            sys.exit(1)

        print(f"Opening {port} @ {args.baud} baud...")
        try:
            ser = serial.Serial(port, args.baud, timeout=0.5)
        except serial.SerialException as e:
            print(f"ERROR: Cannot open {port}: {e}")
            sys.exit(1)
        print(f"Connected to {port}")
        time.sleep(0.5)

    clear_buffer(ser, 0.3)
    success = True

    try:
        if not args.monitor:
            if is_tcp:
                success = run_all_tests(ser, is_tcp=True,
                                        tcp_host=tcp_host, tcp_port=tcp_port)
            else:
                success = run_all_tests(ser)

            if not args.test_only:
                print("\nStarting monitor mode...")
                try:
                    monitor_mode(ser, is_tcp=is_tcp,
                                 tcp_host=tcp_host, tcp_port=tcp_port)
                except KeyboardInterrupt:
                    print()
        else:
            monitor_mode(ser, is_tcp=is_tcp,
                         tcp_host=tcp_host, tcp_port=tcp_port)

    except serial.SerialException as e:
        print(f"ERROR: Serial communication error: {e}")
        success = False
    except (ConnectionError, OSError) as e:
        print(f"ERROR: Connection error: {e}")
        success = False
    finally:
        ser.close()
        if is_tcp:
            print(f"\nDisconnected from {tcp_host}:{tcp_port}")
        else:
            print(f"\nClosed {port}")

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
