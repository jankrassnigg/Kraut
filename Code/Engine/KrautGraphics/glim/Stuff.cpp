/*
** GLIM - OpenGL Immediate Mode
** Copyright Jan Krassnigg (Jan@Krassnigg.de)
** For more details, see the included Readme.txt.
*/

#include "glim.h"
#include <iostream>
#include <stdio.h>

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

namespace NS_GLIM
{
	using namespace std;

	

	#ifdef AE_RENDERAPI_OPENGL
		GLIM_API g_GlimAPI = GLIM_OPENGL;
	#else
		GLIM_API g_GlimAPI = GLIM_NONE;
	#endif

	GLIM_CALLBACK GLIM_Interface::s_StateChangeCallback = nullptr;
	bool GLIM_Interface::s_bForceWireframe = false;

	glimException::glimException (const string &err) : runtime_error (err)
	{
		printf (err.c_str ());
		cerr << err;

#ifdef WIN32
		MessageBox (nullptr, err.c_str (), "GLIM - Error", MB_ICONERROR);
#endif
	}



}
