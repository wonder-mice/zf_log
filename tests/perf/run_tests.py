#!/usr/bin/python

import sys
import os
import argparse
import subprocess
import multiprocessing
import json
import pprint

def take_first(value):
	if type(value) is tuple or type(value) is list:
		return value[0]
	return value

def take_order(value, order):
	for i in range(len(order)):
		if value == order[i]:
			return i
	return len(order)

def take_map(value, keys, vals):
	if len(keys) != len(vals):
		raise RuntimeError("Length of keys and vals must match")
	for i in range(len(keys)):
		if value == keys[i]:
			return vals[i]
	return value

def take_best(values, key, compare, reverse=False):
	if 0 == len(values):
		return []
	values = sorted(values, key=key, reverse=reverse)
	for i in range(1, len(values)):
		if not compare(values[0], values[i]):
			return values[0:i]
	return values

def take_plural(count, base, suffix):
	if 1 == abs(count):
		return base
	return base + suffix

def take_threads_variants():
	# single thread
	variants = [1]
	cpus = multiprocessing.cpu_count()
	if 1 > cpus:
		cpus = 1
	# load all CPUs
	if 1 < cpus:
		variants.append(cpus)
	# overcommit
	variants.append(2 * cpus)
	return variants

def _cmp_percentage(p, key, a, b):
	if key(a) == key(b):
		return True
	if p >= abs((key(a) - key(b)) / float(key(a))):
		return True
	return False

def cmp_percentage(p, key):
	return lambda a, b: _cmp_percentage(p, key, a, b)

def translate_test(test):
	name = take_first(test)
	if "call_site_size.str" == name:
		return 1000, "Call site size: string"
	if "call_site_size.fmti" == name:
		return 2000, "Call site size: 3 integers"
	if "executable_size.m1" == name:
		return 3000, "Executable size: 1 module"
	if "compile_time" == name:
		return 4000, "Module compile time"
	if "link_time" == name:
		return 5000, "Executable link time"
	if "speed" == name:
		threads = test[1]
		mode = test[2]
		mode_keys = ["str",    "fmti",       "str-off",        "slowf-off"]
		mode_vals = ["string", "3 integers", "string, off", "slow function, off"]
		tr_mode = take_map(mode, mode_keys, mode_vals)
		order = 10 * threads + take_order(mode, mode_keys)
		return 6000 + order, "Speed: %i %s, %s" % (threads, take_plural(threads, "thread", "s"), tr_mode)
	if type(test) is tuple or type(test) is list:
		return 31416, ", ".join(test)
	return 27183, test

def translate_subj(subj):
	if "zf_log_n" == subj:
		return 31416, "zf_log"
	if "easylog" == subj:
		return 31416, "Easylogging++"
	return 31416, subj

def translation_sort_key(v):
	return "[%04i] %s" % (v[0], v[1])

def translation_value(v):
	return v[1]

class data_cell:
	def __init__(self):
		self.best = False
	def set_best(self, best=True):
		self.best = best
	def ifbest(self, a, b):
		if hasattr(self, "best") and self.best:
			return a
		return b

class data_str(data_cell):
	def __init__(self, value):
		if type(value) is not str:
			raise RuntimeError("Not a string")
		self.value = value
	def __str__(self):
		return self.value
	def __repr__(self):
		return repr(self.value)

class data_bytes(data_cell):
	def __init__(self, value):
		if type(value) is not int:
			raise RuntimeError("Not an int")
		self.value = value
	def __str__(self):
		if self.value < 1024:
			return "%i B" % (self.value)
		if self.value < 1024 * 1024:
			return "%.2f KB" % (self.value / 1024.0)
		return "%.2f MB" % (self.value / 1024.0 / 1024.0)
	def __repr__(self):
		return repr(self.value)

class data_seconds(data_cell):
	def __init__(self, value):
		if type(value) is not int and type(value) is not float:
			raise RuntimeError("Not an int or float")
		self.value = value
	def __str__(self):
		return "%.3f sec" % (self.value)
	def __repr__(self):
		return repr(self.value)

class data_freq(data_cell):
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
	def freq(self):
		return self.count / self.seconds

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
		table[i][0] = data_str(translation_value(translate_test(tests[i - 1])))
	for j in range(1, cols):
		table[0][j] = data_str(translation_value(translate_subj(subjs[j - 1])))
	# put data
	for i in range(1, rows):
		for j in range(1, cols):
			test = tests[i - 1]
			subj = subjs[j - 1]
			if subj in result[test]:
				table[i][j] = result[test][subj]
	# gen cells content
	for i in range(0, rows):
		for j in range(0, cols):
			value = table[i][j]
			if value is None:
				value = data_str("")
			elif not isinstance(value, data_cell):
				raise RuntimeError("Value \"%s\" is of unsupported type \"%s\"" % (value, type(value)))
			table[i][j] = value
	# find cols width
	widths = [0 for _ in range(cols)]
	for j in range(0, cols):
		for i in range(0, rows):
			s = str(table[i][j])
			if widths[j] < len(s):
				widths[j] = len(s)
	return table, rows, cols, widths

