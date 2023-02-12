QT += widgets network
CONFIG += c++17
CONFIG += resources_big
DEFINES += QT_MESSAGELOGCONTEXT
requires(qtConfig(filedialog))

RC_ICONS = QEditorIcon.ico

HEADERS       = \
    include/common/Constants.h \
    include/common/Logger.h \
    include/common/RangeMap.h \
    include/common/Settings.h \
    include/common/SingleApp.h \
    include/common/Utils.h \
    include/diff/diff_match_patch/diff_match_patch.h \
    include/diff/Diff.h \
    include/file/FileEncoding.h \
    include/file/FileRecorder.h \
    include/file/FileType.h \
    include/file/RecentFiles.h \
    include/hierarchy/AnfNodeHierarchy.h \
    include/hierarchy/AnfNodeHierarchyScene.h \
    include/hierarchy/AnfNodeItem.h \
    include/hierarchy/Arrow.h \
    include/hierarchy/FunctionHierarchy.h \
    include/hierarchy/FunctionHierarchyScene.h \
    include/hierarchy/FunctionItem.h \
    include/hierarchy/HierarchyScene.h \
    include/hierarchy/NodeItem.h \
    include/parser/IParser.h \
    include/parser/IrParser.h \
    include/view/ComboView.h \
    include/view/DockView.h \
    include/view/EditView.h \
    include/view/ExplorerTreeView.h \
    include/view/GotoLineDialog.h \
    include/view/MainTabView.h \
    include/view/MainWindow.h \
    include/view/OutlineList.h \
    include/view/RichEditView.h \
    include/view/SearchDialog.h \
    include/view/SearchResultItem.h \
    include/view/SearchResultList.h \
    include/view/TextHighlighter.h \
    include/view/Toast.h \
    include/win/WinTheme.h

SOURCES       = \
    src/Entry.cpp \
    src/common/Constants.cpp \
    src/common/Settings.cpp \
    src/diff/diff_match_patch/diff_match_patch.cpp \
    src/diff/Diff.cpp \
    src/file/FileEncoding.cpp \
    src/file/FileRecorder.cpp \
    src/file/RecentFiles.cpp \
    src/hierarchy/AnfNodeHierarchy.cpp \
    src/hierarchy/AnfNodeHierarchyScene.cpp \
    src/hierarchy/AnfNodeItem.cpp \
    src/hierarchy/Arrow.cpp \
    src/hierarchy/FunctionHierarchy.cpp \
    src/hierarchy/FunctionHierarchyScene.cpp \
    src/hierarchy/FunctionItem.cpp \
    src/hierarchy/HierarchyScene.cpp \
    src/hierarchy/NodeItem.cpp \
    src/parser/IrParser.cpp \
    src/view/ComboView.cpp \
    src/view/DockView.cpp \
    src/view/EditView.cpp \
    src/view/ExplorerTreeView.cpp \
    src/view/GotoLineDialog.cpp \
    src/view/MainTabView.cpp \
    src/view/MainWindow.cpp \
    src/view/OutlineList.cpp \
    src/view/RichEditView.cpp \
    src/view/SearchDialog.cpp \
    src/view/SearchResultItem.cpp \
    src/view/SearchResultList.cpp \
    src/view/TextHighlighter.cpp \
    src/view/Toast.cpp

RESOURCES = QEditor.qrc

FORMS += \
    form/GotoLineDialog.ui \
    form/SearchDialog.ui

INCLUDEPATH += \
    ./include/ \
    ./include/common/ \
    ./include/file/ \
    ./include/hierarchy/ \
    ./include/parser/ \
    ./include/view/ \
    ./include/diff/ \
    ./include/diff/diff_match_patch/ \

VERSION = "0.0.2"
QMAKE_TARGET_PRODUCT = "QEditor"
QMAKE_TARGET_COMPANY = "Q"
QMAKE_TARGET_DESCRIPTION = "QEditor"
QMAKE_TARGET_COPYRIGHT = Copyright(2022)
