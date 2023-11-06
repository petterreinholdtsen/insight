##############################################################################
#
#   Creation date:  2017-07-12
#   Created by:     Ole Liabo
#
#   Copyright (c) 2017 Piql AS, Norway. All rights reserved.
#
##############################################################################

QT               +=   xml widgets gui core printsupport sql core5compat pdf 
TEMPLATE          =   app
win32:TEMPLATE   +=   vcapp
CONFIG           +=   qt debug_and_release console
CONFIG           +=   lrelease 
macx:CONFIG      +=   app_bundle
TARGET            =   insight
!win32:DESTDIR    =   ./innsyn-v1.3.0

##  SUPPORT DEBUG AND RELEASE BUILDS  ##
!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        CURBUILD = debug
    }
    CONFIG(release, debug|release) {
        CURBUILD = release
        DEFINES += NDEBUG
    }
}

##  LNX PLATFORM  ##
PLATFORM                    =   LNX
linux-g++-64:PLATFORM       =   LNX64

##  WIN PLATFORM  ##
win32 {
    contains(QT_ARCH, i386) {
        message("Windows 32-bit")
        PLATFORM              =   W32
    } else {
        message("Windows 64-bit")
        PLATFORM              =   W64
    }
}


OBJECTS_DIR                    =  obj/$$PLATFORM/$$CURBUILD
MOC_DIR                        =  obj/$$PLATFORM/$$CURBUILD
win32:QMAKE_CXXFLAGS_RELEASE  +=  /O2 /Ob2 /Oi /Ot /GL
win32:QMAKE_LFLAGS_RELEASE    +=  /LTCG /DEBUG
macx:QMAKE_LFLAGS             +=  -stdlib=libc++ -liconv -Wno-c++11-narrowing -framework CoreFoundation


CMU112_BASE       =   ../../../base

INCLUDEPATH       =   src \
                      src/gui \
                      src/models \
                      src/formats \
                      src/thirdparty/quazip-1.4/quazip/ \
                      src/thirdparty/minixml/inc \
                      src/thirdparty/tools/inc \
                      src/thirdparty/posixtar/inc \
                      $$[QT_INSTALL_PREFIX]/include/QtZlib \
                      src/thirdparty/quazip-1.4
win32:INCLUDEPATH+=   $$(ZLIB_INCLUDE)
macx:INCLUDEPATH +=   $$(CV_BOOST_INCLUDE)
DEPENDPATH       +=   $$INCLUDEPATH

win32:release:LIBS += \
                      #lib/win64/release/zlib.lib \
                      src/thirdparty/quazip-1.4/lib/quazip/Release/quazip1-qt6.lib
win32:debug:LIBS += \
                      #lib/win64/release/zlib.lib \
                      src/thirdparty/quazip-1.4/lib/quazip/Debug/quazip1-qt6d.lib \
                      "C:/Program Files/MySQL/MySQL Server 8.0/lib/mysqlclient.lib" \
                      "C:/Program Files/MySQL/MySQL Server 8.0/lib/libmysql.lib"

   
# Library dependency checking
PRE_TARGETDEPS   +=   $$LIBS
macx:LIBS +=          -L/usr/local/lib \
                      -lboost_thread-mt \
                      -lboost_date_time \
                      -lboost_regex \
                      -lboost_system \
                      -lboost_chrono \
                      -lquazip1-qt6.1.4
unix:!macx:LIBS +=    -lboost_thread \
                      -lboost_date_time \
                      -lboost_regex \
                      -lboost_system \
                      -lboost_chrono

macx:LIBS        +=   -L/usr/local/Cellar/zlib/1.2.11/lib  -lz -L./src/thirdparty/quazip-1.4/out/quazip/

# Tested on Debian GNU/Linux using distribution
# libraries (libquazip5-dev)
unix:!macx {
    INCLUDEPATH      +=   /usr/include/quazip5
    LIBS             +=   -lquazip5 -lz
    target.path       =   /usr/bin
    INSTALLS         +=   target
}

TOOLS_SOURCES     =   src/thirdparty/tools/src/derror.cpp \
                      src/thirdparty/tools/src/derrorman.cpp \
                      src/thirdparty/tools/src/dpath.cpp \
                      src/thirdparty/tools/src/dtartools.cpp \
                      src/thirdparty/tools/src/dtimeval.cpp \
                      src/thirdparty/tools/src/dstringtools.cpp \
                      src/thirdparty/tools/src/dtimetools.cpp \
                      src/thirdparty/tools/src/dsystemtools.cpp \
                      src/thirdparty/tools/src/dfilesystemtools.cpp \
                      src/thirdparty/tools/src/dexception.cpp \
                      src/thirdparty/tools/src/dbase.cpp \
                      src/thirdparty/tools/src/dbaseio.cpp \
                      src/thirdparty/tools/src/dbasefile.cpp \
                      src/thirdparty/tools/src/dfile.cpp
        
MODELS_SOURCES    =   src/models/dtreeitem.cpp \
                      src/models/dtreemodel.cpp \
                      src/models/dimport.cpp
                                
MODELS_HEADERS    =   src/models/dtreeitem.h \
                      src/models/dtreemodel.h \
                      src/models/dimport.h
                                          
