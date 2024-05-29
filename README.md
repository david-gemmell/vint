# VInt
Variable-length quantity(VLQ) 7 bits per byte encoder/decoder for 32-bit integers. 
Read more about VLQ at https://en.wikipedia.org/wiki/Variable-length_quantity

## Contents
- [Technologies](#technologies)
- [Getting started](#getting-started)
- [Development](#development)
- [Testing](#testing)
- [Contributing](#contributing)
- [Authors](#authors)

## Technologies
- [GCC](https://gcc.gnu.org/)
- [Make](https://www.gnu.org/software/make/)
- [Minunit](http://www.jera.com/techinfo/jtns/jtn002.html)

## Getting started
Build and install the utility using the command:
```sh
$ sudo make install
```

Examples of use binary:
```sh
$ echo -en '\x00\x00\x00\x31' | vint -e -b | od -x
0000000 0031
0000001
$ echo -en '\x00\x00\x00\x31' | vint -e -b | vint -d -b | od -x
0000000 0000 3100
0000004
```
Examples of use text:
```sh
$ echo -n '49' | vint -e | od -x
0000000 0031
0000001
$ echo -n '49' | vint -e -t | vint -d -t
49
```
Examples of use formatted text:
```sh
$ echo -n '49 79 2 97 911' | od -x
0000000 3934 3720 2039 2032 3739 3920 3131
0000016
$ echo -n '49 79 2 97 911' | vint -e -t --no-header -s 'col1,col2,col3:a' | od -x
0000000 4f31 6102 0f87
0000006
$ echo -n '49 79 2 97 911' | vint -e -t --no-header -s 'col1,col2,col3:a' | vint -d -t -s 'col1,col2,col3:a'
col1    col2    col3
49      79      97:911
```

## Development

### Build creation
Build the utility using the command: 
```sh
$ sudo make
```

## Testing
Test the utility using the command:
```sh
$ sudo make test
```

## Contributing

### Why was the project developed?
The utility is ralised as a training project for learning the C language. And for viewing VLC files at work.

## Authors
David Gemmell