#!/usr/bin/python

import sys
import time
import subprocess
import json

def usage(f, st):
	f.write("Usage:\n")
	f.write("    time_it.py file utility [argument ...]\n\n")
	f.write("Writes running time of utility into file.\n")
	return st

def main(argv):
	if 3 > len(argv):
		return usage(sys.stderr, -1)
	t = time.time()
	ret = subprocess.call(argv[2:])
	t = time.time() - t
	if 0 == ret:
		with open(argv[1], "w") as f:
			json.dump({"dt":t}, f)
	return ret

if __name__ == "__main__":
	sys.exit(main(sys.argv))
