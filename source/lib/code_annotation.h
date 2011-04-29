/* Copyright (c) 2010 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * macros for code annotation.
 */

#ifndef INCLUDED_CODE_ANNOTATION
#define INCLUDED_CODE_ANNOTATION

#include "lib/sysdep/compiler.h"
#include "lib/sysdep/arch.h"	// ARCH_AMD64

/**
 * mark a function local variable or parameter as unused and avoid
 * the corresponding compiler warning.
 * use inside the function body, e.g. void f(int x) { UNUSED2(x); }
 **/
#if ICC_VERSION
// NB: #pragma unused is documented but "unrecognized" when used;
// casting to void isn't sufficient, but the following is:
# define UNUSED2(param) param = param
#else
# define UNUSED2(param) (void)param
#endif

/**
 * mark a function parameter as unused and avoid
 * the corresponding compiler warning.
 * wrap around the parameter name, e.g. void f(int UNUSED(x))
 **/
#define UNUSED(param)


/**
 * "unreachable code" helpers
 *
 * unreachable lines of code are often the source or symptom of subtle bugs.
 * they are flagged by compiler warnings; however, the opposite problem -
 * erroneously reaching certain spots (e.g. due to missing return statement)
 * is worse and not detected automatically.
 *
 * to defend against this, the programmer can annotate their code to
 * indicate to humans that a particular spot should never be reached.
 * however, that isn't much help; better is a sentinel that raises an
 * error if if it is actually reached. hence, the UNREACHABLE macro.
 *
 * ironically, if the code guarded by UNREACHABLE works as it should,
 * compilers may flag the macro's code as unreachable. this would
 * distract from genuine warnings, which is unacceptable.
 *
 * even worse, compilers differ in their code checking: GCC only complains if
 * non-void functions end without returning a value (i.e. missing return
 * statement), while VC checks if lines are unreachable (e.g. if they are
 * preceded by a return on all paths).
 *
 * the implementation below enables optimization and automated checking
 * without raising warnings.
 **/
#define UNREACHABLE	// actually defined below.. this is for
# undef UNREACHABLE	// CppDoc's benefit only.

// this macro should not generate any fallback code; it is merely the
// compiler-specific backend for UNREACHABLE.
// #define it to nothing if the compiler doesn't support such a hint.
#define HAVE_ASSUME_UNREACHABLE 1
#if MSC_VERSION && !ICC_VERSION // (ICC ignores this)
# define ASSUME_UNREACHABLE __assume(0)
#elif GCC_VERSION >= 450
# define ASSUME_UNREACHABLE __builtin_unreachable()
#else
# define ASSUME_UNREACHABLE
# undef HAVE_ASSUME_UNREACHABLE
# define HAVE_ASSUME_UNREACHABLE 0
#endif

// compiler supports ASSUME_UNREACHABLE => allow it to assume the code is
// never reached (improves optimization at the cost of undefined behavior
// if the annotation turns out to be incorrect).
#if HAVE_ASSUME_UNREACHABLE && !CONFIG_PARANOIA
# define UNREACHABLE ASSUME_UNREACHABLE
// otherwise (or if CONFIG_PARANOIA is set), add a user-visible
// warning if the code is reached. note that abort() fails to stop
// ICC from warning about the lack of a return statement, so we
// use an infinite loop instead.
#else
# define UNREACHABLE\
	STMT(\
		debug_assert(0);	/* hit supposedly unreachable code */\
		for(;;){};\
	)
#endif

/**
convenient specialization of UNREACHABLE for switch statements whose
default can never be reached. example usage:
int x;
switch(x % 2)
{
	case 0: break;
	case 1: break;
	NODEFAULT;
}
**/
#define NODEFAULT default: UNREACHABLE


// generate a symbol containing the line number of the macro invocation.
// used to give a unique name (per file) to types made by cassert.
// we can't prepend __FILE__ to make it globally unique - the filename
// may be enclosed in quotes. PASTE3_HIDDEN__ is needed to make sure
// __LINE__ is expanded correctly.
#define PASTE3_HIDDEN__(a, b, c) a ## b ## c
#define PASTE3__(a, b, c) PASTE3_HIDDEN__(a, b, c)
#define UID__  PASTE3__(LINE_, __LINE__, _)
#define UID2__ PASTE3__(LINE_, __LINE__, _2)

