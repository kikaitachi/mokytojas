project('mokytojas', 'c')

incdir = include_directories('include')

gtk_dep = dependency('gtk+-3.0')

gnome = import('gnome')
resources = gnome.compile_resources('resources',
  'mokytojas.gresource.xml',
  source_dir: '.')

executable('mokytojas',
	['src/mokytojas.c'],
	resources,
	dependencies: [gtk_dep],
	include_directories: incdir,
	override_options: ['c_std=c11'],
	install: true)

