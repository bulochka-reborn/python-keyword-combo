# python-keyword-combo

An indicator that makes combo of written Python keywords 
[alt image](https://raw.githubusercontent.com/bulochka-reborn/python-keyword-combo/4a2811b7ff761ffce89d6b8313136b384aab9c3c/demonstration.gif)

## Table of contents

- [Installation](#installation)
- [Usage](#usage)
- [Dependencies](#dependencies)
- [To do](#to-do)
- [Thanks](#thanks)

## Installation

In the directory with main.c and Makefile use:

```sh
$ make
```
Ready to use file will be named main.out.

## Usage

In the directory with main.c and Makefile use:

```sh
$ ./main.out -x window_pos_x -y window_pos_y
```

## Dependencies

- libx11
- libudev
- libinput
- libxft

## To do

- [ ] Add proper command line argunents handling
- [ ] Add auto positioning
- [ ] Add language and Caps Lock tracking
- [ ] Add more keywords and functions
- [ ] Delete keywords and functions like "if", "id" etc.

## Thanks

Thanks to [rickyzhang82](https://gist.github.com/rickyzhang82) for list of linux keycodes.