GUI_SOURCES       =   src/gui/dinsightmainwindow.cpp \
                      src/gui/dinsightreportwindow.cpp \
                      src/gui/dinsightjournalwindow.cpp \
                      src/gui/dtreeview.cpp \
                      src/gui/dwaitcursor.cpp \
                      src/gui/qpersistantfiledialog.cpp \
                      src/gui/qaboutdialog.cpp \
                      src/gui/dfixedfolderdialog.cpp
GUI_HEADERS       =   src/gui/dinsightmainwindow.h \
                      src/gui/dinsightreportwindow.h \
                      src/gui/dinsightjournalwindow.h \
                      src/gui/dtreeview.h \
                      src/gui/dwaitcursor.h \
                      src/gui/qpersistantfiledialog.h \
                      src/gui/qaboutdialog.h  \
                      src/gui/dfixedfolderdialog.h

FORMS             =   src/gui/dinsightmainwindow.ui \
                      src/gui/dinsightjournalwindow.ui \
                      src/gui/dinsightreportwindow.ui \
                      src/gui/fixedfolderdialog.ui \
                      src/gui/qaboutdialog.ui

RESOURCES         =   resources.qrc

include( src/thirdparty/yxml/yxml.pri )


SOURCES          +=   src/main.cpp \
                      src/drunguard.cpp \
                      src/dregexp.cpp \
                      src/dxmlparser.cpp \
                      src/ddirparser.cpp \
                      src/dattachmentindexer.cpp \
                      src/dattachmentparser.cpp \
                      src/dsearchthread.cpp \
                      src/dinsightconfig.cpp \
                      src/dinsightreport.cpp \
                      src/dimportformat.cpp \
                      src/dosxtools.cpp \
                      src/dleafmatcher.cpp \
                      src/djournalmatcher.cpp \
                      $$GUI_SOURCES \
                      $$ZIP_SOURCES \
                      $$POSIXTAR_SOURCES \
                      $$MODELS_SOURCES

HEADERS          +=   src/drunguard.h \
                      src/dxmlparser.h \
                      src/ddirparser.h \
                      src/dattachmentindexer.h \
                      src/dattachmentparser.h \
                      src/dsearchthread.h \
                      src/dinsightconfig.h \
                      src/dinsightreport.h \
                      src/dimportformat.h \
                      src/dpendingimport.h \
                      src/dosxtools.h \
                      src/dleafmatcher.h \
                      src/djournalmatcher.h \
                      $$GUI_HEADERS \
                      $$ZIP_HEADERS \
                      $$TOOLS_INCLUDES \
                      $$POSIXTAR_INCLUDES \
                      $$MODELS_HEADERS

TRANSLATIONS      =   insight_nb.ts \
                      insight_nn.ts \
                      insight_en.ts

# Install create_xml tool, used by Sphinx indexer
createxml.depends = src/thirdparty/create_xml/create_xml.pro
createxml.target = sphinx/create_xml
unix:createxml.commands = (cd src/thirdparty/create_xml; qmake; make; cp release/create_xml ../../../sphinx/. )
macx:createxml.commands = (cd src/thirdparty/create_xml; qmake; make; cp create_xml.app/Contents/MacOS/create_xml ../../../sphinx/. )
win32:createxml.commands = (cd src/thirdparty/create_xml; qmake; make; copy release/create_xml ../../../sphinx/. )

# OS-X create icons
createicons.depends = src/gui/resources/icon_32x32.png
createicons.target = src/gui/resources/icon_32x32.icns
createicons.commands = \
    /bin/zsh -c                                            \'; \
    src=src/gui/resources/icon_32x32.png                     ; \
                                                             ; \
    NAME=\$$\(basename \$$src .png); DIR="\$$NAME.iconset"   ; \
    mkdir -pv \$$DIR                                     \'\"; \
    for m r in \'n\' \'\' \'((n+1))\' \'@2x\'; do        \"\'; \
        for n in \$$\(seq 4 9 | grep -v 6); do               ; \
            p=\$$\((2**\$$m)); q=\$$\((2**\$$n))             ; \
            OUT="\$$DIR/icon_\$${q}x\$${q}\$${r}.png"        ; \
            sips -z \$$p \$$p \$$src --out \$$OUT            ; \
        done                                                 ; \
    done                                                     ; \
    iconutil -c icns \$$DIR                                  ; \
    rm -frv \$$DIR                                           ; \
    mv icon_32x32.icns src/gui/resources/.                 \' 


LANGUAGE_FILES.files = insight_en.qm insight_nb.qm insight_nn.qm
LANGUAGE_FILES.path  = Contents/Resources    
CONFIG_FILES.files   = insight.conf formats testdata
CONFIG_FILES.path    = Contents/Resources
QMAKE_BUNDLE_DATA   += CONFIG_FILES LANGUAGE_FILES

#macx:ICON                 = src/gui/resources/icon_32x32.icns
#!macx:ICON                = src/gui/resources/icon_32x32.png
#macx:QMAKE_EXTRA_TARGETS += createicons
#macx:PRE_TARGETDEPS      += src/gui/resources/icon_32x32.icns
#PRE_TARGETDEPS      += sphinx/create_xml
#QMAKE_EXTRA_TARGETS += createxml