/**
 * Compile-time debug_assert. Causes a compile error if the expression
 * evaluates to zero/false.
 *
 * No runtime overhead; may be used anywhere, including file scope.
 * Especially useful for testing sizeof types.
 *
 * @param expr Expression that is expected to evaluate to non-zero at compile-time.
 **/
#define cassert(expr) typedef static_assert_<(expr)>::type UID__
template<bool> struct static_assert_;
template<> struct static_assert_<true>
{
	typedef int type;
};

/**
 * @copydoc cassert(expr)
 *
 * This version must be used if expr uses a dependent type (e.g. depends on
 * a template parameter).
 **/
#define cassert_dependent(expr) typedef typename static_assert_<(expr)>::type UID__

/**
 * @copydoc cassert(expr)
 *
 * This version has a less helpful error message, but redefinition doesn't
 * trigger warnings.
 **/
#define cassert2(expr) extern u8 CASSERT_FAILURE[1][(expr)]

// indicate a class is noncopyable (usually due to const or reference members).
// example:
// class C {
//   NONCOPYABLE(C);
// public: // etc.
// };
// this is preferable to inheritance from boost::noncopyable because it
// avoids ICC 11 W4 warnings about non-virtual dtors and suppression of
// the copy assignment operator.
#define NONCOPYABLE(className)\
private:\
	className(const className&);\
	const className& operator=(const className&)

#if ICC_VERSION
# define ASSUME_ALIGNED(ptr, multiple) __assume_aligned(ptr, multiple)
#else
# define ASSUME_ALIGNED(ptr, multiple)
#endif

// annotate printf-style functions for compile-time type checking.
// fmtpos is the index of the format argument, counting from 1 or
// (if it's a non-static class function) 2; the '...' is assumed
// to come directly after it.
#if GCC_VERSION
# define PRINTF_ARGS(fmtpos) __attribute__ ((format (printf, fmtpos, fmtpos+1)))
# define VPRINTF_ARGS(fmtpos) __attribute__ ((format (printf, fmtpos, 0)))
# if CONFIG_DEHYDRA
#  define WPRINTF_ARGS(fmtpos) __attribute__ ((user("format, w, printf, " #fmtpos ", +1")))
# else
#  define WPRINTF_ARGS(fmtpos) /* not currently supported in GCC */
# endif
# define VWPRINTF_ARGS(fmtpos) /* not currently supported in GCC */
#else
# define PRINTF_ARGS(fmtpos)
# define VPRINTF_ARGS(fmtpos)
# define WPRINTF_ARGS(fmtpos)
# define VWPRINTF_ARGS(fmtpos)
// TODO: support _Printf_format_string_ for VC9+
#endif

// annotate vararg functions that expect to end with an explicit NULL
#if GCC_VERSION
# define SENTINEL_ARG __attribute__ ((sentinel))
#else
# define SENTINEL_ARG
#endif

/**
 * prevent the compiler from reordering loads or stores across this point.
 **/
#if ICC_VERSION
# define COMPILER_FENCE __memory_barrier()
#elif MSC_VERSION
# include <intrin.h>
# pragma intrinsic(_ReadWriteBarrier)
# define COMPILER_FENCE _ReadWriteBarrier()
#elif GCC_VERSION
# define COMPILER_FENCE asm volatile("" : : : "memory")
#else
# define COMPILER_FENCE
#endif


// try to define _W64, if not already done
// (this is useful for catching pointer size bugs)
#ifndef _W64
# if MSC_VERSION
#  define _W64 __w64
# elif GCC_VERSION
#  define _W64 __attribute__((mode (__pointer__)))
# else
#  define _W64
# endif
#endif


