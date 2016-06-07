#! /usr/bin/env python
# encoding: utf-8

def options(opt):
	opt.recurse('fna')
	opt.recurse('gref')
	opt.load('compiler_c')

def configure(conf):
	conf.recurse('fna')
	conf.recurse('gref')

	conf.load('ar')
	conf.load('compiler_c')

	conf.env.append_value('CFLAGS', '-O3')
	conf.env.append_value('CFLAGS', '-std=c99')
	conf.env.append_value('CFLAGS', '-march=native')

	conf.env.append_value('LIB_SR', conf.env.LIB_FNA + conf.env.LIB_GREF)
	conf.env.append_value('DEFINES_SR', conf.env.DEFINES_FNA + conf.env.DEFINES_GREF)
	conf.env.append_value('OBJ_SR', ['sr.o'] + conf.env.OBJ_FNA + conf.env.OBJ_GREF)


def build(bld):
	bld.recurse('fna')
	bld.recurse('gref')

	bld.objects(source = 'sr.c', target = 'sr.o')

	bld.stlib(
		source = ['unittest.c'],
		target = 'sr',
		use = bld.env.OBJ_SR,
		lib = bld.env.LIB_SR,
		defines = bld.env.DEFINES_SR)

	bld.program(
		source = ['unittest.c'],
		target = 'unittest',
		use = bld.env.OBJ_SR,
		lib = bld.env.LIB_SR,
		defines = ['TEST'] + bld.env.DEFINES_SR)
