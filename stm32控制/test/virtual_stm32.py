#!/usr/bin/env python3
"""
@file    virtual_stm32.py
@brief   Virtual STM32 device — TCP server that emulates the motor firmware.

Single-threaded, blocking I/O. One client at a time (any additional clients
queue up and get served after the current one disconnects).

Protocol: same as the current Bluetooth USART3 command path. Line commands are terminated by \r or \n;
single-byte F/B/L/R/S commands execute immediately when the command buffer is
empty, matching the Qt press/release direction buttons.
Commands: go, stop, F, B, L, R, S

Usage:
  python virtual_stm32.py                    # default port 54321
  python virtual_stm32.py --port 9000        # custom port
  python virtual_stm32.py --state            # query motor state
"""

import sys
import time
import socket
import argparse
import threading


# =========================================================================
# Motor state (same as mycar.c)
# =========================================================================
motor = {"PB8": 0, "PB9": 0, "PB12": 0, "PB13": 0, "PB14": 0, "PB15": 0}
motor_running = 0
car_mode = "MANUAL"
motor_speed = 40
last_distance_mm = 0
track_state = (0, 0, 0)


# =========================================================================
# Motor control (identical logic to mycar.c)
# =========================================================================

def go():
    motor["PB8"] = 1; motor["PB9"] = 1
    motor["PB12"] = 0; motor["PB13"] = 1
    motor["PB14"] = 0; motor["PB15"] = 1
    global motor_running; motor_running = 1

def stop_motors():
    for k in motor: motor[k] = 0
    global motor_running; motor_running = 0

def car_forward(): go()

def car_backward():
    motor["PB8"] = 1; motor["PB9"] = 1
    motor["PB12"] = 1; motor["PB13"] = 0
    motor["PB14"] = 1; motor["PB15"] = 0
    global motor_running; motor_running = 1

def car_left():
    motor["PB8"] = 1; motor["PB9"] = 1
    motor["PB12"] = 1; motor["PB13"] = 0
    motor["PB14"] = 0; motor["PB15"] = 1
    global motor_running; motor_running = 1

def car_right():
    motor["PB8"] = 1; motor["PB9"] = 1
    motor["PB12"] = 0; motor["PB13"] = 1
    motor["PB14"] = 1; motor["PB15"] = 0
    global motor_running; motor_running = 1

def get_state_str():
    return (
        f"Motor State:\n"
        f"PB8={motor['PB8']}\n"
        f"PB9={motor['PB9']}\n"
        f"PB12={motor['PB12']}\n"
        f"PB13={motor['PB13']}\n"
        f"PB14={motor['PB14']}\n"
        f"PB15={motor['PB15']}\n"
        f"motor_running={motor_running}\n"
        f"mode={car_mode}\n"
        f"speed={motor_speed}\n"
    )


# =========================================================================
# Command processing
# =========================================================================

def set_manual_mode():
    global car_mode
    car_mode = "MANUAL"


def set_tracking_mode():
    global car_mode
    car_mode = "TRACKING"


def set_wait_user_mode():
    global car_mode
    car_mode = "WAIT_USER"


def tracking_diag_lines():
    left, mid, right = track_state
    return [
        f"SPEED:{motor_speed}\r\n",
        f"DIST:{last_distance_mm}mm\r\n",
        f"TRACK:{left},{mid},{right}\r\n",
    ]


def process_cmd(cmd):
    global car_mode, motor_speed, last_distance_mm, track_state
    lines = []
    now = time.monotonic()

    if cmd == "go":
        set_manual_mode()
        go()
        lines.extend(["MODE:MANUAL\r\n", "OK: go\r\n"])
    elif cmd == "stop":
        set_manual_mode()
        stop_motors()
        lines.extend(["MODE:MANUAL\r\n", "OK: stop\r\n"])
    elif cmd == "F":
        set_manual_mode()
        car_forward()
        lines.extend(["MODE:MANUAL\r\n", "OK: F\r\n"])
    elif cmd == "B":
        set_manual_mode()
        car_backward()
        lines.extend(["MODE:MANUAL\r\n", "OK: B\r\n"])
    elif cmd == "L":
        set_manual_mode()
        car_left()
        lines.extend(["MODE:MANUAL\r\n", "OK: L\r\n"])
    elif cmd == "R":
        set_manual_mode()
        car_right()
        lines.extend(["MODE:MANUAL\r\n", "OK: R\r\n"])
    elif cmd == "S":
        set_manual_mode()
        stop_motors()
        lines.extend(["MODE:MANUAL\r\n", "OK: S\r\n"])
    elif cmd in ("track", "TRACK", "track_on", "TRACK_ON"):
        set_tracking_mode()
        last_distance_mm = 123
        track_state = (1, 0, 1)
        lines.extend(["MODE:TRACKING\r\n", "OK: track\r\n"])
        lines.extend(tracking_diag_lines())
    elif cmd in ("track_off", "TRACK_OFF"):
        set_manual_mode()
        stop_motors()
        lines.extend(["MODE:MANUAL\r\n", "OK: track off\r\n"])
    elif cmd in ("continue", "CONTINUE", "resume", "RESUME"):
        if car_mode == "WAIT_USER":
            set_tracking_mode()
            last_distance_mm = 118
            track_state = (0, 1, 0)
            lines.extend(["MODE:TRACKING\r\n", "OK: continue\r\n"])
            lines.extend(tracking_diag_lines())
        else:
            lines.append("Error: no wait state\r\n")
    elif cmd in ("hold", "HOLD", "wait", "WAIT"):
        set_wait_user_mode()
        stop_motors()
        lines.extend(["MODE:WAIT_USER\r\n", "OK: hold\r\n"])
    elif cmd in ("obs", "OBS", "obstacle", "OBSTACLE"):
        set_wait_user_mode()
        stop_motors()
        last_distance_mm = 120
        lines.extend(["MODE:OBSTACLE_STOP\r\n", "OBS:120mm\r\n"])
    elif cmd.startswith("V") or cmd.startswith("v"):
        value_text = cmd[1:]
        if value_text.isdigit() and 0 <= int(value_text) <= 100:
            motor_speed = int(value_text)
            set_manual_mode()
            lines.extend(["MODE:MANUAL\r\n", f"OK: V{motor_speed}\r\n"])
        else:
            lines.append("Error: speed 0-100\r\n")
    elif cmd in ("snap", "SNAP", "photo", "PHOTO"):
        lines.append("Error: K210 uses USB\r\n")
    elif cmd in ("kping", "KPING", "pingk210", "PINGK210"):
        lines.append("Error: K210 uses USB\r\n")
    elif cmd in ("status", "STATUS", "diag", "DIAG"):
        lines.extend([f"MODE:{car_mode}\r\n"])
        lines.extend(tracking_diag_lines())
        lines.append("BT_RX_OVF:0\r\n")
        lines.append("OK: status\r\n")
    else:
        lines.append("Error: unknown command\r\n")

    return lines


