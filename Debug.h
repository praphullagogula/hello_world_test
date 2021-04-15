/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2004-2005 Adobe Systems Incorporated                  */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

#ifndef DVACORE_DEBUG_DEBUG_H
#define DVACORE_DEBUG_DEBUG_H

#include "dvacore/config/PreConfig.h"

#pragma once

#include "dvacore/debug/DebugTrace.h"
#include "dvacore/debug/DefaultAssert.h"
#include "dvacore/threads/ThreadID.h"

#if defined(DVA_OS_IOS)
#include <signal.h>
#endif

// [2016-03-28 - jzibble/dmccormi] Globally address BOOST_STATIC_ASSERT_MSG errors in boost 1.60 (error: implicit instantiation of undefined template).
#include <boost/optional/optional_io.hpp>

// There is a bug with the Intel Compiler in Visual Studio 2012 and 2013 (release only) which is supposed to be fixed in VS 2015 RTM.
// _Yarn<wchar_t> specialization does not link if /Zc:wchar_t- is specified
// See: https://connect.microsoft.com/VisualStudio/feedback/details/817221/yarn-wchar-t-specialization-does-not-link-if-zc-wchar-t-is-specified
// This bug manifests in the DVA_ASSERT_MSG, DVA_VALIDATE_MSG and DVA_TRACE_ALWAYS macros and is fixed by not calling the _MSG version
// of the macros when the Intel compiler is used.
// 
// jkrueger 2015-03-24
#if (defined(__INTEL_COMPILER) && (_MSC_VER < 1900) && (!DVA_DEBUG))
#define DVA_MSVC_INTEL_HACK_DISABLE_ASSERT_MSG 1
#endif

