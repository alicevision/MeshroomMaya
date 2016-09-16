// /!\ Include this file instead of QtCore / QtGui

// X11 defines CamelCase macros that clash with Qt.
// To avoid include-order based issues, we undefine those problematic names.

#if defined(LINUX)
#if defined(None)
#undef None
#endif
#if defined(KeyPress)
#undef KeyPress
#endif
#if defined(KeyRelease)
#undef KeyRelease
#endif
#if defined(FocusIn)
#undef FocusIn
#endif
#if defined(FocusOut)
#undef FocusOut
#endif
#if defined(FontChange)
#undef FontChange
#endif
#if defined(CursorShape)
#undef CursorShape
#endif
#if defined(Status)
#undef Status
#endif
#if defined(Bool)
#undef Bool
#endif
#if defined(Unsorted)
#undef Unsorted
#endif
#if defined(GrayScale)
#undef GrayScale
#endif
#endif
#include <QtCore/QtCore>
#include <QtGui/QtGui>
