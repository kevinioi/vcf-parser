# vcf-parser

A simple C library for parsing .vcf files

## Program Description

This package contains functions used to parse a provided VCard (vcf) file.
It is assumed that the vCard file will follow the vCard version 4.0 format.

Two separate libraries can be compiled from this library. One being the full
parser, libcparse.a, contianing all necessary functions for parsing (apart from a main). the
second library, libllist, solely contains a linkedlist API.

## Compiling

To compile 'libcparse.a' and 'libllist.a':
	make
		or
	make all

To compile 'libcparse.a':
	make parser

To compile 'libllist.a':
	make list
