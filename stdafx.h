// stdafx.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets qui sont utilisés fréquemment,
// et sont rarement modifiés
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclure les en-têtes Windows rarement utilisés
// Fichiers d'en-tête Windows :
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <aclapi.h>
#include <sddl.h>
#include <wincrypt.h>
#include <wininet.h>
#include <shellapi.h>
#include <ctime>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wininet.lib")
#if _DEBUG
#pragma comment(lib, "lua53d.lib")
#else
#pragma comment(lib, "lua53.lib")
#endif

// Fichiers d'en-tête C RunTime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>



