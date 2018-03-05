TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -std=c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CFLAGS_RELEASE   += -O3
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4

HEADERS += \
    PreprocessingTest.h \
    TestBinarySearch.h \
    TestStateMachine.h

SOURCES += \
    PreprocessingTest.cpp \
    TestBinarySearch.cpp \
    ../src/AppConfig.cpp \
    ../src/Dfa.cpp \
    ../src/DfaCell.cpp \
    ../src/EventLoop.cpp \
    ../src/IFunctor.cpp \
    ../src/InternalCriticalSection.cpp \
    ../src/MRE_Linux.cpp \
    ../src/Nfa.cpp \
    ../src/NfaRaw.cpp \
    ../src/Re2Postfix.cpp \
    ../src/SrmApplication.cpp \
    main.cpp \
    TestStateMachine.cpp

INCLUDEPATH += ../include

LIBS += -lcppunit -lpthread





