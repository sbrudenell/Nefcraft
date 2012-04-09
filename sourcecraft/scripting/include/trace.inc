/**
 * vim: set ai et ts=4 sw=4 syntax=sourcepawn :
 * File: trace.inc
 * Description: Function library to trace execution.
 * Author(s): Naris (Murray Wilson)
 */

#if defined _trace_included
 #endinput
#endif
#define _trace_included

// Define _TRACE to enable trace logging for debugging
//#define _TRACE

/**
 * Define Macros to call TraceMessage() with various verbosity levels.
 */
#if defined _TRACE
    #define TraceAudit(%1)      TraceMessage(1,%1)
    #define TraceCritical(%1)   TraceMessage(2,%1)
    #define TraceError(%1)      TraceMessage(3,%1)
    #define TraceProblem(%1)    TraceMessage(4,%1)
    #define TraceWarning(%1)    TraceMessage(5,%1)
    #define TraceInfo(%1)       TraceMessage(6,%1)
    #define TraceDecision(%1)   TraceMessage(7,%1)
    #define TraceDebug(%1)      TraceMessage(8,%1)
    #define TraceDetail(%1)     TraceMessage(9,%1)
    #define TraceCat(%1,%2)     SetTraceCategory(%1); TraceMessage(9,%2); ResetTraceCategory()
    #define Trace(%1)           TraceMessage(9,%1)
#else // Define macros to make the trace natives go away
    #define TraceInto(%1);
    #define TraceDump(%1);
    #define TraceReturn(%1);
    #define SetTraceCat(%1);
    #define SetTraceCategory(%1);
    #define ResetTraceCategory(%1);

    #define TraceAudit(%1);
    #define TraceCritical(%1);
    #define TraceError(%1);
    #define TraceProblem(%1);
    #define TraceWarning(%1);
    #define TraceInfo(%1);
    #define TraceDecision(%1);
    #define TraceDebug(%1);
    #define TraceDetail(%1);
    #define TraceCat(%1,%2);
    #define Trace(%1);
#endif

#if !defined _TRACE
 #endinput
#endif

/**
 * Trace Into a Function
 * @param class: The name of the class/module tracing into.
 * @param method: The name of the method tracing into.
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 */
native TraceInto(const String:class[], const String:method[], const String:fmt[]="", any:...);

/**
 * Trace Return (out of) a Function
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 */
native TraceReturn(const String:fmt[]="", any:...);

/**
 * TraceMessage - Log a Trace Message (inside) a Function
 * @param verbosity: The verbosity level to use.
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 */
native TraceMessage(verbosity, const String:fmt[]="", any:...);

/**
 * Dump the Trace Stack
 * @param fmt: The format string for the message
 * @param ...: Format arguments (if any)
 * @noreturn
 */
native TraceDump(const String:fmt[]="", any:...);

/**
 * Sets the current category
 * @param category: String to place into the category.
 * @noreturn
 */
native SetTraceCategory(const String:category[]="");

/**
 * Resets the current category to nothing
 * @noreturn
 */
native ResetTraceCategory();

/**
 * Retrieves the current category
 * @param category: String to place the category into.
 * @param maxlength: The size of the category buffer.
 * @noreturn
 */
native GetTraceCategory(String:category[], maxlength);

/**
 * Retrieves the name of the current trace function
 * @param name: String to place the name into.
 * @param maxlength: The size of the name buffer.
 * @noreturn
 */         
native GetTraceMethod(String:name[], maxlength);

/* DO NOT EDIT BELOW THIS LINE */

public SharedPlugin:__pl_trace = 
{
	name = "trace",
	file = "trace.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_trace_SetNTVOptional()
{
	MarkNativeAsOptional("TraceInto");
	MarkNativeAsOptional("TraceReturn");
	MarkNativeAsOptional("TraceMessage");
	MarkNativeAsOptional("TraceDump");
	MarkNativeAsOptional("SetTraceCategory");
	MarkNativeAsOptional("GetTraceCategory");
	MarkNativeAsOptional("ResetTraceCategory");
	MarkNativeAsOptional("GetTraceMethod");
}
#endif