namespace dvacore {
namespace debug {

/**
**	Validate Macros
**	These macros never wash out in code. Use them for tests you want to keep even in
**	release builds. The debug variant of DVA_VALIDATE may expand to something different
**	than the release variant but both release and debug will expand to at least a check.
**
**	DVA_VALIDATE
**	@param exp is the expression to test. If the expression fails the validate is triggered.
**
**	DVA_VALIDATE_MSG
**	@param exp is the expression to test. If the expression fails the validate is triggered.
**	@param (char*) msg is the message to display if the validate triggers.
*/

#if DVA_DEBUG_VALIDATE
	#define DVA_VALIDATE(exp)							DVA_VALIDATE_(exp,#exp)
	#define DVA_VALIDATE_MSG(exp,msg)					DVA_VALIDATE_MSG_(exp,#exp,msg)
#else
	#define DVA_VALIDATE(exp)							DVA_VALIDATE_(exp,#exp)
	#define DVA_VALIDATE_MSG(exp,msg)					DVA_VALIDATE_MSG_(exp,#exp,msg)
#endif

/**
**	Assert Macros
**
**	DVA_ASSERT
**	Debug - Displays dialog. Always logs failure.
**	Release - NOP (washes out)
**	@param exp is the expression to test. If the expression fails the assert is triggered.
**
**	DVA_ASSERT_MSG
**	Debug - Displays dialog. Always logs failure.
**	Release - NOP (washes out)
**	@param exp is the expression to test. If the expression fails the assert is triggered.
**	@param (char*) msg is the message to display if the assert triggers.
**
**	DVA_ASSERT_ALWAYS
**	Debug - Displays dialog. Always logs failure.
**	Release - Displays dialog if asserts are enabled. Always logs failure.
**	@param exp is the expression to test. If the expression fails the assert is triggered.
**
**	DVA_ASSERT_MSG_ALWAYS
**	Debug - Displays dialog. Always logs failure.
**	Release - Displays dialog if asserts are enabled. Always logs failure.
**	@param exp is the expression to test. If the expression fails the assert is triggered.
**	@param (char*) msg is the message to display if the assert triggers.
*/

#if DVA_DEBUG_ASSERT
	#define DVA_ASSERT(exp)								DVA_ASSERT_(exp,#exp,true)
	#define DVA_ASSERT_MSG(exp,msg)						DVA_ASSERT_MSG_(exp,#exp,msg,true)
	#define DVA_ASSERT_NO_LOG(exp)								DVA_ASSERT_(exp,#exp,false)
	#define DVA_ASSERT_MSG_NO_LOG(exp,msg)						DVA_ASSERT_MSG_(exp,#exp,msg,false)
#else
	#define DVA_ASSERT(exp)								((void)(0 && (exp)))
	#define DVA_ASSERT_MSG(exp,msg)						((void)(0 && (exp)))
	#define DVA_ASSERT_NO_LOG(exp)						DVA_ASSERT(exp)
	#define DVA_ASSERT_MSG_NO_LOG(exp,msg)				DVA_ASSERT_MSG(exp,msg)
#endif

#define DVA_ASSERT_ALWAYS(exp)							DVA_ASSERT_(exp,#exp,true)
#define DVA_ASSERT_MSG_ALWAYS(exp,msg)					DVA_ASSERT_MSG_(exp,#exp,msg,true)
#define DVA_ASSERT_ALWAYS_NO_LOG(exp)							DVA_ASSERT_(exp,#exp,false)
#define DVA_ASSERT_MSG_ALWAYS_NO_LOG(exp,msg)					DVA_ASSERT_MSG_(exp,#exp,msg,false)

/**
**	Trace Macros
**
**	How do these guys work?  You have a master volume and a volume per catagory.  These are set via the console or the Trace Database.xml file created
**	by the console.  Each new catagory defaults to volume 1.  And you master volume defaults to 10.  Within each catagory you can say when you should here
**	a message.  Here is an example:  DVA_TRACE("MyCatgory", 5, "This is a message");  MyCatgory has a default volume of 1.  This message is logged at volume 5
**	so you won't here this message.  You can type trace.set MyCategory 6 in the console and that will mean that all messages in MyCategory that are logged
**	up to volume 6 show up in the consoel.  However, the master volume trumps that...  If you have a master volume of 2 you won't see any messages traced
**	at volumes above 2.  This gives you a ton of flexability in what you see in your log windows.  To create a new catagory just use it in DVA_TRACE.  
**	probably should use a notation like dvacore.threads so we don't collide.  The file "Trace Database.xml" will show up in your prefs folder.  You can
**	control the enabled state and the volume for any catagories that have been traced once.
**
**	DVA_TRACE
**	Debug - Logs message.
**	Release - NOP
**	@param (char*) cat is the category that the trace relates to. (no spaces)
**	@param (int) vol is the volume level required to hear this trace.
**	@param (char*) msg is the message for this trace.
**
**	DVA_TRACE_IF
**	Debug - Logs message if expression is true.
**	Release - NOP
**	@param (char*) cat is the category that the trace relates to. (no spaces)
**	@param (int) vol is the volume level required to hear this trace.
**	@param exp is the expression to test. If the expression is true the trace triggers.
**	@param (char*) msg is the message for this trace.
**
**	DVA_TRACE_ALWAYS
**	Debug - Logs message.
**	Release - Logs message.
**	@param (char*) cat is the category that the trace relates to. (no spaces)
**	@param vol is the volume level required to hear this trace.
**	@param (char*) msg is the message for this trace.
**
**	DVA_TRACE_IF_ALWAYS
**	Debug - Logs message.
**	Release - Logs message if expression is true.
**	@param (char*) cat is the category that the trace relates to. (no spaces)
**	@param (int) vol is the volume level required to hear this trace.
**	@param exp is the expression to test. If the expression is true the trace triggers.
**	@param (char*) msg is the message for this trace.
*/

#if DVA_DEBUG_TRACE
	#define DVA_TRACE(cat,vol,msg)						DVA_TRACE_(cat,vol,true,msg)
	#define DVA_TRACE_IF(cat,vol,exp,msg)				DVA_TRACE_(cat,vol,exp,msg)
#else
	#define DVA_TRACE(cat,vol,msg)						((void)0)
	#define DVA_TRACE_IF(cat,vol,exp,msg)				((void)(0 && (exp)))
#endif

#if defined(DVA_MSVC_INTEL_HACK_DISABLE_ASSERT_MSG)
	#define DVA_TRACE_ALWAYS(cat,vol,msg)					((void)0)
	#define DVA_TRACE_IF_ALWAYS(cat,vol,exp,msg)			((void)(0 && (exp)))
#else
	#define DVA_TRACE_ALWAYS(cat,vol,msg)					DVA_TRACE_(cat,vol,true,msg)
	#define DVA_TRACE_IF_ALWAYS(cat,vol,exp,msg)			DVA_TRACE_(cat,vol,exp,msg)
#endif

/**
**	Debug only macro for wrapping code that should only execute
**	in debug mode.
**
**	@usage
**	DVA_DEBUG_ONLY(SomeFunction(args));
*/
#if DVA_DEBUG
	#define DVA_DEBUG_ONLY(code)						code
#else
	#define DVA_DEBUG_ONLY(code)						((void)0)
#endif

/**
**	Macro for displaying the name of the current function.
**
**	@usage
**	For example, when creating a local scoped RecursiveMutex:
**	dvacore::threads::RecursiveMutex::ScopedLock lock(mLock,DVA_FUNCTION)
*/
#define DVA_FUNCTION									DVA_PRIV_FUNCTION

//	=================================================================
//	PRIVATE!!! - DO NOT USE ANYTHING BELOW THIS LINE IN YOUR CODE!!!
//	=================================================================

//
// DELAY
//
#ifdef __GNUC__
#define DVA_PRIV_FUNCTION __PRETTY_FUNCTION__
#else
#define DVA_PRIV_FUNCTION __FUNCTION__
#endif

#define DVA_VALIDATE_(exp,xstr)							DVA_INTERNAL_ASSERT_(exp,xstr,__FILE__,__LINE__,DVA_PRIV_FUNCTION,true)
#if defined(DVA_MSVC_INTEL_HACK_DISABLE_ASSERT_MSG)
	#define DVA_VALIDATE_MSG_(exp, xstr, msg)			DVA_INTERNAL_ASSERT_(exp, xstr, __FILE__, __LINE__, DVA_PRIV_FUNCTION, true)
#else
	#define DVA_VALIDATE_MSG_(exp, xstr, msg)			DVA_INTERNAL_ASSERT_MSG_(exp, xstr, msg, __FILE__, __LINE__, DVA_PRIV_FUNCTION, true)
#endif

#define DVA_ASSERT_(exp,xstr,log)						DVA_INTERNAL_ASSERT_(exp,xstr,__FILE__,__LINE__,DVA_PRIV_FUNCTION,log)
#if defined(DVA_MSVC_INTEL_HACK_DISABLE_ASSERT_MSG)
	#define DVA_ASSERT_MSG_(exp,xstr,msg,log)			DVA_INTERNAL_ASSERT_(exp,xstr,__FILE__,__LINE__,DVA_PRIV_FUNCTION,log)
#else
	#define DVA_ASSERT_MSG_(exp,xstr,msg,log)			DVA_INTERNAL_ASSERT_MSG_(exp,xstr,msg,__FILE__,__LINE__,DVA_PRIV_FUNCTION,log)
#endif

#define DVA_TRACE_(cat,vol,exp,msg)						DVA_INTERNAL_TRACE_(cat,vol,exp,msg,__FILE__,__LINE__,DVA_PRIV_FUNCTION)
#define DVA_PRIV_RETHROW_MSG_DELAY						DVA_PRIV_RETHROW_MSG_IMPL

//
// IMPL
//

// DW_M2D begin
#if defined(DVA_OS_WIN)
#	define DVA_DEBUG_OUTPUT(message) ::OutputDebugStringA(message)
#elif defined(DVA_OS_MAC)
#	define DVA_DEBUG_OUTPUT(message) fputs(message, stdout)
#elif defined(DVA_OS_LINUX)
#	define DVA_DEBUG_OUTPUT(message) puts(message)
#elif defined(DVA_OS_IOS)
#	define DVA_DEBUG_OUTPUT(message) puts(message)
#endif

// Different ways of programmatically breaking into the debugger
//
#ifdef _MSC_VER
#    define    DVA_DEBUG_BREAK_IMPL()    __debugbreak()
#elif defined(DVA_TARGET_ARCH_X64) || defined(DVA_TARGET_ARCH_X86)
    // For x86/x64 architectures, prefer the INT 3 method
#    define    DVA_DEBUG_BREAK_IMPL()    __asm__("int $3")
#else
    // SIGTRAP works better than __builtin_trap() on ARM/iOS (can continue)
#    define    DVA_DEBUG_BREAK_IMPL()    raise(SIGTRAP)
#endif
    

#define DVA_DEBUG_BREAK() do{ if ( dvacore::debug::IsDebuggerAttached() ) { DVA_DEBUG_BREAK_IMPL(); } } while(0)
// DW_M2D end

#define DVA_INTERNAL_ASSERT_(exp,xstr,file,line,func,log)				\
	do																\
	{																\
		static bool sDisabled = false;								\
		if (!sDisabled && !(exp))									\
		{															\
			if (dvacore::debug::BreakAfterAssertDialog(sDisabled, xstr, dvacore::utility::CharAsUTF8(""), file, line, func, log)) \
			{														\
				DVA_DEBUG_BREAK();									\
			}														\
		}															\
	} while (0)

// Note:  msg may be something odd like:
// "(" << x << "," << y << ")"
#define DVA_INTERNAL_ASSERT_MSG_(exp,xstr,msg,file,line,func,log)		\
	do																\
	{																\
		static bool sDisabled = false;								\
		if (!sDisabled && !(exp))									\
		{															\
			dvacore::StdOutputStringStream stream;					\
			stream << msg << dvacore::utility::ENDL;				\
			if (dvacore::debug::BreakAfterAssertDialog(sDisabled, xstr, dvacore::utility::CharAsUTF8(stream.str().c_str()), file, line, func, log)) \
			{														\
				DVA_DEBUG_BREAK();									\
			}														\
		}															\
	} while (0)
	

//	Trace needs to be fast so we format the message here in the
//	macro so we can avoid extra buffer copies.
//	We now store the value of the previous call to TraceEnabled
//	to avoid always checking as the time it takes to lookup
//	can really add up when done often. Changes in volume can
//	only occur as a result of UI actions in the console panel
//	and thus are very rare. Atomics are intentionally not
//	used here to reduce the penalty of trace and if we ever
//	get caught in the small race condition this creates the
//	worst that might happen is one trace too few or too many.

#define DVA_INTERNAL_TRACE_(cat,vol,expr,msg,file,line,func)		\
	do																\
	{																\
		std::int32_t volume = vol;									\
		bool shouldTrace = false;									\
		if ((expr) != 0)											\
		{															\
			static int sTraceChangeCount = -1;						\
			int changeCount = dvacore::debug::TraceChangeCount();	\
			std::swap(sTraceChangeCount, changeCount);				\
																	\
			static bool sEnabled = false;							\
			if (sTraceChangeCount != changeCount)					\
			{														\
				sEnabled = dvacore::debug::TraceEnabled(			\
						dvacore::utility::CharAsUTF8(cat), volume);	\
			}														\
			shouldTrace = sEnabled;									\
		}															\
																	\
		if (shouldTrace)											\
		{															\
			dvacore::threads::ThreadID thread =						\
				dvacore::threads::GetCurrentThreadID();				\
			dvacore::StdOutputStringStream stream;					\
			stream << "<" << thread << "> ";						\
			stream << "<" << cat << "> ";							\
			stream << "<" << volume << "> ";						\
			stream << msg << dvacore::utility::ENDL;									\
			dvacore::debug::Trace(dvacore::utility::CharAsUTF8(stream.str().c_str()));			\
		}															\
	} while (0)

#define DVA_PRIV_RETHROW_MSG_IMPL(msg)								\
	do																\
	{																\
		static bool sDisabled = false;								\
		if (!sDisabled)												\
		{															\
			dvacore::StdOutputStringStream stream;					\
			stream << msg << dvacore::utility::ENDL;								\
			if (dvacore::debug::ThrowAfterUnhandledExceptionAlert(sDisabled, stream.str())) \
			{														\
				throw;												\
			}														\
		}															\
	} while (0)

} // namespace debug
} // namespace dvacore

#include "dvacore/config/PostConfig.h"

#endif

