#!/usr/bin/env python3

import subprocess, os, time

TTY="/dev/ttyACM0"

cmd_map = {
    b'1': ['true'],
	b'2': ['mpc', 'volume', '+2'],
    b'3': ['true'],
    b'X': ['true'],

    b'4': ['mpc', 'prev'],
    b'5': ['mpc', 'toggle'],
    b'6': ['mpc', 'next'],
    b'Y': ['true'],

    b'7': ['true'],
	b'8': ['mpc', 'volume', '-2'],
    b'9': ['true'],
    b'Y': ['true'],

    b'C': ['true'],
    b'0': ['true'],
    b'B': ['true'],
    b'W': ['true'],
}


def run_cmd(f, args):
    output = subprocess.check_output(args)
    lines = output.split(b'\n')
    if lines[1].startswith(b'[playing]'):
        f.write(b'P')
    else:
        f.write(b'O')
    f.flush()

def run():

    try:
        subprocess.check_output(['stty', '-F', TTY, 'raw'])
    except subprocess.CalledProcessError:
        return

    with open(TTY, 'r+b', buffering=0) as f:

        run_cmd(f, ['mpc'])

        c = None
        while c != b'':
            c = f.read(1)
            cmd = cmd_map.get(c, None)
            if cmd:
                run_cmd(f, cmd)
 
while True:
    run()
    time.sleep(1)

