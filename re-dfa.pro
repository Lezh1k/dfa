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
    include/AppConfig.h \
    include/Commons.h \
    include/Dfa.h \
    include/DfaCell.h \
    include/EventLoop.h \
    include/EventLoopException.h \
    include/EventLoopExceptionInfo.h \
    include/FunctorWithoutResult.h \
    include/FunctorWithResult.h \
    include/IfstreamWrapper.h \
    include/IFunctor.h \
    include/InternalCriticalSection.h \
    include/Locker.h \
    include/MRE_Linux.h \
    include/MRE_Wrapper.h \
    include/Nfa.h \
    include/NfaRaw.h \
    include/Re2Postfix.h \
    include/SrmApplication.h \
    include/ThreadWrapper.h \
    include/TimeMeasurements.h \

SOURCES += \
    src/AppConfig.cpp \
    src/Dfa.cpp \
    src/DfaCell.cpp \
    src/EventLoop.cpp \
    src/IFunctor.cpp \
    src/InternalCriticalSection.cpp \
    src/main.cpp \
    src/MRE_Linux.cpp \
    src/Nfa.cpp \
    src/NfaRaw.cpp \
    src/Re2Postfix.cpp \
    src/SrmApplication.cpp \

INCLUDEPATH += include
LIBS += -lpthread
DEFINES+=TIME_MEASUREMENTS