def gen_table_ascii(result):
	table, rows, cols, widths = get_table_data(result)
	# apply cell format
	margin_norm  = (" ", " ")
	margin_best  = ("*", " ")
	margins = max(map(len, margin_norm)) + max(map(len, margin_norm))
	for i in range(1, rows):
		table[i][0] = str(table[i][0]).ljust(widths[0]).join(margin_norm)
	for j in range(0, cols):
		table[0][j] = str(table[0][j]).center(widths[j]).join(margin_norm)
	for i in range(1, rows):
		for j in range(1, cols):
			data = table[i][j]
			margin = data.ifbest(margin_best, margin_norm)
			table[i][j] = str(data).rjust(widths[j]).join(margin)
	# draw chart
	line = "+" + "-" * (sum(widths) + (margins + 1) * len(widths) - 1) + "+\n"
	chart = line
	for row in table:
		chart += "|"
		for cell in row:
			chart += "%s|" % (cell)
		chart += "\n" + line
	return chart

def gen_table_markdown(result):
	table, rows, cols, widths = get_table_data(result)
	# apply cell format
	margin_norm  = ("  ", "  ")
	margin_best  = ("**", "**")
	margins = max(map(len, margin_norm)) + max(map(len, margin_norm))
	for i in range(1, rows):
		table[i][0] = str(table[i][0]).ljust(widths[0]).join(margin_norm)
	for j in range(0, cols):
		table[0][j] = str(table[0][j]).center(widths[j]).join(margin_norm)
	for i in range(1, rows):
		for j in range(1, cols):
			data = table[i][j]
			margin = data.ifbest(margin_best, margin_norm)
			table[i][j] = str(data).join(margin).rjust(widths[j] + margins)
	# draw chart
	chart = ""
	if 0 == rows:
		return chart
	chart += "|"
	for cell in table[0]:
		chart += "%s|" % (cell)
	chart += "\n"
	chart += "| " + "-" * (margins + widths[0] - 2) + " |"
	for i in range(1, cols):
		chart += " " + "-" * (margins + widths[i] - 2) + ":|"
	chart += "\n"
	for i in range(1, rows):
		chart += "|"
		for j in range(0, cols):
			chart += "%s|" % (table[i][j])
		chart += "\n"
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
			data = data_bytes(sz2 - sz1)
			values[subj] = data
		result[name] = values
		for best in take_best(values.values(),
							  key=lambda x: x.value,
							  compare=cmp_percentage(0.0, key=lambda x: x.value)):
			best.set_best()

def run_executable_size(params, result):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	id = "executable_size"
	params = params[id]
	for mode in params:
		name = "%s.%s" % (id, mode)
		values = dict()
		for subj in params[mode]:
			sz = os.path.getsize(params[mode][subj])
			values[subj] = data_bytes(sz)
		result[name] = values
		for best in take_best(values.values(),
							  key=lambda x: x.value,
							  compare=cmp_percentage(0.0, key=lambda x: x.value)):
			best.set_best()

def run_build_time(params, result, id, optional=False):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	if optional and id not in params:
		return
	params = params[id]
	values = dict()
	for subj in params:
		with open(params[subj], "r") as f:
			dt = json.load(f)["dt"]
			values[subj] = data_seconds(dt)
	result[id] = values
	for best in take_best(values.values(),
						  key=lambda x: x.value,
						  compare=cmp_percentage(0.2, key=lambda x: x.value)):
		best.set_best()

def run_speed(params, result, threads_variants, seconds=1):
	if type(result) is not dict:
		raise RuntimeError("Not a dictionary")
	id = "speed"
	params = params[id]
	for threads in threads_variants:
		for mode in params:
			name = (id, threads, mode)
			values = dict()
			for subj in params[mode]:
				path = params[mode][subj]
				p = subprocess.Popen([path, str(threads), str(seconds)], stdout=subprocess.PIPE)
				stdout, stderr = p.communicate()
				values[subj] = data_freq(int(stdout), seconds)
			result[name] = values
			for best in take_best(values.values(),
								  key=lambda x: x.freq(), reverse=True,
								  compare=cmp_percentage(0.1, key=lambda x: x.freq())):
				best.set_best()

def run_tests(params):
	result = dict()
	run_call_site_size(params, result)
	run_executable_size(params, result)
	run_build_time(params, result, "compile_time", optional=True)
	run_build_time(params, result, "link_time", optional=True)
	run_speed(params, result, take_threads_variants())
	return result

def main(argv):
	parser = argparse.ArgumentParser()
	parser.add_argument("-t", "--text", metavar="PATH", default=None,
			help="Text output file path")
	parser.add_argument("-m", "--markdown", metavar="PATH", default=None,
			help="Markdown output file path")
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
	if args.verbose:
		sys.stderr.write(pprint.pformat(params, indent=4))
		sys.stderr.write("\n")
	# run, run, run!
	result = run_tests(params)
	if args.verbose:
		sys.stderr.write(pprint.pformat(result, indent=4))
		sys.stderr.write("\n")
	if args.text is not None:
		with open(args.text, "w") as f:
			table = gen_table_ascii(result)
			f.write(table)
	if args.markdown is not None:
		with open(args.markdown, "w") as f:
			table = gen_table_markdown(result)
			f.write(table)
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