# =========================================================================
# Per-connection handler (blocking, sequential)
# =========================================================================

def serve_client(conn, addr):
    """Handle one client session — read commands, send responses."""
    sys.stdout.write(f"  [连接] {addr}\n")
    sys.stdout.flush()

    buf = b""
    conn.settimeout(1.0)

    try:
        while True:
            try:
                data = conn.recv(256)
            except socket.timeout:
                continue
            except (ConnectionError, OSError):
                break

            if not data:
                break

            for byte in data:
                if byte in b"FBLRS" and not buf:
                    line = bytes([byte])
                elif byte in b"\r\n":
                    line = buf
                    buf = b""
                else:
                    buf += bytes([byte])
                    continue

                line = line.strip(b"\r\n")
                if not line:
                    continue

                cmd = line.decode("utf-8", errors="replace")
                sys.stdout.write(f"  [<<<] \"{cmd}\"\n")
                sys.stdout.flush()

                responses = process_cmd(cmd)
                for usb_resp in responses:
                    conn.sendall(usb_resp.encode("utf-8"))
                    sys.stdout.write(f"  [>>>] \"{usb_resp.strip()}\"\n")
                    sys.stdout.flush()
                    if usb_resp.startswith("IMG:DATA:"):
                        time.sleep(0.01)

    except Exception as e:
        sys.stdout.write(f"  [err] {e}\n")
        sys.stdout.flush()
    finally:
        conn.close()
        sys.stdout.write(f"  [断开] {addr}\n")
        sys.stdout.flush()


def serve_state(conn, addr):
    """Handle a state query."""
    try:
        conn.sendall(get_state_str().encode("utf-8"))
    finally:
        conn.close()


# =========================================================================
# Main
# =========================================================================

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=54321)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--state", action="store_true")
    args = parser.parse_args()

    # State query mode
    if args.state:
        try:
            s = socket.socket()
            s.settimeout(3.0)
            s.connect((args.host, args.port + 1))
            data = s.recv(4096)
            print(data.decode("utf-8"))
            s.close()
        except (ConnectionRefusedError, OSError) as e:
            print(f"Error: virtual STM32 not running: {e}")
            sys.exit(1)
        return

    # Create main server socket
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((args.host, args.port))
    server.listen(1)
    server.settimeout(1.0)

    # State server — runs in a background thread so it's always responsive
    state_srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    state_srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    state_srv.bind((args.host, args.port + 1))
    state_srv.listen(5)
    state_srv.settimeout(1.0)

    def state_loop():
        """Thread: accept state queries and respond immediately."""
        while True:
            try:
                conn, addr = state_srv.accept()
                serve_state(conn, addr)
            except socket.timeout:
                continue
            except OSError:
                break

    state_thread = threading.Thread(target=state_loop, daemon=True)
    state_thread.start()

    sys.stdout.write(
        f"\n"
        f"  Virtual STM32 — Motor Firmware Emulator\n"
        f"  {'=' * 40}\n"
        f"  Main port:   {args.host}:{args.port}\n"
        f"  State port:  {args.host}:{args.port + 1}\n"
        f"  Commands:    go, stop, F, B, L, R, S, V0..V100,\n"
        f"               track, track_off, hold, continue, obs\n"
        f"  {'=' * 40}\n"
        f"  Test:  python test_serial.py --tcp\n"
        f"  State: python virtual_stm32.py --state\n"
        f"\n"
    )
    sys.stdout.flush()

    # Try both accept() calls with timeout; handle whichever connects.
    # If timeout fires on both, loop and try again.
    try:
        while True:
            # Try main server
            try:
                conn, addr = server.accept()
                serve_client(conn, addr)
                continue
            except socket.timeout:
                pass

            # Try state server
            try:
                conn, addr = state_srv.accept()
                serve_state(conn, addr)
                continue
            except socket.timeout:
                pass

    except KeyboardInterrupt:
        sys.stdout.write("\n  Shutting down...\n")
    finally:
        server.close()
        state_srv.close()
        sys.stdout.write("  Done.\n")


if __name__ == "__main__":
    main()
