# python-keyword-combo

An indicator that makes combo of written Python keywords 
![alt image](https://raw.githubusercontent.com/bulochka-reborn/python-keyword-combo/4a2811b7ff761ffce89d6b8313136b384aab9c3c/demonstration.gif)

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
Ready to use file will be named "main.out".

## Usage

Usage: 
```sh
$ ./main.out
```
Start with Caps Lock turned off (will be improved in the future)

To specify window`s position:
```sh
$ ./main.out -x pos_x -y pos_y
```

To use autopositioning:
```sh
$ ./main.out -a corner_num
```
corner_num = 1 - upper right corner
corner_num = 2 - upper left corner
corner_num = 3 - lower right corner
corner_num = 4 - lower left corner

To specify window`s width and height:
```sh
$ ./main.out -w width -h height
```

To disable Caps Lock tracking:
```sh
$ ./main.out -c
```

To disable Shift tracking:
```sh
$ ./main.out -s
```

To specify border width:
```sh
$ ./main.out -b border_width
```

## Dependencies

- libx11
- libudev
- libinput
- libxft
- freetype2

## To do

- [ ] Add language tracking
- [ ] Add more keywords, functions and methods
- [ ] Delete keywords and functions like "if", "id" etc.

## Thanks

Thanks to [rickyzhang82](https://gist.github.com/rickyzhang82) for list of linux keycodes.



