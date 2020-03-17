project('mokytojas', 'c')

incdir = include_directories('include')

gtk_dep = dependency('gtk+-3.0')

gnome = import('gnome')
resources = gnome.compile_resources('resources',
  'mokytojas.gresource.xml',
  source_dir: '.')

libkikaitachi_proj = subproject('libkikaitachi')
libkikaitachi_dep = libkikaitachi_proj.get_variable('libkikaitachi_dep')

executable('mokytojas',
	['src/mokytojas.c'],
	resources,
	dependencies: [gtk_dep, libkikaitachi_dep],
	include_directories: incdir,
	override_options: ['c_std=c11'],
	install: true)

