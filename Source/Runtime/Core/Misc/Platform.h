//
// Created by MarvelLi on 2024/3/25.
// Edited by Rick Zhang on Sep 05, 2024
//

#pragma once


/*#ifdef ME_BUILD_DEBUG*/
/*#define FORCEINLINE inline*/ 	/* Don't force code to be inline */
/*#endif*/


// Define ME_BUILD_TYPE based on the build type
#if defined(ME_BUILD_DEBUG)
	#define ME_BUILD_TYPE "Debug"
#elif defined(ME_BUILD_RELEASE)
	#define ME_BUILD_TYPE "Release"
#elif defined(ME_BUILD_RELWITHDEBINFO)
	#define ME_BUILD_TYPE "RelWithDebInfo"
#else
	#define ME_BUILD_TYPE "Unknown"
#endif

//Enable LOG_DEBUG if it's a Debug build type
#if defined(ME_BUILD_DEBUG) || defined(ME_BUILD_RELWITHDEBINFO)
	#define ENABLE_LOG_DEBUG 1
#else
	#define ENABLE_LOG_DEBUG 0
#endif



#include <sstream>


#if ENABLE_LOG_DEBUG
	#define LOG_DEBUG(...) MechEngine::Logger::Get().GetDefaultLogger()->debug(__VA_ARGS__)
#else
	#define LOG_DEBUG(...) // No operation
#endif


#ifndef FORCEINLINE
	#if (_MSC_VER >= 1200) //MSVC compiler
	#define FORCEINLINE __forceinline
	#else
			#ifdef __GNUG__
					#define FORCEINLINE inline __attribute__((always_inline))
			#endif
	#endif
#endif

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef uint8
typedef unsigned char uint8;
#endif

// Export dll symbol
#ifdef _MSC_VER
	#ifdef ENGINE_API_EXPORT
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif

	#ifdef EDITOR_API_EXPORT
		#define EDITOR_API __declspec(dllexport)
	#else
		#define EDITOR_API __declspec(dllimport)
	#endif

	#ifdef USER_API_EXPORT
		#define USER_API __declspec(dllexport)
	#else
		#define USER_API __declspec(dllimport)
	#endif

#else
	#define ENGINE_API
	#define EDITOR_API
#endif
