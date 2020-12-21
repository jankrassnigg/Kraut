// #define USE_BULLET

#include <KrautFoundation/IncludeAll.h>
#include <KrautGraphics/VertexArray/Declarations.h>
#include <KrautGraphics/glim/glim.h>
#include <KrautGraphics/ShaderManager/ShaderManager.h>
#include <KrautFoundation/Containers/HybridArray.h>
#include <KrautGraphics/TextureManager/TextureManager.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautEditorBasics/Plugin.h>
#include <KrautEditorBasics/Picking/Picking.h>
#include <KrautGraphics/ShaderManager/ShaderManager.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/glim/glim.h>
#include <QtGui>
#include <QToolButton>
#include <QToolBar>
#include <QFileDialog>
#include <QShortcut>
#include <QMainWindow>
#include <QApplication>
#include <QDockWidget>
//#include <QGraphicsView>
#include <QListView>
#include <QStringListModel>
#include <QMessageBox>
#include <QMenu>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <omp.h>
#include <QProgressDialog>
#include <KrautFoundation/DynamicTree/DynamicOctree.h>


#ifdef USE_BULLET
#include <Bullet/btBulletDynamicsCommon.h>
#endif






