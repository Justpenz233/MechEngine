//
// Created by MarvelLi on 2024/3/25.
//

#pragma once


#ifdef ME_BUILD_DEBUG
#define FORCEINLINE inline 	/* Don't force code to be inline */
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
#else
	#define ENGINE_API
	#define EDITOR_API
#endif