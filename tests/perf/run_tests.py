#!/usr/bin/python

import sys

def main(argv):
	f = open('testfile.txt', 'w')
	f.write(str(argv))
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
