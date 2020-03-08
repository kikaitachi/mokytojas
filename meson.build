project('mokytojas', 'c')

incdir = include_directories('include')

gtk_dep = dependency('gtk+-3.0')

gnome = import('gnome')
asresources = gnome.compile_resources('as-resources',
  'mokytojas.gresource.xml',
  source_dir: '.',
  c_name: 'as')

executable('mokytojas',
	['src/mokytojas.c'],
	asresources,
	dependencies: [gtk_dep],
	include_directories: incdir,
	override_options: ['c_std=c11'],
	install: true)

