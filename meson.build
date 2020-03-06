project('mokytojas', 'c')

incdir = include_directories('include')

gtk_dep = dependency('gtk+-3.0')

executable('naminukas',
	['src/mokytojas.c'],
	dependencies: [gtk_dep],
	include_directories: incdir,
	override_options: ['c_std=c11'],
	install: true)

