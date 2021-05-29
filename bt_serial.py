#!/bin/python

import struct
import dmenu
from socket import *
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser()

    parser.add_argument('-m', '--mac', type=str, default='98:D3:71:F9:84:96') 
    args = parser.parse_args()

    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)

    sock.bind(('A8:7E:EA:BF:40:27', 1))
    sock.connect((args.mac, 1))

    opts = {''}
    opt = 'none'
    while opt:
        dmenu.show()

