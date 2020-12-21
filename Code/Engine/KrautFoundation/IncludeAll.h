#ifndef AE_FOUNDATION_INCLUDEALL_H
#define AE_FOUNDATION_INCLUDEALL_H

#pragma once

#include "Defines.h"
#include "Types.h"

#include "Basics/Checks.h"

#include "Memory/Memory.h"
#include "Memory/MemoryManagement.h"

#include "Containers/Deque.h"
#include "Containers/Array.h"
#include "Containers/HybridArray.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/Stack.h"

#include "Math/Math.h"
#include "Math/Vec3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/RandomNumberGenerator.h"

#include "Strings/String.h"
#include "Strings/StaticString.h"
#include "Strings/HybridString.h"
#include "Strings/LocaleFixer.h"
#include "Strings/PathFunctions.h"
#include "Strings/StringFunctions.h"

#include "Utility/Flags.h"
#include "Utility/Enum.h"
#include "Utility/EnumerableClass.h"
#include "Utility/SingletonClass.h"

#include "Communication/Event.h"
#include "Communication/GlobalEvent.h"

#include "Configuration/VariableRegistry.h"
#include "Configuration/Startup.h"
#include "Configuration/CVar.h"

#include "Logging/Log.h"
#include "Logging/ConsoleWriter.h"
#include "Logging/HTMLWriter.h"
#include "Logging/VisualStudioWriter.h"

#include "Streams/Streams.h"
#include "Streams/MemoryStream.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/Source/FileSystemWindows.h"
#include "FileSystem/FileIn.h"
#include "FileSystem/FileOut.h"

#include "Plugin/PluginManager.h"

#endif