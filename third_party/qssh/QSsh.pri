INCLUDEPATH += $$PWD/include/
message($$INCLUDEPATH)

LIBPATH = $$PWD/lib/
message($$LIBPATH)

LIBS += -L$${LIBPATH} -lQSsh

win32:CONFIG(release, debug|release): LIBS += -L$${LIBPATH} -lQSsh
else:win32:CONFIG(debug, debug|release): LIBS += -L$${LIBPATH} -lQSsh