// C99-like restrict (non-standard in C++, but widely supported in various forms).
//
// May be used on pointers. May also be used on member functions to indicate
// that 'this' is unaliased (e.g. "void C::m() RESTRICT { ... }").
// Must not be used on references - GCC supports that but VC doesn't.
//
// We call this "RESTRICT" to avoid conflicts with VC's __declspec(restrict),
// and because it's not really the same as C99's restrict.
//
// To be safe and satisfy the compilers' stated requirements: an object accessed
// by a restricted pointer must not be accessed by any other pointer within the
// lifetime of the restricted pointer, if the object is modified.
// To maximise the chance of optimisation, any pointers that could potentially
// alias with the restricted one should be marked as restricted too.
//
// It would probably be a good idea to write test cases for any code that uses
// this in an even very slightly unclear way, in case it causes obscure problems
// in a rare compiler due to differing semantics.
//
// .. GCC
#if GCC_VERSION
# define RESTRICT __restrict__
// .. VC8 provides __restrict
#elif MSC_VERSION >= 1400
# define RESTRICT __restrict
// .. ICC supports the keyword 'restrict' when run with the /Qrestrict option,
//    but it always also supports __restrict__ or __restrict to be compatible
//    with GCC/MSVC, so we'll use the underscored version. One of {GCC,MSC}_VERSION
//    should have been defined in addition to ICC_VERSION, so we should be using
//    one of the above cases (unless it's an old VS7.1-emulating ICC).
#elif ICC_VERSION
# error ICC_VERSION defined without either GCC_VERSION or an adequate MSC_VERSION
// .. unsupported; remove it from code
#else
# define RESTRICT
#endif


// C99-style __func__
// .. newer GCC already have it
#if GCC_VERSION >= 300
// nothing need be done
// .. old GCC and MSVC have __FUNCTION__
#elif GCC_VERSION >= 200 || MSC_VERSION
# define __func__ __FUNCTION__
// .. unsupported
#else
# define __func__ "(unknown)"
#endif


// extern "C", but does the right thing in pure-C mode
#if defined(__cplusplus)
# define EXTERN_C extern "C"
#else
# define EXTERN_C extern
#endif


#if MSC_VERSION
# define INLINE __forceinline
#else
# define INLINE inline
#endif


#if MSC_VERSION
# define CALL_CONV __cdecl
#else
# define CALL_CONV
#endif


#if MSC_VERSION && !ARCH_AMD64
# define DECORATED_NAME(name) _##name
#else
# define DECORATED_NAME(name) name
#endif


// workaround for preprocessor limitation: macro args aren't expanded
// before being pasted.
#define STRINGIZE2(id) # id
#define STRINGIZE(id) STRINGIZE2(id)


//-----------------------------------------------------------------------------
// C++0x rvalue references (required for UniqueRange)

/**
 * expands to the type `rvalue reference to T'; used in function
 * parameter declarations. for example, UniqueRange's move ctor is:
 * UniqueRange(RVALUE_REF(UniqueRange) rvalue) { ... }
 **/
#define RVALUE_REF(T) T&&

/**
 * convert an rvalue to an lvalue
 **/
#define LVALUE(rvalue) rvalue	// in C++0x, a named rvalue reference is already an lvalue

/**
 * convert anything (lvalue or rvalue) to an rvalue
 **/
#define RVALUE(lvalue) std::move(lvalue)


#if !HAVE_CPP0X	// partial emulation

// lvalue references are wrapped in this class, which is the
// actual argument passed to the "move ctor" etc.
template<typename T>
class RValue
{
public:
	explicit RValue(T& lvalue): lvalue(lvalue) {}
	T& LValue() const { return lvalue; }

private:
	// (avoid "assignment operator could not be generated" warning)
	const RValue& operator=(const RValue&);

	T& lvalue;
};

// (to deduce T automatically, we need function templates)

template<class T>
static inline RValue<T> ToRValue(T& lvalue)
{
	return RValue<T>(lvalue);
}

template<class T>
static inline RValue<T> ToRValue(const T& lvalue)
{
	return RValue<T>((T&)lvalue);
}

template<class T>
static inline RValue<T> ToRValue(const RValue<T>& rvalue)
{
	return RValue<T>(rvalue.LValue());
}

#undef RVALUE_REF
#undef LVALUE
#undef RVALUE

#define RVALUE_REF(T) const RValue<T>&
#define LVALUE(rvalue) rvalue.LValue()
#define RVALUE(lvalue) ToRValue(lvalue)

#endif	// #if !HAVE_CPP0X

#endif	// #ifndef INCLUDED_CODE_ANNOTATION
