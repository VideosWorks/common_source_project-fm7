message("* qt/sc3000")

set(s_qt_sc3000_headers
	  menuclasses.h
)

if(USE_QT_5)
  QT5_WRAP_CPP(s_qt_sc3000_headers_MOC ${s_qt_sc3000_headers})
else()
  QT4_WRAP_CPP(s_qt_sc3000_headers_MOC ${s_qt_sc3000_headers})
endif()

add_library(qt_sc3000
	  MainWindow.cpp
	  ${s_qt_sc3000_headers_MOC}
)


