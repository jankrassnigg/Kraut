// #define USE_BULLET

#include <KrautEditorBasics/Picking/Picking.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautFoundation/Containers/HybridArray.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/ShaderManager/ShaderManager.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/TextureManager/TextureManager.h>
#include <KrautGraphics/VertexArray/Declarations.h>
#include <KrautGraphics/glim/glim.h>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QShortcut>
#include <QToolBar>
#include <QToolButton>
#include <QtGui>
//#include <QGraphicsView>
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <KrautGraphics/DynamicTree/DynamicOctree.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStringListModel>
#include <omp.h>


#ifdef USE_BULLET
#  include <Bullet/btBulletDynamicsCommon.h>
#endif
