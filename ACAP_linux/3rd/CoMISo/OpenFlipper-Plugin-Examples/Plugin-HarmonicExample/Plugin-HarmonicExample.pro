################################################################################
#
################################################################################

include( $$TOPDIR/qmake/all.include )

Plugin()
#qwt()
#gmm()
#lapack()
#physim2()
#cgal()

DIRECTORIES = . 

# Input
HEADERS += $$getFilesFromDir($$DIRECTORIES,*.hh)
SOURCES += $$getFilesFromDir($$DIRECTORIES,*.cc)
FORMS   += $$getFilesFromDir($$DIRECTORIES,*.ui)

################################################################################
