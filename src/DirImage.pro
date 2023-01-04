INCLUDEPATH += . ../vendors

contains(QMAKE_TARGET.arch, x86_64) {
	LIBS += -lFreeImage -L$$_PRO_FILE_PWD_/../vendors/FreeImage/x64
} else {
	LIBS += -lFreeImage -L$$_PRO_FILE_PWD_/../vendors/FreeImage/x32
}

TARGET = DirImage
TEMPLATE = lib

#QMAKE_CXXFLAGS += -Wno-unused-parameter

gcc: QMAKE_LFLAGS = -static -static-libgcc --enable-stdcall-fixup -mwindows

msvc: LIBS += -luser32 -lgdi32
msvc: QMAKE_CXXFLAG = -std:c++14

CONFIG += c++14

CONFIG(release, debug|release) {
	EXTRA_FILES += $$_PRO_FILE_PWD_/../runtime/$${TARGET}.ini \
		$$_PRO_FILE_PWD_/../runtime/lgpl-3.0.txt \
		$$_PRO_FILE_PWD_/../runtime/readme.ru.txt \
		$$_PRO_FILE_PWD_/../runtime/readme.en.txt
		
	contains(QMAKE_TARGET.arch, x86_64) {
		EXTRA_FILES += $$_PRO_FILE_PWD_/../vendors/FreeImage/x64/FreeImage.dll
		DEPLOY_DIR = $$_PRO_FILE_PWD_/../deploy/x64
	} else {
		EXTRA_FILES += $$_PRO_FILE_PWD_/../vendors/FreeImage/x32/FreeImage.dll
		DEPLOY_DIR = $$_PRO_FILE_PWD_/../deploy/x32
	}
	
	# Copies the files in EXTRA_FILES to the DEPLOY_DIR
	for (FILE, EXTRA_FILES) {
		QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$FILE) $$shell_path($$DEPLOY_DIR) $$escape_expand(\n\t)
	}
}

DLLDESTDIR = $$DEPLOY_DIR

contains(QMAKE_TARGET.arch, x86_64) {
	TARGET_EXT = .wlx64
} else {
	TARGET_EXT = .wlx
}

SOURCES += \
	INI.cpp \
	L10n.cpp \
	Logger.cpp \
	NaturalCompare.cpp \
	Thumbs.cpp \
	main.cpp \
	Master.cpp \
	HotKey.cpp \
	Image.cpp \
	FileIterator.cpp

HEADERS += \
	INI.h \
	L10n.h \
	NaturalCompare.h \
	Singleton.h \
	Logger.h \
	Thumbs.h \
	TCPS.h \
	Master.h \
	HotKey.h \
	Image.h \
	FileIterator.h

DEF_FILE += DirImage.def

OTHER_FILES += \
	DirImage.def \
	resources/DirImage.ini