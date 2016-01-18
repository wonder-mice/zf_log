#!/usr/bin/python

import sys
import os
import argparse
import pprint

def run_call_site_size(params):
	ps = params["call_site_size"]
	result = dict()
	pprint.pprint(ps)
	for subj in ps.keys():
		print "subj", subj
		sz1 = os.path.getsize(ps[subj]["1"])
		sz2 = os.path.getsize(ps[subj]["2"])
		result[subj] = sz2 - sz1
	return result

def run_min_executable_size(params):
	ps = params["min_executable_size"]
	result = dict()
	pprint.pprint(ps)
	for subj in ps.keys():
		print "subj", subj
		sz = os.path.getsize(ps[subj])
		result[subj] = sz
	return result

def run_tests(out, params):
	result = dict()
	result["call_site_size"] = run_call_site_size(params)
	result["min_executable_size"] = run_min_executable_size(params)
	pprint.pprint(result, out)

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument("-o", "--out", metavar="PATH",
			help="Output file path")
	parser.add_argument("-p", "--parameter", metavar="VALUE", action="append", default=[],
			help="Input parameter")
	parser.add_argument("-b", "--build", metavar="TYPE",
			help="Input parameter")
	args = parser.parse_args(argv[1:])
	params = dict()
	for p in args.parameter:
		d = params
		vs = p.split(":")
		key = None
		for i in range(len(vs)):
			if i == len(vs) - 1:
				d[key] = vs[i]
				break
			if key is not None:
				d = d[key]
			key = vs[i]
			if key not in d:
				d[key] = dict()
	out = sys.stdout
	if args.out is not None:
		out = open(args.out, 'w')
	run_tests(out, params)
	out.flush()
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
