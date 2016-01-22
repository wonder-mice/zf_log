#!/usr/bin/python

import sys
import os
import argparse
import subprocess
import json

def translate_test(test):
	if type(test) is tuple:
		name = test[0]
	else:
		name = test
	if "call_site_size.str" == name:
		return 100, "Call site size: string"
	if "call_site_size.fmti" == name:
		return 200, "Call site size: 4 integers"
	if "call_site_size.fmtf" == name:
		return 300, "Call site size: 4 int functions"
	if "executable_size" == name:
		return 400, "Executable size"
	if "compile_time" == name:
		return 500, "Compile time"
	if "link_time" == name:
		return 600, "Link time"
	if "speed" == name:
		threads = test[1]
		mode = test[2]
		dn = 2 * threads
		if "str" == mode:
			mode = "string"
		elif "fmti" == mode:
			mode = "4 integers"
			dn += 1
		return 700 + dn, "Performance: %i threads, %s" % (threads, mode)
	if type(test) is tuple:
		return 3142, ", ".join(test)
	return 2718, ", ".test

def translate_subj(subj):
	if "zf_log_n" == subj:
		return 3142, "zf_log"
	if "easylog" == subj:
		return 3142, "Easylogging++"
	return 3142, subj

def translation_sort_key(v):
	return "[%04i] %s" % (v[0], v[1])

def translation_value(v):
	return v[1]

class data_bytes:
	def __init__(self, value):
		if type(value) is not int:
			raise RuntimeError("Not an int")
		self.value = value
	def __str__(self):
		if self.value < 1024:
			return "%i B" % (self.value)
		if self.value < 1024 * 1024:
			return "%.2f KiB" % (self.value / 1024.0)
		return "%.2f MiB" % (self.value / 1024.0 / 1024.0)
	def __repr__(self):
		return repr(self.value)

class data_seconds:
	def __init__(self, value):
		if type(value) is not int and type(value) is not float:
			raise RuntimeError("Not an int or float")
		self.value = value
	def __str__(self):
		return "%.3f sec" % (self.value)
	def __repr__(self):
		return repr(self.value)

class data_freq:
	def __init__(self, count, seconds):
		if type(count) is not int and type(count) is not float:
			raise RuntimeError("Not an int or float")
		if type(seconds) is not int and type(seconds) is not float:
			raise RuntimeError("Not an int or float")
		self.count = count
		self.seconds = seconds
	def __str__(self):
		return "{:,}".format(self.count / self.seconds)
	def __repr__(self):
		return repr((self.count, self.seconds))

def get_table_data(result):
	# collect all tests
	tests = result.keys()
	tests = sorted(tests, key=lambda x: translation_sort_key(translate_test(x)))
	# collect all subjects
	subjs = set()
	for test in tests:
		subjs.update(result[test].keys())
	subjs = sorted(subjs, key=lambda x: translation_sort_key(translate_subj(x)))
	# create table
	rows = len(tests) + 1
	cols = len(subjs) + 1
	table = [[None for _ in range(cols)] for _ in range(rows)]
	# put names and captions
	for i in range(1, rows):
		table[i][0] = translation_value(translate_test(tests[i - 1]))
	for j in range(1, cols):
		table[0][j] = translation_value(translate_subj(subjs[j - 1]))
	# put data
	for i in range(1, rows):
		for j in range(1, cols):
			test = tests[i - 1]
			subj = subjs[j - 1]
			if subj in result[test]:
				table[i][j] = result[test][subj]
	return table, rows, cols

def gen_table_ascii(result):
	table, rows, cols = get_table_data(result)
	# gen cells content
	for i in range(0, rows):
		for j in range(0, cols):
			value = table[i][j]
			if value is None:
				value = "-"
			elif type(value) is int:
				value = "{:,}".format(value)
			elif type(value) is not str:
				value = str(value)
			table[i][j] = value
	# find cols width
	widths = [0 for _ in range(cols)]
	for j in range(0, cols):
		for i in range(0, rows):
			s = table[i][j]
			if widths[j] < len(s):
				widths[j] = len(s)
	# align cells
	for i in range(1, rows):
		table[i][0] = table[i][0].ljust(widths[0])
	for j in range(0, cols):
		table[0][j] = table[0][j].center(widths[j])
	for i in range(1, rows):
		for j in range(1, cols):
			table[i][j] = table[i][j].rjust(widths[j])
	# draw chart
	margin = " "
	line = "+" + "-" * (sum(widths) + (2 * len(margin) + 1) * len(widths) - 1) + "+\n"
	chart = line
	for row in table:
		chart += "|"
		for cell in row:
			chart += "%s%s%s|" % (margin, cell, margin)
		chart += "\n" + line
	return chart

def run_call_site_size(params, result):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	id = "call_site_size"
	params = params[id]
	for mode in params:
		name = "%s.%s" % (id, mode)
		values = dict()
		for subj in params[mode]:
			data = params[mode][subj]
			sz1 = os.path.getsize(data["1"])
			sz2 = os.path.getsize(data["2"])
			values[subj] = data_bytes(sz2 - sz1)
		result[name] = values

def run_min_executable_size(params, result):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	id = "executable_size"
	params = params[id]
	values = dict()
	for subj in params:
		sz = os.path.getsize(params[subj])
		values[subj] = data_bytes(sz)
	result[id] = values

def run_build_time(params, result, id):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	params = params[id]
	values = dict()
	for subj in params:
		with open(params[subj], "r") as f:
			dt = json.load(f)["dt"]
			values[subj] = data_seconds(dt)
	result[id] = values

def run_speed(params, result, threads, seconds=1):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	id = "speed"
	params = params[id]
	for mode in params:
		name = (id, threads, mode)
		values = dict()
		for subj in params[mode]:
			path = params[mode][subj]
			p = subprocess.Popen([path, str(threads)], stdout=subprocess.PIPE)
			stdout, stderr = p.communicate()
			values[subj] = data_freq(int(stdout), seconds)
		result[name] = values

def run_tests(params):
	result = dict()
	run_call_site_size(params, result)
	run_min_executable_size(params, result)
	run_build_time(params, result, "compile_time")
	run_build_time(params, result, "link_time")
	run_speed(params, result, 1)
	run_speed(params, result, 4)
	return result

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument("-o", "--out", metavar="PATH",
			help="Output file path")
	parser.add_argument("-p", "--parameter", metavar="VALUE", action="append", default=[],
			help="Input parameter")
	parser.add_argument("-b", "--build", metavar="TYPE",
			help="Input parameter")
	parser.add_argument("-v", "--verbose", action="store_true",
			help="Verbose output")
	args = parser.parse_args(argv[1:])
	# process parameters
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
	if args.verbose:
		out.write(json.dumps(params, indent=4, default=repr))
		out.write("\n")
	result = run_tests(params)
	if args.verbose:
		out.write(json.dumps(result, indent=4, default=repr))
		out.write("\n")
	table = gen_table_ascii(result)
	out.write(table)
	out.flush()
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
