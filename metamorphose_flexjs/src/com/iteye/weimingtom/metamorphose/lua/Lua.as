/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/Lua.java#3 $
 * Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//see jillcode(Java Implementation of Lua Language, Jill):
//	http://code.google.com/p/jillcode/
//这里的代码移植自jillcode(Lua的Java实现，Jill):
//	http://code.google.com/p/jillcode/	
package com.iteye.weimingtom.metamorphose.lua 
{
	import com.iteye.weimingtom.metamorphose.java.DataOutputStream;
	import com.iteye.weimingtom.metamorphose.java.Enumeration;
	import com.iteye.weimingtom.metamorphose.java.IOException;
	import com.iteye.weimingtom.metamorphose.java.IllegalArgumentException;
	import com.iteye.weimingtom.metamorphose.java.InputStream;
	import com.iteye.weimingtom.metamorphose.java.NullPointerException;
	import com.iteye.weimingtom.metamorphose.java.NumberFormatException;
	import com.iteye.weimingtom.metamorphose.java.OutOfMemoryError;
	import com.iteye.weimingtom.metamorphose.java.OutputStream;
	import com.iteye.weimingtom.metamorphose.java.Reader;
	import com.iteye.weimingtom.metamorphose.java.Runtime;
	import com.iteye.weimingtom.metamorphose.java.RuntimeException;
	import com.iteye.weimingtom.metamorphose.java.Stack;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;
	
	import flash.utils.getQualifiedClassName;
	
	/**
	 * <p>
	 * Encapsulates a Lua execution environment.  A lot of Jill's public API
	 * manifests as public methods in this class.  A key part of the API is
	 * the ability to call Lua functions from Java (ultimately, all Lua code
	 * is executed in this manner).
	 * </p>
	 *
	 * <p>
	 * The Stack
	 * </p>
	 *
	 * <p>
	 * All arguments to Lua functions and all results returned by Lua
	 * functions are placed onto a stack.  The stack can be indexed by an
	 * integer in the same way as the PUC-Rio implementation.  A positive
	 * index is an absolute index and ranges from 1 (the bottom-most
	 * element) through to <var>n</var> (the top-most element),
	 * where <var>n</var> is the number of elements on the stack.  Negative
	 * indexes are relative indexes, -1 is the top-most element, -2 is the
	 * element underneath that, and so on.  0 is not used.
	 * </p>
	 *
	 * <p>
	 * Note that in Jill the stack is used only for passing arguments and
	 * returning results, unlike PUC-Rio.
	 * </p>
	 *
	 * <p>
	 * The protocol for calling a function is described in the {@link #call}
	 * method.  In brief: push the function onto the stack, then push the
	 * arguments to the call.
	 * </p>
	 *
	 * <p>
	 * The methods {@link #push}, {@link #pop}, {@link #value},
	 * {@link #getTop}, {@link #setTop} are used to manipulate the stack.
	 * </p>
	 */
	public final class Lua
	{		
		public static const D:Boolean = false; 
		
		/** Version string. */
		public static const VERSION:String = "Lua 5.1 (Jill 1.0.1)";
		
		public static const RELEASE:String = "Lua 5.1.4 (Jill 1.0.1)";
		public static const VERSION_NUM:int = 501;
		public static const COPYRIGHT:String = "Copyright (C) 1994-2008 Lua.org, PUC-Rio (Copyright (C) 2006 Nokia Corporation and/or its subsidiary(-ies))";
		/** http://www.ravenbrook.com */
		public static const AUTHORS:String = "R. Ierusalimschy, L. H. de Figueiredo & W. Celes (Ravenbrook Limited)";
		
		/** Table of globals (global variables).  This actually shared across
		* all threads (with the same main thread), but kept in each Lua
		* thread as an optimisation.
		*/
		private var _global:LuaTable = null;
		private var _registry:LuaTable = null;
		
		/** Reference the main Lua thread.  Itself if this is the main Lua
		* thread.
		*/
		private var _main:Lua = null;

		/** VM data stack.
		*/
		private var _stack:Array = new Array(); //TODO:=0? Slot[] 
		/**
		* One more than the highest stack slot that has been written to
		* (ever).
		* Used by {@link #stacksetsize} to determine which stack slots
		* need nilling when growing the stack.
		*/
		private var _stackhighwater:int = 0;   // = 0;
		/**
		* Number of active elemements in the VM stack.  Should always be
		* <code><= stack.length</code>.
		*/
		private var _stackSize:int = 0;        // = 0;
		/**
		* The base stack element for this stack frame.  If in a Lua function
		* then this is the element indexed by operand field 0; if in a Java
		* functipn then this is the element indexed by Lua.value(1).
		*/
		private var _base:int = 0;     // = 0;

		//TODO:public
		private var _nCcalls:int = 0;  // = 0;
		/** Instruction to resume execution at.  Index into code array. */
		private var _savedpc:int = 0;  // = 0;
		/**
		* Vector of CallInfo records.  Actually it's a Stack which is a
		* subclass of Vector, but it mostly the Vector methods that are used.
		*/
		private var _civ:Stack = new Stack();
		
		//TODO:
		private function initCiv():void
		{
			this._civ.addElement(new CallInfo());
		}
		
		/** CallInfo record for currently active function. */
		private function __ci():CallInfo
		{
			return this._civ.lastElement() as CallInfo;
		}

		/** Open Upvalues.  All UpVal objects that reference the VM stack.
		* openupval is a java.util.Vector of UpVal stored in order of stack
		* slot index: higher stack indexes are stored at higher Vector
		* positions.
		*/
		private var _openupval:Array = new Array();//Vector = new Vector();

		private var _hookcount:int = 0;
		private var _basehookcount:int = 0;
		private var _allowhook:Boolean = true;
		private var _hook:Hook = null;
		private var _hookmask:int = 0;

		/** Number of list items to accumulate before a SETLIST instruction. */
		public static const LFIELDS_PER_FLUSH:int = 50;
		
		/** Limit for table tag-method chains (to avoid loops) */
		private static const MAXTAGLOOP:int = 100;

		/**
		* The current error handler (set by {@link #pcall}).  A Lua
		* function to call.
		*/
		private var _errfunc:Object = null;

		/**
		* thread activation status.
		*/
		private var _status:int = 0;

		/** Nonce object used by pcall and friends (to detect when an
		* exception is a Lua error). */
		private static const LUA_ERROR:String = "";

		/** Metatable for primitive types.  Shared between all threads. */
		private var _metatable:Array = null; //LuaTable[]

		/**
		 * Maximum number of local variables per function.  As per
		 * LUAI_MAXVARS from "luaconf.h".  Default access so that {@link
		 * FuncState} can see it.
		 */
		public static const MAXVARS:int = 200;
		public static const MAXSTACK:int = 250;
		public static const MAXUPVALUES:int = 60;

		/**
		 * Stored in Slot.r to denote a numeric value (which is stored at 
		 * Slot.d).
		 */
		public static var NUMBER:Object = new Object();
		
		/**
		 * Spare Slot used for a temporary.
		 */
		private static var SPARE_SLOT:Slot = new Slot();

		/**
		* Registry key for loaded modules.
		*/
		public static const LOADED:String = "_LOADED";
		
		/**
		 * Used to construct a Lua thread that shares its global state with
		 * another Lua state.
		 */
		public function Lua(L:Lua = null) 
		{
			if (L == null)
			{
				//Creates a fresh Lua state.
				this._global = new LuaTable();
				this._registry = new LuaTable();
				this._metatable = new Array(NUM_TAGS); //LuaTable[]
				this._main = this;				
			}
			else
			{
				// Copy the global state, that's shared across all threads that
				// share the same main thread, into the new Lua thread.
				// Any more than this and the global state should be shunted to a
				// separate object (as it is in PUC-Rio).
				this._global = L._global;
				this._registry = L._registry;
				this._metatable = L._metatable;
				this._main = (L == null ? L : this);
			}
			
			//TODO:附加操作，初始化CallInfo堆栈
			initCiv();
		}
		
		//////////////////////////////////////////////////////////////////////
		// Public API

		/**
		 * Creates a fresh Lua state.
		 */
		/*
		public function Lua()
		{
			this._global = new LuaTable();
			this._registry = new LuaTable();
			this._metatable = new Array(NUM_TAGS); //LuaTable[]
			this._main = this;
		}
		*/

		/**
		* Equivalent of LUA_MULTRET.
		*/
		// Required, by vmPoscall, to be negative.
		public static const MULTRET:int = -1;
		/**
		* The Lua <code>nil</code> value.
		*/
		public static var NIL:Object = new Object();

		// Lua type tags, from lua.h
		/** Lua type tag, representing no stack value. */
		public static const TNONE:int         = -1;
		/** Lua type tag, representing <code>nil</code>. */
		public static const TNIL:int          = 0;
		/** Lua type tag, representing boolean. */
		public static const TBOOLEAN:int      = 1;
		// TLIGHTUSERDATA not available.  :todo: make available?
		/** Lua type tag, representing numbers. */
		public static const TNUMBER:int       = 3;
		/** Lua type tag, representing strings. */
		public static const TSTRING:int       = 4;
		/** Lua type tag, representing tables. */
		public static const TTABLE:int        = 5;
		/** Lua type tag, representing functions. */
		public static const TFUNCTION:int     = 6;
		/** Lua type tag, representing userdata. */
		public static const TUSERDATA:int     = 7;
		/** Lua type tag, representing threads. */
		public static const TTHREAD:int       = 8;
		/** 
		 * Number of type tags.  Should be one more than the
		 * last entry in the list of tags.
		 * 类型标签个数
		 */
		private static const NUM_TAGS:int     = 9;
		/** Names for above type tags, starting from {@link #TNIL}.
		* Equivalent to luaT_typenames.
		*/
		private static var TYPENAME:Array =  //final String[]
		[
			"nil", "boolean", "userdata", "number",
			"string", "table", "function", "userdata", "thread"
		];

		/**
		* Minimum stack size that Lua Java functions gets.  May turn out to
		* be silly / redundant.
		*/
		public static const MINSTACK:int = 20;

		/** Status code, returned from pcall and friends, that indicates the
		* thread has yielded.
		*/
		public static const YIELD:int         = 1;
		/** Status code, returned from pcall and friends, that indicates
		* a runtime error.
		*/
		public static const ERRRUN:int        = 2;
		/** Status code, returned from pcall and friends, that indicates
		* a syntax error.
		*/
		public static const ERRSYNTAX:int     = 3;
		/** Status code, returned from pcall and friends, that indicates
		* a memory allocation error.
		*/
		private static const ERRMEM:int        = 4;
		/** Status code, returned from pcall and friends, that indicates
		* an error whilst running the error handler function.
		*/
		public static const ERRERR:int        = 5;
		/** Status code, returned from loadFile and friends, that indicates
		* an IO error.
		*/
		public static const ERRFILE:int       = 6;

		// Enums for gc().
		/** Action, passed to {@link #gc}, that requests the GC to stop. */
		public static const GCSTOP:int        = 0;
		/** Action, passed to {@link #gc}, that requests the GC to restart. */
		public static const GCRESTART:int     = 1;
		/** Action, passed to {@link #gc}, that requests a full collection. */
		public static const GCCOLLECT:int     = 2;
		/** Action, passed to {@link #gc}, that returns amount of memory
		 * (in Kibibytes) in use (by the entire Java runtime).
		 */
		public static const GCCOUNT:int       = 3;
		/** Action, passed to {@link #gc}, that returns the remainder of
		 * dividing the amount of memory in use by 1024.
		 */
		public static const GCCOUNTB:int      = 4;
		/** Action, passed to {@link #gc}, that requests an incremental
		 * garbage collection be performed.
		 */
		public static const GCSTEP:int        = 5;
		/** Action, passed to {@link #gc}, that sets a new value for the
		 * <var>pause</var> of the collector.
		 */
		public static const GCSETPAUSE:int    = 6;
		/** Action, passed to {@link #gc}, that sets a new values for the
		 * <var>step multiplier</var> of the collector.
		 */
		public static const GCSETSTEPMUL:int  = 7;

		// Some of the hooks, etc, aren't implemented, so remain private.
		private static const HOOKCALL:int = 0;
		private static const HOOKRET:int = 1;
		private static const HOOKLINE:int = 2;
		/**
		 * When {@link Hook} callback is called as a line hook, its
		 * <var>ar.event</var> field is <code>HOOKCOUNT</code>.
		 */
		public static const HOOKCOUNT:int = 3;
		private static const HOOKTAILRET:int = 4;

		private static const MASKCALL:int = 1 << HOOKCALL;
		private static const MASKRET:int  = 1 << HOOKRET;
		private static const MASKLINE:int = 1 << HOOKLINE;
		/**
		* Bitmask that specifies count hook in call to {@link #setHook}.
		*/
		public static const MASKCOUNT:int = 1 << HOOKCOUNT;
		
		/**
		 * Calls a Lua value.  Normally this is called on functions, but the
         * semantics of Lua permit calls on any value as long as its metatable
         * permits it.
		 *
		 * In order to call a function, the function must be
		 * pushed onto the stack, then its arguments must be
		 * {@link #push pushed} onto the stack; the first argument is pushed
		 * directly after the function,
		 * then the following arguments are pushed in order (direct
		 * order).  The parameter <var>nargs</var> specifies the number of
		 * arguments (which may be 0).
		 *
		 * When the function returns the function value on the stack and all
		 * the arguments are removed from the stack and replaced with the
		 * results of the function, adjusted to the number specified by
		 * <var>nresults</var>.  So the first result from the function call will
		 * be at the same index where the function was immediately prior to
		 * calling this method.
		 *
		 * @param nargs     The number of arguments in this function call.
		 * @param nresults  The number of results required.
		 */
		public function call(nargs:int, nresults:int):void
		{
			apiChecknelems(nargs+1);
			var func:int = this._stackSize - (nargs + 1);
			this.vmCall(func, nresults);
		}

		/**
		 * Closes a Lua state.  In this implementation, this method does
         * nothing.
         */
		public function close():void
		{
			
		}

		/**
		 * Concatenate values (usually strings) on the stack.
	     * <var>n</var> values from the top of the stack are concatenated, as
		 * strings, and replaced with the resulting string.
	     * @param n  the number of values to concatenate.
		 */
		public function concat(n:int):void
		{
			apiChecknelems(n);
			if (n >= 2)
			{
				vmConcat(n, (this._stackSize - this._base) - 1);
				pop(n-1);
			}
			else if (n == 0)          // push empty string
			{
				pushString("");
			} // else n == 1; nothing to do
		}

		/**
		* Creates a new empty table and returns it.
		* @param narr  number of array elements to pre-allocate.
		* @param nrec  number of non-array elements to pre-allocate.
		* @return a fresh table.
		* @see #newTable
		*/
		public function createTable(narr:int, nrec:int):LuaTable
		{
			var t:LuaTable = new LuaTable();
			t.init(narr, nrec);
			return t;
		}

		/**
		 * Dumps a function as a binary chunk.
		 * @param function  the Lua function to dump.
		 * @param writer    the stream that receives the dumped binary.
		 * @throws IOException when writer does.
		 */
		public static function dump(_function:Object, writer:OutputStream):void 
		  //throws IOException
		{
			if (!(_function is LuaFunction))
			{
				throw new IOException("Cannot dump " + typeName(____type(_function)));
			}
			var f:LuaFunction = _function as LuaFunction;
			uDump(f.proto, writer, false);
		}

		/**
		 * Tests for equality according to the semantics of Lua's
		 * <code>==</code> operator (so may call metamethods).
		 * @param o1  a Lua value.
		 * @param o2  another Lua value.
		 * @return true when equal.
		 */
		public function equal(o1:Object, o2:Object):Boolean
		{
			if (o1 is Number)
			{
				return o1.equals(o2);
			}
			return vmEqualRef(o1, o2);
		}

		/**
		 * Generates a Lua error using the error message.
		 * @param message  the error message.
		 * @return never.
		 */
		public function error(message:Object):int
		{
			return gErrormsg(message);
		}

		/**
		* Control garbage collector.  Note that in Jill most of the options
		* to this function make no sense and they will not do anything.
		* @param what  specifies what GC action to take.
		* @param data  data that may be used by the action.
		* @return varies.
		*/
		public function gc(what:int, data:int):int
		{
			var rt:Runtime;

			switch (what)
			{
				case GCSTOP:
					return 0;
					
				case GCRESTART:
				case GCCOLLECT:
				case GCSTEP:
					SystemUtil.gc();
					return 0;
			  
				case GCCOUNT:
					rt = Runtime.getRuntime();
					return (int)((rt.totalMemory() - rt.freeMemory()) / 1024);
			  
				case GCCOUNTB:
					rt = Runtime.getRuntime();
					return (int)((rt.totalMemory() - rt.freeMemory()) % 1024);
			  
				case GCSETPAUSE:
				case GCSETSTEPMUL:
					return 0;
			}
			return 0;
		}

		/**
		* Returns the environment table of the Lua value.
		* @param o  the Lua value.
		* @return its environment table.
		*/
		public function getFenv(o:Object):LuaTable
		{
			if (o is LuaFunction)
			{
				var f1:LuaFunction = o as LuaFunction;
				return f1.env;
			}
			if (o is LuaJavaCallback)
			{
				var f2:LuaJavaCallback = o as LuaJavaCallback;
				// :todo: implement this case.
				return null;
			}
			
			if (o is LuaUserdata)
			{
				var u:LuaUserdata = o as LuaUserdata;
				return u.env;
			}
			if (o is Lua)
			{
				var l:Lua = o as Lua;
				return l.global;
			}
			return null;
		}

		/**
		 * Get a field from a table (or other object).
		 * @param t      The object whose field to retrieve.
		 * @param field  The name of the field.
		 * @return  the Lua value
		 */
		public function getField(t:Object, field:String):Object
		{
			return getTable(t, field);
		}

		/**
		 * Get a global variable.
	     * @param name  The name of the global variable.
	     * @return  The value of the global variable.
	     */
		public function getGlobal(name:String):Object
		{
			return getField(this._global, name);
		}

		/**
		 * Gets the global environment.  The global environment, where global
		 * variables live, is returned as a <code>LuaTable</code>.  Note that
		 * modifying this table has exactly the same effect as creating or
		 * changing global variables from within Lua.
		 * @return  The global environment as a table.
		 */
		public function getGlobals():LuaTable
		{
			return this._global;
		}

		/** Get metatable.
		 * @param o  the Lua value whose metatable to retrieve.
		 * @return The metatable, or null if there is no metatable.
		 */
		public function getMetatable(o:Object):LuaTable
		{
			var mt:LuaTable;

			if (o is LuaTable)
			{
				var t:LuaTable = o as LuaTable;
				mt = t.metatable;
			}
			else if (o is LuaUserdata)
			{
				var u:LuaUserdata = o as LuaUserdata;
				mt = u.metatable;
			}
			else
			{
				mt = this._metatable[____type(o)];
			}
			return mt;
		}

		/**
		 * Gets the registry table.
		 */
		public function getRegistry():LuaTable
		{
			return this._registry;
		}

		/**
		 * Indexes into a table and returns the value.
		 * @param t  the Lua value to index.
		 * @param k  the key whose value to return.
		 * @return the value t[k].
		 */
		public function getTable(t:Object, k:Object):Object
		{
			var s:Slot = new Slot();
			s.init2(k);
			var v:Slot = new Slot();
			vmGettable(t, s, v);
			return v.asObject();
		}

		/**
		 * Gets the number of elements in the stack.  If the stack is not
		 * empty then this is the index of the top-most element.
		 * @return number of stack elements.
		*/
		public function getTop():int
		{
			return this._stackSize - this._base;
		}

		/**
		 * Insert Lua value into stack immediately at specified index.  Values
		 * in stack at that index and higher get pushed up.
		 * @param o    the Lua value to insert into the stack.
		 * @param idx  the stack index at which to insert.
		 */
		public function insert(o:Object, idx:int):void
		{
			idx = absIndexUnclamped(idx);
			stackInsertAt(o, idx);
		}

		/**
		 * Tests that an object is a Lua boolean.
		 * @param o  the Object to test.
		 * @return true if and only if the object is a Lua boolean.
		 */
		public static function isBoolean(o:Object):Boolean
		{
			return o is Boolean;
		}

		/**
		 * Tests that an object is a Lua function implementated in Java (a Lua
		 * Java Function).
		 * @param o  the Object to test.
		 * @return true if and only if the object is a Lua Java Function.
		 */
		public static function isJavaFunction(o:Object):Boolean
		{
			return o is LuaJavaCallback;
		}

		/**
		 * Tests that an object is a Lua function (implemented in Lua or
		 * Java).
		 * @param o  the Object to test.
		 * @return true if and only if the object is a function.
		 */
		public static function isFunction(o:Object):Boolean
		{
			return o is LuaFunction ||
				o is LuaJavaCallback;
		}

		/**
		 * Tests that a Lua thread is the main thread.
		 * @return true if and only if is the main thread.
		 */
		public function isMain():Boolean
		{
			return this == this._main;
		}

		/**
		 * Tests that an object is Lua <code>nil</code>.
		 * @param o  the Object to test.
		 * @return true if and only if the object is Lua <code>nil</code>.
		 */
		public static function isNil(o:Object):Boolean
		{
			return NIL == o;
		}

		/**
		 * Tests that an object is a Lua number or a string convertible to a
		 * number.
		 * @param o  the Object to test.
		 * @return true if and only if the object is a number or a convertible string.
		 */
		public static function isNumber(o:Object):Boolean
		{
			SPARE_SLOT.setObject(o);
			return tonumber(SPARE_SLOT, NUMOP);
		}

		/**
		 * Tests that an object is a Lua string or a number (which is always
		 * convertible to a string).
		 * @param o  the Object to test.
		 * @return true if and only if object is a string or number.
		 */
		public static function isString(o:Object):Boolean
		{
			return o is String || o is Number;
		}

		/**
		 * Tests that an object is a Lua table.
		 * @param o  the Object to test.
		 * @return <code>true</code> if and only if the object is a Lua table.
		 */
		public static function isTable(o:Object):Boolean
		{
			return o is LuaTable;
		}

		/**
		 * Tests that an object is a Lua thread.
	 	 * @param o  the Object to test.
		 * @return <code>true</code> if and only if the object is a Lua thread.
		 */
		public static function isThread(o:Object):Boolean
		{
			return o is Lua;
		}

		/**
		 * Tests that an object is a Lua userdata.
		 * @param o  the Object to test.
		 * @return true if and only if the object is a Lua userdata.
		 */
		public static function isUserdata(o:Object):Boolean
		{
			return o is LuaUserdata;
		}

		/**
		 * <p>
		 * Tests that an object is a Lua value.  Returns <code>true</code> for
		 * an argument that is a Jill representation of a Lua value,
		 * <code>false</code> for Java references that are not Lua values.
		 * For example <code>isValue(new LuaTable())</code> is
		 * <code>true</code>, but <code>isValue(new Object[] { })</code> is
		 * <code>false</code> because Java arrays are not a representation of
		 * any Lua value.
		 * </p>
		 * <p>
		 * PUC-Rio Lua provides no
		 * counterpart for this method because in their implementation it is
		 * impossible to get non Lua values on the stack, whereas in Jill it
		 * is common to mix Lua values with ordinary, non Lua, Java objects.
		 * </p>
		 * @param o  the Object to test.
		 * @return true if and if it represents a Lua value.
		 */
		public static function isValue(o:Object):Boolean
		{
			return o == NIL ||
				o is Boolean ||
				o is String ||
				o is Number ||
				o is LuaFunction ||
				o is LuaJavaCallback ||
				o is LuaTable ||
				o is LuaUserdata;
		}

		/**
		 * Compares two Lua values according to the semantics of Lua's
		 * <code>&lt;</code> operator, so may call metamethods.
		 * @param o1  the left-hand operand.
		 * @param o2  the right-hand operand.
		 * @return true when <code>o1 < o2</code>.
		 */
		public function lessThan(o1:Object, o2:Object):Boolean
		{
			var a:Slot = new Slot();
			a.init2(o1);
			var b:Slot = new Slot();
			b.init2(o2);
			return vmLessthan(a, b);
		}

		/**
		 * <p>
		 * Loads a Lua chunk in binary or source form.
		 * Comparable to C's lua_load.  If the chunk is determined to be
		 * binary then it is loaded directly.  Otherwise the chunk is assumed
		 * to be a Lua source chunk and compilation is required first; the
		 * <code>InputStream</code> is used to create a <code>Reader</code>
		 * using the UTF-8 encoding
		 * (using a second argument of <code>"UTF-8"</code> to the
		 * {@link java.io.InputStreamReader#InputStreamReader(java.io.InputStream,
		 * java.lang.String)}
		 * constructor) and the Lua source is compiled.
		 * </p>
		 * <p>
		 * If successful, The compiled chunk, a Lua function, is pushed onto
		 * the stack and a zero status code is returned.  Otherwise a non-zero
		 * status code is returned to indicate an error and the error message
		 * is pushed onto the stack.
		 * </p>
		 * @param in         The binary chunk as an InputStream, for example from
		 *                   {@link Class#getResourceAsStream}.
		 * @param chunkname  The name of the chunk.
		 * @return           A status code.
		 */
		public function load(_in:InputStream, chunkname:String):int
		{
			var li:LuaInternal = new LuaInternal();
			li.init1(_in, chunkname);
			pushObject(li);
			return pcall(0, 1, null);
		}

		/**
		 * Loads a Lua chunk in source form.
		 * Comparable to C's lua_load.  This method takes a {@link
		 * java.io.Reader} parameter,
		 * and is normally used to load Lua chunks in source form.
		 * However, it if the input looks like it is the output from Lua's
		 * <code>string.dump</code> function then it will be processed as a
		 * binary chunk.
		 * In every other respect this method is just like {@link
		 * #load(InputStream, String)}.
		 * 废弃，合并入load
		 * @param in         The source chunk as a Reader, for example from
		 *                   <code>java.io.InputStreamReader(Class.getResourceAsStream())</code>.
		 * @param chunkname  The name of the chunk.
		 * @return           A status code.
		 * @see java.io.InputStreamReader
		 */
		public function __load(_in:Reader, chunkname:String):int
		{
			var li:LuaInternal = new LuaInternal();
			li.init2(_in, chunkname);
			pushObject(li);
			return pcall(0, 1, null);
		}

		/**
		 * Slowly get the next key from a table.  Unlike most other functions
		 * in the API this one uses the stack.  The top-of-stack is popped and
		 * used to find the next key in the table at the position specified by
		 * index.  If there is a next key then the key and its value are
		 * pushed onto the stack and <code>true</code> is returned.
		 * Otherwise (the end of the table has been reached)
		 * <code>false</code> is returned.
		 * @param idx  stack index of table.
		 * @return  true if and only if there are more keys in the table.
		 * @deprecated Use {@link #tableKeys} enumeration protocol instead.
		 */
		public function next(idx:int):Boolean
		{
			var o:Object = value(idx);
			// :todo: api check
			var t:LuaTable = o as LuaTable;
			var key:Object = value(-1);
			pop(1);
			var e:Enumeration = t.keys();
			if (key == NIL)
			{
				if (e.hasMoreElements())
				{
					key = e.nextElement();
					pushObject(key);
					pushObject(t.getlua(key));
					return true;
				}
				return false;
			}
			while (e.hasMoreElements())
			{
				var k:Object = e.nextElement();
				if (k.equals(key))
				{
					if (e.hasMoreElements())
					{
						key = e.nextElement();
						pushObject(key);
						pushObject(t.getlua(key));
						return true;
					}
					return false;
				}
			}
			// protocol error which we could potentially diagnose.
			return false;
		}

		/**
		 * Creates a new empty table and returns it.
		 * @return a fresh table.
		 * @see #createTable
		 */
		public function newTable():LuaTable
		{
			return new LuaTable();
		}

		/**
		 * Creates a new Lua thread and returns it.
	     * @return a new Lua thread.
		 */
		public function newThread():Lua
		{
			return new Lua(this);
		}

		/**
		 * Wraps an arbitrary Java reference in a Lua userdata and returns it.
		 * @param ref  the Java reference to wrap.
		 * @return the new LuaUserdata.
		 */
		public function newUserdata(ref:Object):LuaUserdata
		{
			return new LuaUserdata(ref);
		}

		/**
		 * Return the <em>length</em> of a Lua value.  For strings this is
		 * the string length; for tables, this is result of the <code>#</code>
		 * operator; for other values it is 0.
		 * @param o  a Lua value.
		 * @return its length.
		 */
		public static function objLen(o:Object):int
		{
			if (o is String)
			{
				var s:String = o as String;
				return s.length;
			}
			if (o is LuaTable)
			{
				var t:LuaTable = o as LuaTable;
				return t.getn();
			}
			if (o is Number)
			{
				return vmTostring(o).length;
			}
			return 0;
		}


		/**
		 * <p>
		 * Protected {@link #call}.  <var>nargs</var> and
		 * <var>nresults</var> have the same meaning as in {@link #call}.
		 * If there are no errors during the call, this method behaves as
		 * {@link #call}.  Any errors are caught, the error object (usually
		 * a message) is pushed onto the stack, and a non-zero error code is
		 * returned.
		 * </p>
		 * <p>
		 * If <var>er</var> is <code>null</code> then the error object that is
		 * on the stack is the original error object.  Otherwise
		 * <var>ef</var> specifies an <em>error handling function</em> which
		 * is called when the original error is generated; its return value
		 * becomes the error object left on the stack by <code>pcall</code>.
		 * </p>
		 * @param nargs     number of arguments.
		 * @param nresults  number of result required.
		 * @param ef        error function to call in case of error.
		 * @return 0 if successful, else a non-zero error code.
		 */
		public function pcall(nargs:int, nresults:int, ef:Object):int
		{
			apiChecknelems(nargs + 1);
			var restoreStack:int = this._stackSize - (nargs + 1);
			// Most of this code comes from luaD_pcall
			var restoreCi:int = this._civ.size;
			var oldnCcalls:int = this._nCcalls;
			var old_errfunc:Object = this._errfunc;
			this._errfunc = ef;
			var old_allowhook:Boolean = this._allowhook;
			var errorStatus:int = 0;
			try
			{
				call(nargs, nresults);
			}
			catch (e:Error)
			{
				if (e is LuaError)
				{
					var e1:LuaError = e as LuaError;
					trace(e1.getStackTrace());
					fClose(restoreStack);   // close eventual pending closures
					dSeterrorobj(e1.errorStatus, restoreStack);
					this._nCcalls = oldnCcalls;
					this._civ.size = restoreCi;
					var ci:CallInfo = __ci();
					this._base = ci.base;
					this._savedpc = ci.savedpc;
					this._allowhook = old_allowhook;
					errorStatus = e1.errorStatus;
				}
				else if (e is OutOfMemoryError)
				{
					var e2:OutOfMemoryError = e as OutOfMemoryError;
					trace(e2.getStackTrace());
					fClose(restoreStack);     // close eventual pending closures
					dSeterrorobj(ERRMEM, restoreStack);
					this._nCcalls = oldnCcalls;
					this._civ.size = restoreCi;
					var ci2:CallInfo = __ci();
					this._base = ci.base;
					this._savedpc = ci2.savedpc;
					this._allowhook = old_allowhook;
					errorStatus = ERRMEM;
				}
			}
			/**/
			this._errfunc = old_errfunc;
			return errorStatus;
		}

		/**
		 * Removes (and discards) the top-most <var>n</var> elements from the stack.
		 * @param n  the number of elements to remove.
		 */
		public function pop(n:int):void
		{
			if (n < 0)
			{
				throw new IllegalArgumentException();
			}
			stacksetsize(this._stackSize - n);
		}

		/**
		 * Pushes a value onto the stack in preparation for calling a
		 * function (or returning from one).  See {@link #call} for
		 * the protocol to be used for calling functions.  See {@link
		 * #pushNumber} for pushing numbers, and {@link #pushValue} for
		 * pushing a value that is already on the stack.
		 * @param o  the Lua value to push.
		 */
		public function pushObject(o:Object):void
		{
			// see also a private overloaded version of this for Slot.
			stackAdd(o);
		}

		/**
		 * Push boolean onto the stack.
		 * @param b  the boolean to push.
		 */
		public function pushBoolean(b:Boolean):void
		{
			pushObject(valueOfBoolean(b));
		}

		/**
		 * Push literal string onto the stack.
		 * @param s  the string to push.
		 */
		public function pushLiteral(s:String):void
		{
			pushObject(s);
		}

		/** Push nil onto the stack. */
		public function pushNil():void
		{
			pushObject(NIL);
		}

		/**
		* Pushes a number onto the stack.  See also {@link #push}.
		* @param d  the number to push.
		*/
		public function pushNumber(d:Number):void
		{
			// :todo: optimise to avoid creating Double instance
			pushObject(new Number(d));
		}

		/**
		 * Push string onto the stack.
		 * @param s  the string to push.
		 */
		public function pushString(s:String):void
		{
			pushObject(s);
		}

		/**
		 * Copies a stack element onto the top of the stack.
		 * Equivalent to <code>L.push(L.value(idx))</code>.
		 * @param idx  stack index of value to push.
		 */
		public function pushValue(idx:int):void
		{
			// :todo: optimised to avoid creating Double instance
			pushObject(value(idx));
		}

		/**
		 * Implements equality without metamethods.
		 * @param o1  the first Lua value to compare.
		 * @param o2  the other Lua value.
		 * @return  true if and only if they compare equal.
		 */
		public static function rawEqual(o1:Object, o2:Object):Boolean
		{
			return oRawequal(o1, o2);
		}

		/**
		 * Gets an element from a table, without using metamethods.
		 * @param t  The table to access.
		 * @param k  The index (key) into the table.
		 * @return The value at the specified index.
		 */
		public static function rawGet(t:Object, k:Object):Object
		{
			var table:LuaTable = t as LuaTable;
			return table.getlua(k);
		}

		/**
		* Gets an element from an array, without using metamethods.
		* @param t  the array (table).
		* @param i  the index of the element to retrieve.
		* @return  the value at the specified index.
		*/
		public static function rawGetI(t:Object, i:int):Object
		{
			var table:LuaTable = t as LuaTable;
			return table.getnum(i);
		}

		/**
		 * Sets an element in a table, without using metamethods.
		 * @param t  The table to modify.
		 * @param k  The index into the table.
		 * @param v  The new value to be stored at index <var>k</var>.
		 */
		public function rawSet(t:Object, k:Object, v:Object):void
		{
			var table:LuaTable = t as LuaTable;
			table.putluaObj(this, k, v);
		}

		/**
		* Sets an element in an array, without using metamethods.
		* @param t  the array (table).
		* @param i  the index of the element to set.
		* @param v  the new value to be stored at index <var>i</var>.
		*/
		public function rawSetI(t:Object, i:int, v:Object):void
		{
			apiCheck(t is LuaTable);
			var h:LuaTable = t as LuaTable;
			h.putnum(i, v);
		}

		/**
		 * Register a {@link LuaJavaCallback} as the new value of the global
		 * <var>name</var>.
		 * @param name  the name of the global.
		 * @param f     the LuaJavaCallback to register.
		 */
		public function register(name:String, f:LuaJavaCallback):void
		{
			setGlobal(name, f);
		}

		/**
		 * Starts and resumes a Lua thread.  Threads can be created using
		 * {@link #newThread}.  Once a thread has begun executing it will
		 * run until it either completes (with error or normally) or has been
		 * suspended by invoking {@link #yield}.
		 * @param narg  Number of values to pass to thread.
		 * @return Lua.YIELD, 0, or an error code.
		 */
		public function resume(narg:int):int
		{
			if (status != YIELD)
			{
				if (status != 0)
					return resume_error("cannot resume dead coroutine");
				else if (this._civ.size != 1)
					return resume_error("cannot resume non-suspended coroutine");
			}
			// assert errfunc == 0 && nCcalls == 0;
			var errorStatus:int = 0;
		protect:
			try
			{
				// This block is equivalent to resume from ldo.c
				var firstArg:int = this._stackSize - narg;
				if (status == 0)  // start coroutine?
				{
					// assert civ.size() == 1 && firstArg > base);
					if (vmPrecall(firstArg - 1, MULTRET) != PCRLUA)
						break protect;
				}
				else      // resuming from previous yield
				{
					// assert status == YIELD;
					status = 0;
					if (!isLua(__ci()))       // 'common' yield
					{
						// finish interrupted execution of 'OP_CALL'
						// assert ...
						if (vmPoscall(firstArg))      // complete it...
							stacksetsize(__ci().top);  // and correct top
					}
					else    // yielded inside a hook: just continue its execution
						this._base = __ci().base;
				}
				vmExecute(this._civ.size - 1);
			}
			catch (e:LuaError)
			{
				trace(e.getStackTrace());
				status = e.errorStatus;   // mark thread as 'dead'
				dSeterrorobj(e.errorStatus, this._stackSize);
				__ci().top = this._stackSize;
			}
			return status;
		}

		/**
		* Set the environment for a function, thread, or userdata.
		* @param o      Object whose environment will be set.
		* @param table  Environment table to use.
		* @return true if the object had its environment set, false otherwise.
		*/
		public function setFenv(o:Object, table:Object):Boolean
		{
			// :todo: consider implementing common env interface for
			// LuaFunction, LuaJavaCallback, LuaUserdata, Lua.  One cast to an
			// interface and an interface method call may be shorter
			// than this mess.
			var t:LuaTable = table as LuaTable;

			if (o is LuaFunction)
			{
				var f1:LuaFunction = o as LuaFunction;
				f1.env = t;
				return true;
			}
			if (o is LuaJavaCallback)
			{
				var f2:LuaJavaCallback = o as LuaJavaCallback;
				// :todo: implement this case.
				return false;
			}
			if (o is LuaUserdata)
			{
				var u:LuaUserdata = o as LuaUserdata;
				u.env = t;
				return true;
			}
			if (o is Lua)
			{
				var l:Lua = o as Lua;
				l.global = t;
				return true;
			}
			return false;
		}

		/**
		 * Set a field in a Lua value.
		 * @param t     Lua value of which to set a field.
		 * @param name  Name of field to set.
		 * @param v     new Lua value for field.
		 */
		public function setField(t:Object, name:String, v:Object):void
		{
			var s:Slot = new Slot();
			s.init2(name as Object);
			vmSettable(t, s, v);
		}

		/**
		* Sets the metatable for a Lua value.
		* @param o   Lua value of which to set metatable.
		* @param mt  The new metatable.
		*/
		public function setMetatable(o:Object, mt:Object):void
		{
			if (isNil(mt))
			{
				mt = null;
			}
			else
			{
				apiCheck(mt is LuaTable);
			}
			var mtt:LuaTable = mt as LuaTable;
			if (o is LuaTable)
			{
				var t:LuaTable = o as LuaTable;
				t.setMetatable(mtt);
			}
			else if (o is LuaUserdata)
			{
				var u:LuaUserdata = o as LuaUserdata;
				u.metatable = mtt;
			}
			else
			{
				this._metatable[____type(o)] = mtt;
			}
		}

		/**
		 * Set a global variable.
		 * @param name   name of the global variable to set.
		 * @param value  desired new value for the variable.
		 */
		public function setGlobal(name:String, value:Object):void
		{
			var s:Slot = new Slot();
			s.init2(name as Object);
			vmSettable(this._global, s, value);
		}

		/**
		 * Does the equivalent of <code>t[k] = v</code>.
		 * @param t  the table to modify.
		 * @param k  the index to modify.
		 * @param v  the new value at index <var>k</var>.
		 */
		public function setTable(t:Object, k:Object, v:Object):void
		{
			var s:Slot = new Slot();
			s.init2(k);
			vmSettable(t, s, v);
		}

		/**
		* Set the stack top.
		* @param n  the desired size of the stack (in elements).
		*/
		public function setTop(n:int):void
		{
			if (n < 0)
			{
				throw new IllegalArgumentException();
			}
			stacksetsize(this._base + n);
		} 

		/**
		 * Status of a Lua thread.
		 * @return 0, an error code, or Lua.YIELD.
		 */
		public function get status():int
		{
			return _status;
		}

		public function set status(status:int):void
		{
			this._status = status;
		}
		
		/**
		 * Returns an {@link java.util.Enumeration} for the keys of a table.
		 * @param t  a Lua table.
		 * @return an Enumeration object.
		 */
		public function tableKeys(t:Object):Enumeration
		{
			if (!(t is LuaTable))
			{
				error("table required");
				// NOTREACHED
			}
			return (t as LuaTable).keys();
		}

		/**
		 * Convert to boolean.
		 * @param o  Lua value to convert.
		 * @return  the resulting primitive boolean.
		 */
		public function toBoolean(o:Object):Boolean
		{
			return !(o == NIL || o == false);
		}

		/**
		 * Convert to integer and return it.  Returns 0 if cannot be
		 * converted.
		 * @param o  Lua value to convert.
		 * @return  the resulting int.
		 */
		public function toInteger(o:Object):int
		{
			return toNumber(o) as int;
		}

		/**
		 * Convert to number and return it.  Returns 0 if cannot be
		 * converted.
		 * @param o  Lua value to convert.
		 * @return  The resulting number.
		 */
		public function toNumber(o:Object):Number
		{
			SPARE_SLOT.setObject(o);
			if (Lua.tonumber(SPARE_SLOT, NUMOP))
			{
				return NUMOP[0];
			}
			return 0;
		}

		/**
		 * Convert to string and return it.  If value cannot be converted then
		 * <code>null</code> is returned.  Note that unlike
		 * <code>lua_tostring</code> this
		 * does not modify the Lua value.
		 * @param o  Lua value to convert.
		 * @return  The resulting string.
		 */
		public function toString(o:Object):String
		{
			return vmTostring(o);
		}

		/**
		 * Convert to Lua thread and return it or <code>null</code>.
		 * @param o  Lua value to convert.
		 * @return  The resulting Lua instance.
		 */
		public function toThread(o:Object):Lua 
		{
			if (!(o is Lua))
			{
				return null;
			}
			return o as Lua;
		}

		/**
		 * Convert to userdata or <code>null</code>.  If value is a {@link
		 * LuaUserdata} then it is returned, otherwise, <code>null</code> is
		 * returned.
		 * @param o  Lua value.
		 * @return  value as userdata or <code>null</code>.
		 */
		public function toUserdata(o:Object):LuaUserdata
		{
			if (o is LuaUserdata)
			{
				return o as LuaUserdata;
			}
			return null;
		}

		/**
		 * Type of the Lua value at the specified stack index.
		 * @param idx  stack index to type.
		 * @return  the type, or {@link #TNONE} if there is no value at <var>idx</var>
		 */
		public /*static*/ function type(idx:int):int
		{
			idx = absIndex(idx);
			if (idx < 0)
			{
				return TNONE;
			}
			return ___type(this._stack[idx] as Slot);
		}

		/**
		 * 废弃，并入type
		 * @param	s
		 * @return
		 */
		private function ___type(s:Slot):int
		{
			if (s.r == NUMBER)
			{
				return TNUMBER;
			}
			return ____type(s.r);
		}

		/**
		 * Type of a Lua value.
		 * 废弃，并入type
		 * @param o  the Lua value whose type to return.
		 * @return  the Lua type from an enumeration.
		 */
		public static function ____type(o:Object):int
		{
			if (o == NIL)
			{
				return TNIL;
			}
			else if (o is Number)
			{
				return TNUMBER;
			}
			else if (o is Boolean)
			{
				return TBOOLEAN;
			}
			else if (o is String)
			{
				return TSTRING;
			}
			else if (o is LuaTable)
			{
				return TTABLE;
			}
			else if (o is LuaFunction || o is LuaJavaCallback)
			{
				return TFUNCTION;
			}
			else if (o is LuaUserdata)
			{
				return TUSERDATA;
			}
			else if (o is Lua)
			{
				return TTHREAD;
			}
			return TNONE;
		}

		/**
		 * Name of type.
		 * @param type  a Lua type from, for example, {@link #type}.
		 * @return  the type's name.
		 */
		public static function typeName(type:int):String
		{
			if (TNONE == type)
			{
				return "no value";
			}
			return TYPENAME[type];
		}

		/**
		 * Gets a value from the stack.
		 * If <var>idx</var> is positive and exceeds
		 * the size of the stack, {@link #NIL} is returned.
		 * @param idx  the stack index of the value to retrieve.
		 * @return  the Lua value from the stack.
		 */
		public function value(idx:int):Object
		{
			idx = absIndex(idx);
			if (idx < 0)
			{
				return NIL;
			}
			return (this._stack[idx] as Slot).asObject();
		}

		/**
		* Converts primitive boolean into a Lua value.
		* @param b  the boolean to convert.
		* @return  the resulting Lua value.
		*/
		public static function valueOfBoolean(b:Boolean):Object
		{
			// If CLDC 1.1 had
			// <code>java.lang.Boolean.valueOf(boolean);</code> then I probably
			// wouldn't have written this.  This does have a small advantage:
			// code that uses this method does not need to assume that Lua booleans in
			// Jill are represented using Java.lang.Boolean.
			if (b)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		/**
		 * Converts primitive number into a Lua value.
		 * @param d  the number to convert.
		 * @return  the resulting Lua value.
		 */
		public static function valueOfNumber(d:Number):Object
		{
			// :todo: consider interning "common" numbers, like 0, 1, -1, etc.
			return new Number(d);
		}

		/**
		* Exchange values between different threads.
		* @param to  destination Lua thread.
		* @param n   numbers of stack items to move.
		*/
		public function xmove(to:Lua, n:int):void
		{
			if (this == to)
			{
				return;
			}
			apiChecknelems(n);
			// L.apiCheck(from.G() == to.G());
			for (var i:int = 0; i < n; ++i)
			{
				to.pushObject(value( -n + i));
			}
			pop(n);
		}

		/**
		 * Yields a thread.  Should only be called as the return expression
		 * of a Lua Java function: <code>return L.yield(nresults);</code>.
		 * A {@link RuntimeException} can also be thrown to yield.  If the
		 * Java code that is executing throws an instance of {@link
		 * RuntimeException} (direct or indirect) then this causes the Lua 
		 * thread to be suspended, as if <code>L.yield(0);</code> had been
		 * executed, and the exception is re-thrown to the code that invoked
		 * {@link #resume}.
		 * @param nresults  Number of results to return to {@link #resume}.
		 * @return  a secret value.
		 */
		public function yield(nresults:int):int
		{
			if (this._nCcalls > 0)
				gRunerror("attempt to yield across metamethod/Java-call boundary");
			this._base = this._stackSize - nresults;     // protect stack slots below
			status = YIELD;
			return -1;
		}

		// Miscellaneous private functions.

		/** Convert from Java API stack index to absolute index.
		 * @return an index into <code>this.stack</code> or -1 if out of range.
		 */
		private function absIndex(idx:int):int
		{
			var s:int = this._stackSize;

			if (idx == 0)
			{
				return -1;
			}
			if (idx > 0)
			{
				if (idx + this._base > s)
				{
					return -1;
				}
				return this._base + idx - 1;
			}
			// idx < 0
			if (s + idx < this._base)
			{
				return -1;
			}
			return s + idx;
		}

		/**
		* As {@link #absIndex} but does not return -1 for out of range
		* indexes.  Essential for {@link #insert} because an index equal
		* to the size of the stack is valid for that call.
		*/
		private function absIndexUnclamped(idx:int):int
		{
			if (idx == 0)
			{
				return -1;
			}
			if (idx > 0)
			{
				return this._base + idx - 1;
			}
			// idx < 0
			return this._stackSize + idx;
		}


		//////////////////////////////////////////////////////////////////////
		// Auxiliary API

		// :todo: consider placing in separate class (or macroised) so that we
		// can change its definition (to remove the check for example).
		private function apiCheck(cond:Boolean):void
		{
			if (!cond)
			{
				throw new IllegalArgumentException();
			}
		}

		private function apiChecknelems(n:int):void
		{
			apiCheck(n <= this._stackSize - this._base);
		}

		/**
		 * Checks a general condition and raises error if false.
		 * @param cond      the (evaluated) condition to check.
		 * @param numarg    argument index.
		 * @param extramsg  extra error message to append.
		 */
		public function argCheck(cond:Boolean, numarg:int, extramsg:String):void
		{
			if (cond)
			{
				return;
			}
			argError(numarg, extramsg);
		}

		/**
		 * Raise a general error for an argument.
		 * @param narg      argument index.
		 * @param extramsg  extra message string to append.
		 * @return never (used idiomatically in <code>return argError(...)</code>)
		 */
		public function argError(narg:int, extramsg:String):int
		{
			// :todo: use debug API as per PUC-Rio
			if (true)
			{
				return error("bad argument " + narg + " (" + extramsg + ")");
			}
			return 0;
		}

		/**
		 * Calls a metamethod.  Pushes 1 result onto stack if method called.
		 * @param obj    stack index of object whose metamethod to call
		 * @param event  metamethod (event) name.
		 * @return  true if and only if metamethod was found and called.
		 */
		public function callMeta(obj:int, event:String):Boolean
		{
			var o:Object = value(obj);
			var ev:Object = getMetafield(o, event);
			if (ev == NIL)
			{
				return false;
			}
			pushObject(ev);
			pushObject(o);
			call(1, 1);
			return true;
		}

		/**
		 * Checks that an argument is present (can be anything).
		 * Raises error if not.
		 * @param narg  argument index.
		 */
		public function checkAny(narg:int):void
		{
			if (type(narg) == TNONE)
			{
				argError(narg, "value expected");
			}
		}

		/**
		 * Checks is a number and returns it as an integer.  Raises error if
		 * not a number.
		 * @param narg  argument index.
		 * @return  the argument as an int.
		 */
		public function checkInt(narg:int):int
		{
			return checkNumber(narg) as int;
		}

		/**
		 * Checks is a number.  Raises error if not a number.
		 * @param narg  argument index.
		 * @return  the argument as a double.
		 */
		public function checkNumber(narg:int):Number
		{
			var o:Object = value(narg);
			var d:Number = toNumber(o);
			if (d == 0 && !isNumber(o))
			{
				tagError(narg, TNUMBER);
			}
			return d;
		}

		/**
		 * Checks that an optional string argument is an element from a set of
		 * strings.  Raises error if not.
		 * @param narg  argument index.
		 * @param def   default string to use if argument not present.
		 * @param lst   the set of strings to match against.
		 * @return an index into <var>lst</var> specifying the matching string.
		 */
		public function checkOption(narg:int, def:String, lst:Array /*String[] */):int
		{
			var name:String;

			if (def == null)
			{
				name = checkString(narg);
			}
			else
			{
				name = optString(narg, def);
			}
			for (var i:int = 0; i < lst.length; ++i)
			{
				if (lst[i].equals(name))
				{
					return i;
				}
			}
			return argError(narg, "invalid option '" + name + "'");
		}

		/**
		 * Checks argument is a string and returns it.  Raises error if not a
		 * string.
		 * @param narg  argument index.
		 * @return  the argument as a string.
		 */
		public function checkString(narg:int):String
		{
			var s:String = toString(value(narg));
			if (s == null)
			{
				tagError(narg, TSTRING);
			}
			return s;
		}

		/**
		 * Checks the type of an argument, raises error if not matching.
		 * @param narg  argument index.
		 * @param t     typecode (from {@link #type} for example).
		 */
		public function checkType(narg:int, t:int):void
		{
			if (type(narg) != t)
			{
				tagError(narg, t);
			}
		}

		/**
		 * Loads and runs the given string.
		 * @param s  the string to run.
		 * @return  a status code, as per {@link #load}.
		 */
		public function doString(s:String):int
		{
			var status:int = __load(Lua.stringReader(s), s);
			if (status == 0)
			{
				status = pcall(0, MULTRET, null);
			}
			return status;
		}

		private function errfile(what:String, fname:String, e:Error):int
		{
			pushString("cannot " + what + " " + fname + ": " + e.toString());
			return ERRFILE;
		}

		/**
		 * Equivalent to luaL_findtable.  Instead of the table being passed on
		 * the stack, it is passed as the argument <var>t</var>.
		 * Likes its PUC-Rio equivalent however, this method leaves a table on
		 * the Lua stack.
		 */
		public function findTable(t:LuaTable, fname:String, szhint:int):String
		{
			var e:int = 0;
			var i:int = 0;
			do
			{
				e = fname.indexOf('.', i);
				var part:String;
				if (e < 0)
				{
					part = fname.substring(i);
				}
				else
				{
					part = fname.substring(i, e);
				}
				var v:Object = rawGet(t, part);
				if (isNil(v))     // no such field?
				{
					v = createTable(0,
						(e >= 0) ? 1 : szhint);     // new table for field
					setTable(t, part, v);
				}
				else if (!isTable(v))     // field has a non-table value?
				{
					return part;
				}
				t = v as LuaTable;
				i = e + 1;
			} while (e >= 0);
			pushObject(t);
			return null;
		}

		/**
		 * Get a field (event) from an Lua value's metatable.  Returns Lua
		 * <code>nil</code> if there is either no metatable or no field.
		 * @param o           Lua value to get metafield for.
		 * @param event       name of metafield (event).
		 * @return            the field from the metatable, or nil.
		 */
		public function getMetafield(o:Object, event:String):Object
		{
			var mt:LuaTable = getMetatable(o);
			if (mt == null)
			{
				return NIL;
			}
			return mt.getlua(event);
		}

		public function isNoneOrNil(narg:int):Boolean
		{
			return type(narg) <= TNIL;
		}

		/**
		 * Loads a Lua chunk from a file.  The <var>filename</var> argument is
		 * used in a call to {@link Class#getResourceAsStream} where
		 * <code>this</code> is the {@link Lua} instance, thus relative
		 * pathnames will be relative to the location of the
		 * <code>Lua.class</code> file.  Pushes compiled chunk, or error
		 * message, onto stack.
		 * @param filename  location of file.
		 * @return status code, as per {@link #load}.
		 */
		public function loadFile(filename:String):int
		{
			if (filename == null)
			{
				throw new NullPointerException();
			}
			var _in:InputStream = /*getClass()*/SystemUtil.getResourceAsStream(filename); //TODO:
			if (_in == null)
			{
				return errfile("open", filename, new IOException());
			}
			var status:int = 0;
			try
			{
				_in.mark(1);
				var c:int = _in.read();
				if (c == '#'.charCodeAt())       // Unix exec. file?
				{
					// :todo: handle this case
				}
				_in.reset();
				status = load(_in, "@" + filename);
			}
			catch (e:IOException)
			{
				trace(e.getStackTrace())
				return errfile("read", filename, e);
			}
			return status;
		}

		/**
		 * Loads a Lua chunk from a string.  Pushes compiled chunk, or error
		 * message, onto stack.
		 * @param s           the string to load.
		 * @param chunkname   the name of the chunk.
		 * @return status code, as per {@link #load}.
		 */
		public function loadString(s:String, chunkname:String):int
		{
			return __load(stringReader(s), chunkname);
		}

		/**
		 * Get optional integer argument.  Raises error if non-number
		 * supplied.
		 * @param narg  argument index.
		 * @param def   default value for integer.
		 * @return an int.
		 */
		public function optInt(narg:int, def:int):int
		{
			if (isNoneOrNil(narg))
			{
				return def;
			}
			return checkInt(narg);
		}

		/**
		 * Get optional number argument.  Raises error if non-number supplied.
		 * @param narg  argument index.
		 * @param def   default value for number.
		 * @return a double.
		 */
		public function optNumber(narg:int, def:Number):Number
		{
			if (isNoneOrNil(narg))
			{
				return def;
			}
			return checkNumber(narg);
		}

		/**
		 * Get optional string argument.  Raises error if non-string supplied.
		 * @param narg  argument index.
		 * @param def   default value for string.
		 * @return a string.
		 */
		public function optString(narg:int, def:String):String
		{
			if (isNoneOrNil(narg))
			{
				return def;
			}
			return checkString(narg);
		}

		/**
		 * Creates a table in the global namespace and registers it as a loaded
		 * module.
		 * @return the new table
		 */
		public function __register(name:String):LuaTable
		{
			findTable(this._registry, LOADED, 1);
			var loaded:Object = value(-1);
			pop(1);
			var t:Object = getField(loaded, name);
			if (!isTable(t))    // not found?
			{
				// try global variable (and create one if it does not exist)
				if (findTable(getGlobals(), name, 0) != null)
				{
					error("name conflict for module '" + name + "'");
				}
				t = value(-1);
				pop(1);
				setField(loaded, name, t);        // _LOADED[name] = new table
			}
			return t as LuaTable;
		}

		private function tagError(narg:int, tag:int):void
		{
			typerror(narg, typeName(tag));
		}

		/**
		 * Name of type of value at <var>idx</var>.
		 * @param idx  stack index.
		 * @return  the name of the value's type.
		 */
		public function typeNameOfIndex(idx:int):String
		{
			return TYPENAME[type(idx)];
		}

		/**
		 * Declare type error in argument.
		 * @param narg   Index of argument.
		 * @param tname  Name of type expected.
		 */
		public function typerror(narg:int, tname:String):void
		{
			argError(narg, tname + " expected, got " + typeNameOfIndex(narg));
		}

		/**
		 * Return string identifying current position of the control at level
		 * <var>level</var>.
		 * @param level  specifies the call-stack level.
		 * @return a description for that level.
		 */
		public function where(level:int):String 
		{
			var ar:Debug = getStack(level);         // check function at level
			if (ar != null)
			{
				getInfo("Sl", ar);                // get info about it
				if (ar.currentline > 0)         // is there info?
				{
					return ar.shortsrc + ":" + ar.currentline + ": ";
				}
			}
			return "";  // else, no information available...
		}

		/**
		 * Provide {@link java.io.Reader} interface over a <code>String</code>.
		 * Equivalent of {@link java.io.StringReader#StringReader} from J2SE.
		 * The ability to convert a <code>String</code> to a
		 * <code>Reader</code> is required internally,
		 * to provide the Lua function <code>loadstring</code>; exposed
		 * externally as a convenience.
		 * @param s  the string from which to read.
		 * @return a {@link java.io.Reader} that reads successive chars from <var>s</var>.
		 */
		public static function stringReader(s:String):Reader
		{
			return new StringReader(s);
		}

		//////////////////////////////////////////////////////////////////////
		// Debug

		// Methods equivalent to debug API.  In PUC-Rio most of these are in
		// ldebug.c

		public function getInfo(what:String, ar:Debug):Boolean
		{
			var f:Object = null;
			var callinfo:CallInfo = null;
			// :todo: complete me
			if (ar.ici > 0)   // no tail call?
			{
				callinfo = this._civ.elementAt(ar.ici) as CallInfo;
				f = (this._stack[callinfo.func] as Slot).r;
				//# assert isFunction(f)
			}
			var status:Boolean = auxgetinfo(what, ar, f, callinfo);
			if (what.indexOf('f') >= 0)
			{
				if (f == null)
				{
					pushObject(NIL);
				}
				else
				{
					pushObject(f);
				}
			}
			return status;
		}

		/**
		 * Locates function activation at specified call level and returns a
		 * {@link Debug}
		 * record for it, or <code>null</code> if level is too high.
		 * May become public.
		 * @param level  the call level.
		 * @return a {@link Debug} instance describing the activation record.
		 */
		public function getStack(level:int):Debug
		{
			var ici:int;    // Index of CallInfo

			for (ici = this._civ.size - 1; level > 0 && ici > 0; --ici)
			{
				var ci:CallInfo = this._civ.elementAt(ici) as CallInfo;
				--level;
				if (isLua(ci))                    // Lua function?
				{
					level -= ci.tailcalls;        // skip lost tail calls
				}
			}
			if (level == 0 && ici > 0)          // level found?
			{
				return new Debug(ici);
			}
			else if (level < 0)       // level is of a lost tail call?
			{
				return new Debug(0);
			}
			return null;
		}

		/**
		 * Sets the debug hook.
		 */
		public function setHook(func:Hook, mask:int, count:int):void
		{
			if (func == null || mask == 0)      // turn off hooks?
			{
				mask = 0;
				func = null;
			}
			this._hook = func;
			this._basehookcount = count;
			resethookcount();
			this._hookmask = mask;
		}

		/**
		 * @return true is okay, false otherwise (for example, error).
		 */
		private function auxgetinfo(what:String, ar:Debug, f:Object, ci:CallInfo):Boolean
		{
			var status:Boolean = true;
			if (f == null)
			{
				// :todo: implement me
				return status;
			}
			for (var i:int = 0; i < what.length; ++i)
			{
				switch (what.charAt(i))
				{
					case 'S':
						funcinfo(ar, f);
						break;
				
					case 'l':
						ar.currentline = (ci != null) ? currentline(ci) : -1;
						break;
				
					case 'f':       // handled by getInfo
						break;
				
					// :todo: more cases.
					default:
						status = false;
				}
			}
			return status;
		}

		private function currentline(ci:CallInfo):int
		{
			var pc:int = currentpc(ci);
			if (pc < 0)
			{
				return -1;        // only active Lua functions have current-line info
			}
			else
			{
				var faso:Object = (this._stack[ci.func] as Slot).r;
				var f:LuaFunction = faso as LuaFunction;
				return f.proto.getline(pc);
			}
		}

		private function currentpc(ci:CallInfo):int
		{
			if (!isLua(ci))     // function is not a Lua function?
			{
				return -1;
			}
			if (ci == __ci())
			{
				ci.savedpc = this._savedpc;
			}
			return pcRel(ci.savedpc);
		}

		private function funcinfo(ar:Debug, cl:Object):void
		{
			if (cl is LuaJavaCallback)
			{
				ar.source = "=[Java]";
				ar.linedefined = -1;
				ar.lastlinedefined = -1;
				ar.what = "Java";
			}
			else
			{
				var p:Proto = (cl as LuaFunction).proto;
				ar.source = p.source;
				ar.linedefined = p.linedefined;
				ar.lastlinedefined = p.lastlinedefined;
				ar.what = ar.linedefined == 0 ? "main" : "Lua";
			}
		}

		/** Equivalent to macro isLua _and_ f_isLua from lstate.h. */
		private function isLua(callinfo:CallInfo):Boolean
		{
			var f:Object = (this._stack[callinfo.func] as Slot).r;
			return f is LuaFunction;
		}

		private static function pcRel(pc:int):int
		{
			return pc - 1;
		}

		//////////////////////////////////////////////////////////////////////
		// Do

		// Methods equivalent to the file ldo.c.  Prefixed with d.
		// Some of these are in vm* instead.

		/**
		* Equivalent to luaD_callhook.
		*/
		private function dCallhook(event:int, line:int):void
		{
			var hook:Hook = this._hook;
			if (hook != null && this._allowhook)
			{
				var top:int = this._stackSize;
				var ci_top:int = __ci().top;
				var ici:int = this._civ.size - 1;
				if (event == HOOKTAILRET) // not supported yet
				{
					ici = 0;
				}
				var ar:Debug = new Debug(ici);
				ar.event = event;
				ar.currentline = line;
				__ci().top = this._stackSize;
				this._allowhook = false;        // cannot call hooks inside a hook
				hook.luaHook(this, ar);
				//# assert !allowhook
				this._allowhook = true;
				__ci().top = ci_top;
				stacksetsize(top);
			}
		}

		private static const MEMERRMSG:String = "not enough memory";

		/** Equivalent to luaD_seterrorobj.  It is valid for oldtop to be
		* equal to the current stack size (<code>stackSize</code>).
		* {@link #resume} uses this value for oldtop.
		*/
		private function dSeterrorobj(errcode:int, oldtop:int):void
		{
			var msg:Object = objectAt(this._stackSize - 1);
			if (this._stackSize == oldtop)
			{
				stacksetsize(oldtop + 1);
			}
			switch (errcode)
			{
				case ERRMEM:
					(this._stack[oldtop] as Slot).r = MEMERRMSG;
					break;

				case ERRERR:
					(this._stack[oldtop] as Slot).r = "error in error handling";
					break;

				case ERRFILE:
				case ERRRUN:
				case ERRSYNTAX:
					setObjectAt(msg, oldtop);
					break;
			}
			stacksetsize(oldtop+1);
		}

		public function dThrow(status:int):void 
		{
			throw new LuaError(status);
		}


		//////////////////////////////////////////////////////////////////////
		// Func

		// Methods equivalent to the file lfunc.c.  Prefixed with f.

		/** Equivalent of luaF_close.  All open upvalues referencing stack
		 * slots level or higher are closed.
		 * @param level  Absolute stack index.
		 */
		private function fClose(level:int):void
		{
			var i:int = this._openupval.length;
			while (--i >= 0)
			{
				var uv:UpVal = this._openupval[i] as UpVal; //FIXME:var uv:UpVal = this._openupval.elementAt(i) as UpVal;
				if (uv.offset < level)
				{
					break;
				}
				uv.close();
			}
			this._openupval.length = i + 1;
			//openupval.setSize(i+1);
			return;
		}

		private function fFindupval(idx:int):UpVal 
		{
			/*
			 * We search from the end of the Vector towards the beginning,
			 * looking for an UpVal for the required stack-slot.
			 */
			var i:int = this._openupval.length;//FIXME:.size();
			while (--i >= 0)
			{
				var uv2:UpVal = this._openupval[i] as UpVal; //FIXME:var uv2:UpVal = this._openupval.elementAt(i) as UpVal;
				if (uv2.offset == idx)
				{
					return uv2;
				}
				if (uv2.offset < idx)
				{
					break;
				}
			}
			// i points to be position _after_ which we want to insert a new
			// UpVal (it's -1 when we want to insert at the beginning).
			var uv:UpVal = new UpVal(idx, this._stack[idx] as Slot);
			this._openupval.splice(i+1, 0, uv);//FIXME:this._openupval.insertElementAt(uv, i+1);
			return uv;
		}


		//////////////////////////////////////////////////////////////////////
		// Debug

		// Methods equivalent to the file ldebug.c.  Prefixed with g.

		/** <var>p1</var> and <var>p2</var> are operands to a numeric opcode.
		 * Corrupts <code>NUMOP[0]</code>.
		 * There is the possibility of using <var>p1</var> and <var>p2</var> to
		 * identify (for example) for local variable being used in the
		 * computation (consider the error message for code like <code>local
		 * y='a'; return y+1</code> for example).  Currently the debug info is
		 * not used, and this opportunity is wasted (it would require changing
		 * or overloading gTypeerror).
		 */
		private function gAritherror(p1:Slot, p2:Slot):void
		{
			if (!Lua.tonumber(p1, NUMOP))
			{
				p2 = p1;  // first operand is wrong
			}
			gTypeerror(p2, "perform arithmetic on");
		}

		/** <var>p1</var> and <var>p2</var> are absolute stack indexes. */
		private function gConcaterror(p1:int, p2:int):void
		{
			if ((this._stack[p1] as Slot).r is String)
			{
				p1 = p2;
			}
			// assert !(p1 instanceof String);
			gTypeerror(this._stack[p1] as Slot, "concatenate");
		}

		public function gCheckcode(p:Proto):Boolean
		{
			// :todo: implement me.
			return true;
		}

		private function gErrormsg(message:Object):int
		{
			pushObject(message);
			if (this._errfunc != null)        // is there an error handling function
			{
				if (!isFunction(this._errfunc))
				{
					dThrow(ERRERR);
				}
				insert(this._errfunc, getTop());        // push function (under error arg)
				vmCall(this._stackSize - 2, 1);        // call it
			}
			dThrow(ERRRUN);
			// NOTREACHED
			return 0;
		}

		private function gOrdererror(p1:Slot, p2:Slot):Boolean
		{
			var t1:String = typeName(___type(p1));
			var t2:String = typeName(___type(p2));
			if (t1.charAt(2) == t2.charAt(2))
			{
				gRunerror("attempt to compare two " + t1 + "values");
			}
			else
			{
				gRunerror("attempt to compare " + t1 + " with " + t2);
			}
			// NOTREACHED
			return false;
		}

		public function gRunerror(s:String):void
		{
			gErrormsg(s);
		}

		private function gTypeerror(o:Object, op:String):void
		{
			var t:String = typeName(____type(o));
			gRunerror("attempt to " + op + " a " + t + " value");
		}

		private function __gTypeerror(p:Slot, op:String):void
		{
			// :todo: PUC-Rio searches the stack to see if the value (which may
			// be a reference to stack cell) is a local variable.
			// For now we cop out and just call gTypeerror(Object, String)
			gTypeerror(p.asObject(), op);
		}


		//////////////////////////////////////////////////////////////////////
		// Object

		// Methods equivalent to the file lobject.c.  Prefixed with o.

		private static const IDSIZE:int = 60;
		
		/**
		 * @return a string no longer than IDSIZE.
		 */
		public static function oChunkid(source:String):String
		{
			var len:int = IDSIZE;
			if (source.charAt() == "=")
			{
				if (source.length < IDSIZE + 1)
				{
					return source.substring(1);
				}
				else
				{
					return source.substring(1, 1+len);
				}
			}
			// else  "source" or "...source"
			if (source.charAt() == "@")
			{
				source = source.substring(1);
				len -= " '...' ".length;
				var l2:int = source.length;
				if (l2 > len)
				{
					return "..." +  // get last part of file name
						source.substring(source.length - len, source.length);
				}
				return source;
			}
			// else  [string "string"]
			var l:int = source.indexOf('\n');
			if (l == -1)
			{
				l = source.length;
			}
			len -= " [string \"...\"] ".length;
			if (l > len)
			{
				l = len;
			}
			var buf:StringBuffer = new StringBuffer();
			buf.appendString("[string \"");
			buf.appendString(source.substring(0, l));
			if (source.length > l)    // must truncate
			{
				buf.appendString("...");
			}
			buf.appendString("\"]");
			return buf.toString();
		}

		/**
		 * Equivalent to luaO_fb2int.
		 * @see Syntax#oInt2fb
		 */
		private static function oFb2int(x:int):int
		{
			var e:int = (x >>> 3) & 31;
			if (e == 0)
			{
				return x;
			}
			return ((x & 7) + 8) << (e - 1);
		}

		/** Equivalent to luaO_rawequalObj. */
		private static function oRawequal(a:Object, b:Object):Boolean
		{
			// see also vmEqual
			if (NIL == a)
			{
				return NIL == b;
			}
			// Now a is not null, so a.equals() is a valid call.
			// Numbers (Doubles), Booleans, Strings all get compared by value,
			// as they should; tables, functions, get compared by identity as
			// they should.
			return a.equals(b);
		}

		/** Equivalent to luaO_str2d. */
		private static function oStr2d(s:String, out:Array/*double[] */):Boolean
		{
			// :todo: using try/catch may be too slow.  In which case we'll have
			// to recognise the valid formats first.
			try
			{
				out[0] = Number(s);
				return true;
			}
			catch (e0_:NumberFormatException)
			{
				trace(e0_.getStackTrace());
				try
				{
					// Attempt hexadecimal conversion.
					// :todo: using String.trim is not strictly accurate, because it
					// trims other ASCII control characters as well as whitespace.
					s = s.replace(/ /g, "").toUpperCase(); //TODO:
					if (s.substr(0, 2) == "0X")
					{
						s = s.substring(2);  
					}
					else if (s.substr(0, 3) ==  ("-0X"))
					{
						s = "-" + s.substring(3);
					}
					else
					{
						return false;
					}
					out[0] = int(s);// TODO:16进制 16);
					return true;
				}
				catch (e1_:NumberFormatException)
				{
					trace(e1_.getStackTrace());
					return false;
				}
			}
			
			//unreachable
			return false;
		}


		////////////////////////////////////////////////////////////////////////
		// VM

		// Most of the methods in this section are equivalent to the files
		// lvm.c and ldo.c from PUC-Rio.  They're mostly prefixed with vm as
		// well.

		private static const PCRLUA:int =     0;
		private static const PCRJ:int =       1;
		private static const PCRYIELD:int =   2;

		// Instruction decomposition.

		// There follows a series of methods that extract the various fields
		// from a VM instruction.  See lopcodes.h from PUC-Rio.
		// :todo: Consider replacing with m4 macros (or similar).
		// A brief overview of the instruction format:
		// Logically an instruction has an opcode (6 bits), op, and up to
		// three fields using one of three formats:
		// A B C  (8 bits, 9 bits, 9 bits)
		// A Bx   (8 bits, 18 bits)
		// A sBx  (8 bits, 18 bits signed - excess K)
		// Some instructions do not use all the fields (EG OP_UNM only uses A
		// and B).
		// When packed into a word (an int in Jill) the following layouts are
		// used:
		//  31 (MSB)    23 22          14 13         6 5      0 (LSB)
		// +--------------+--------------+------------+--------+
		// | B            | C            | A          | OPCODE |
		// +--------------+--------------+------------+--------+
		//
		// +--------------+--------------+------------+--------+
		// | Bx                          | A          | OPCODE |
		// +--------------+--------------+------------+--------+
		//
		// +--------------+--------------+------------+--------+
		// | sBx                         | A          | OPCODE |
		// +--------------+--------------+------------+--------+

		public static const NO_REG:int = 0xff;       // SIZE_A == 8, (1 << 8)-1

		// Hardwired values for speed.
		/** Equivalent of macro GET_OPCODE */
		public static function OPCODE(instruction:int):int
		{
			// POS_OP == 0 (shift amount)
			// SIZE_OP == 6 (opcode width)
			return instruction & 0x3f;
		}

		/** Equivalent of macro GET_OPCODE */
		public static function SET_OPCODE(i:int, op:int):int
		{
			// POS_OP == 0 (shift amount)
			// SIZE_OP == 6 (opcode width)
			return (i & ~0x3F) | (op & 0x3F);
		}

		/** Equivalent of macro GETARG_A */
		public static function ARGA(instruction:int):int
		{
			// POS_A == POS_OP + SIZE_OP == 6 (shift amount)
			// SIZE_A == 8 (operand width)
			return (instruction >>> 6) & 0xff;
		}

		public static function SETARG_A(i:int, u:int):int
		{
			return (i & ~(0xff << 6)) | ((u & 0xff) << 6);
		}

		/** Equivalent of macro GETARG_B */
		public static function ARGB(instruction:int):int
		{
			// POS_B == POS_OP + SIZE_OP + SIZE_A + SIZE_C == 23 (shift amount)
			// SIZE_B == 9 (operand width)
			/* No mask required as field occupies the most significant bits of a
			 * 32-bit int. */
			return (instruction >>> 23);
		}

		public static function SETARG_B(i:int, b:int):int
		{
			return (i & ~(0x1ff << 23)) | ((b & 0x1ff) << 23);
		}

		/** Equivalent of macro GETARG_C */
		public static function ARGC(instruction:int):int
		{
			// POS_C == POS_OP + SIZE_OP + SIZE_A == 14 (shift amount)
			// SIZE_C == 9 (operand width)
			return (instruction >>> 14) & 0x1ff;
		}

		public static function SETARG_C(i:int, c:int):int
		{
			return (i & ~(0x1ff << 14)) | ((c & 0x1ff) << 14);
		}

		/** Equivalent of macro GETARG_Bx */
		public static function ARGBx(instruction:int):int
		{
			// POS_Bx = POS_C == 14
			// SIZE_Bx == SIZE_C + SIZE_B == 18
			/* No mask required as field occupies the most significant bits of a
			 * 32 bit int. */
			return (instruction >>> 14);
		}

		public static function SETARG_Bx(i:int, bx:int):int
		{
			return (i & 0x3fff) | (bx << 14) ;
		}


		/** Equivalent of macro GETARG_sBx */
		public static function ARGsBx(instruction:int):int
		{
			// As ARGBx but with (2**17-1) subtracted.
			return (instruction >>> 14) - MAXARG_sBx;
		}
		
		public static function SETARG_sBx(i:int, bx:int):int
		{
			return (i & 0x3fff) | ((bx+MAXARG_sBx) << 14) ;  // CHECK THIS IS RIGHT
		}

		public static function ISK(field:int):Boolean
		{
			// The "is constant" bit position depends on the size of the B and C
			// fields (required to be the same width).
			// SIZE_B == 9
			return field >= 0x100;
		}

		/**
		* Near equivalent of macros RKB and RKC.  Note: non-static as it
		* requires stack and base instance members.  Stands for "Register or
		* Konstant" by the way, it gets value from either the register file
		* (stack) or the constant array (k).
		*/
		private function RK(k:Array/*Slot[] */, field:int):Slot
		{
			if (ISK(field))
			{
				return k[field & 0xff] as Slot;
			}
			return this._stack[this._base + field] as Slot;
		}

		/**
		* Slower version of RK that does not receive the constant array.  Not
		* recommend for routine use, but is used by some error handling code
		* to avoid having a constant array passed around too much.
		*/
		private function __RK(field:int):Slot
		{
			var _function:LuaFunction = (this._stack[__ci().func] as Slot).r as LuaFunction;
			var k:Array = _function.proto.constant; //Slot[]
			return RK(k, field);
		}

		// CREATE functions are required by FuncState, so default access.
		public static function CREATE_ABC(o:int, a:int, b:int, c:int):int
		{
			// POS_OP == 0
			// POS_A == 6
			// POS_B == 23
			// POS_C == 14
			return o | (a << 6) | (b << 23) | (c << 14);
		}

		public static function CREATE_ABx(o:int, a:int, bc:int):int
		{
			// POS_OP == 0
			// POS_A == 6
			// POS_Bx == POS_C == 14
			return o | (a << 6) | (bc << 14);
		}

		// opcode enumeration.
		// Generated by a script:
		// awk -f opcode.awk < lopcodes.h
		// and then pasted into here.
		// Made default access so that code generation, in FuncState, can see
		// the enumeration as well.

		public static const OP_MOVE:int = 0;
		public static const OP_LOADK:int = 1;
		public static const OP_LOADBOOL:int = 2;
		public static const OP_LOADNIL:int = 3;
		public static const OP_GETUPVAL:int = 4;
		public static const OP_GETGLOBAL:int = 5;
		public static const OP_GETTABLE:int = 6;
		public static const OP_SETGLOBAL:int = 7;
		public static const OP_SETUPVAL:int = 8;
		public static const OP_SETTABLE:int = 9;
		public static const OP_NEWTABLE:int = 10;
		public static const OP_SELF:int = 11;
		public static const OP_ADD:int = 12;
		public static const OP_SUB:int = 13;
		public static const OP_MUL:int = 14;
		public static const OP_DIV:int = 15;
		public static const OP_MOD:int = 16;
		public static const OP_POW:int = 17;
		public static const OP_UNM:int = 18;
		public static const OP_NOT:int = 19;
		public static const OP_LEN:int = 20;
		public static const OP_CONCAT:int = 21;
		public static const OP_JMP:int = 22;
		public static const OP_EQ:int = 23;
		public static const OP_LT:int = 24;
		public static const OP_LE:int = 25;
		public static const OP_TEST:int = 26;
		public static const OP_TESTSET:int = 27;
		public static const OP_CALL:int = 28;
		public static const OP_TAILCALL:int = 29;
		public static const OP_RETURN:int = 30;
		public static const OP_FORLOOP:int = 31;
		public static const OP_FORPREP:int = 32;
		public static const OP_TFORLOOP:int = 33;
		public static const OP_SETLIST:int = 34;
		public static const OP_CLOSE:int = 35;
		public static const OP_CLOSURE:int = 36;
		public static const OP_VARARG:int = 37;

		// end of instruction decomposition

		public static const SIZE_C:int = 9;
		public static const SIZE_B:int = 9;
		public static const SIZE_Bx:int = SIZE_C + SIZE_B;
		public static const SIZE_A:int = 8;

		public static const SIZE_OP:int = 6;

		public static const POS_OP:int = 0;
		public static const POS_A:int = POS_OP + SIZE_OP;
		public static const POS_C:int = POS_A + SIZE_A;
		public static const POS_B:int = POS_C + SIZE_C;
		public static const POS_Bx:int = POS_C;

		public static const MAXARG_Bx:int = (1<<SIZE_Bx)-1;
		public static const MAXARG_sBx:int = MAXARG_Bx>>1;    // `sBx' is signed


		public static const MAXARG_A:int = (1<<SIZE_A)-1;
		public static const MAXARG_B:int = (1<<SIZE_B)-1;
		public static const MAXARG_C:int = (1<<SIZE_C)-1;

		/* this bit 1 means constant (0 means register) */
		public static const BITRK:int = 1 << (SIZE_B - 1) ;
		public static const MAXINDEXRK:int = BITRK - 1 ;


		/**
		 * Equivalent of luaD_call.
		 * @param func  absolute stack index of function to call.
		 * @param r     number of required results.
		 */
		private function vmCall(func:int, r:int):void
		{
			++this._nCcalls;
			if (vmPrecall(func, r) == PCRLUA)
			{
				vmExecute(1);
			}
			--this._nCcalls;
		}
		
		/** Equivalent of luaV_concat. */
		private function vmConcat(total:int, last:int):void
		{
			do
			{
				var top:int = this._base + last + 1;
				var n:int = 2;  // number of elements handled in this pass (at least 2)
				if (!tostring(top-2)|| !tostring(top-1))
				{
					if (!call_binTM(this._stack[top - 2] as Slot, this._stack[top - 1],
						this._stack[top - 2] as Slot, "__concat"))
					{
						gConcaterror(top-2, top-1);
					}
				}
				else if (((this._stack[top - 1] as Slot).r as String).length > 0)
				{
					var tl:int = ((this._stack[top - 1] as Slot).r as String).length;
					for (n = 1; n < total && tostring(top-n-1); ++n)
					{
						tl += ((this._stack[top - n - 1] as Slot).r as String).length;
						if (tl < 0)
						{
							gRunerror("string length overflow");
						}
					}
					var buffer:StringBuffer = new StringBuffer();
					buffer.init(tl);
					for (var i:int = n; i > 0; i--)         // concat all strings
					{
						buffer.appendString((this._stack[top - i] as Slot).r as String);
					}
					(this._stack[top - n] as Slot).r = buffer.toString();
				}
				total -= n-1;     // got n strings to create 1 new
				last -= n-1;
			} while (total > 1); // repeat until only 1 result left
		}

		/**
		 * Primitive for testing Lua equality of two values.  Equivalent of
		 * PUC-Rio's <code>equalobj</code> macro.
		 * In the loosest sense, this is the equivalent of
		 * <code>luaV_equalval</code>.
		 */
		private function vmEqual(a:Slot, b:Slot):Boolean
		{
			// Deal with number case first
			if (NUMBER == a.r)
			{
				if (NUMBER != b.r)
				{
					return false;
				}
				return a.d == b.d;
			}
			// Now we're only concerned with the .r field.
			return vmEqualRef(a.r, b.r);
		}
		
		/**
		 * Part of {@link #vmEqual}.  Compares the reference part of two
		 * Slot instances.  That is, compares two Lua values, as long as
		 * neither is a number.
		 */
		private function vmEqualRef(a:Object, b:Object):Boolean
		{
			if (a.equals(b))
			{
				return true;
			}
			//TODO:
			//if (a.getClass != b.getClass())
			if (getQualifiedClassName(a) != getQualifiedClassName(b))
			{
				return false;
			}
			// Same class, but different objects.
			if (a is LuaJavaCallback ||
				a is LuaTable)
			{
				// Resort to metamethods.
				var tm:Object = get_compTM(getMetatable(a), getMetatable(b), "__eq");
				if (NIL == tm)    // no TM?
				{
					return false;
				}
				var s:Slot = new Slot();
				callTMres(s, tm, a as Slot, b as Slot);   // call TM   //TODO:
				return !isFalse(s.r);
			}
			return false;
		}

		/**
		 * Array of numeric operands.  Used when converting strings to numbers
		 * by an arithmetic opcode (ADD, SUB, MUL, DIV, MOD, POW, UNM).
		 */
		private static var NUMOP:Array = new Array(2); //double[]

		/** The core VM execution engine. */
		private function vmExecute(nexeccalls:int):void
		{
			// This labelled while loop is used to simulate the effect of C's
			// goto.  The end of the while loop is never reached.  The beginning
			// of the while loop is branched to using a "continue reentry;"
			// statement (when a Lua function is called or returns).
		reentry:
			while (true)
			{
				// assert stack[ci.function()].r instanceof LuaFunction;
				var _function:LuaFunction = (this._stack[__ci().func] as Slot).r as LuaFunction;
				var proto:Proto = _function.proto;
				var code:Array = proto.code; //int[]
				var k:Array = proto.constant; //Slot[] 
				var pc:int = this._savedpc;

				while (true)        // main loop of interpreter
				{
					
					// Where the PUC-Rio code used the Protect macro, this has been
					// replaced with "savedpc = pc" and a "// Protect" comment.
				
					// Where the PUC-Rio code used the dojump macro, this has been
					// replaced with the equivalent increment of the pc and a
					// "//dojump" comment.
					
					var i:int = code[pc++];       // VM instruction.
					// :todo: line hook
					if ((this._hookmask & MASKCOUNT) != 0 && --this._hookcount == 0)
					{
						traceexec(pc);
						if (status == YIELD)  // did hook yield?
						{
							this._savedpc = pc - 1;
							return;
						}
						// base = this.base
					}
					
					var a:int  = ARGA(i);          // its A field.
					var rb:Slot;
					var rc:Slot;
						

					switch (OPCODE(i))
					{
						case OP_MOVE:
							(this._stack[this._base + a] as Slot).r = (this._stack[this._base + ARGB(i)] as Slot).r;
							(this._stack[this._base + a] as Slot).d = (this._stack[this._base + ARGB(i)] as Slot).d;
							continue;
				  
						case OP_LOADK:
							(this._stack[this._base + a] as Slot).r = (k[ARGBx(i)] as Slot).r;
							(this._stack[this._base + a] as Slot).d = (k[ARGBx(i)] as Slot).d;
							if (D) 
							{
								trace("OP_LOADK:stack[" + (this._base+a) + 
									"]=k[" + ARGBx(i) + "]=" + k[ARGBx(i)].d);
							}
							continue;
				  
						case OP_LOADBOOL:
							(this._stack[this._base + a] as Slot).r = valueOfBoolean(ARGB(i) != 0);
							if (ARGC(i) != 0)
							{
								++pc;
							}
							continue;
				  
						case OP_LOADNIL:
							{
								var b:int = this._base + ARGB(i);
								do
								{
									(this._stack[b--] as Slot).r = NIL;
								} while (b >= this._base + a);
								continue;
							}
				  
						case OP_GETUPVAL:
							{
								var b2:int = ARGB(i);
								// :todo: optimise path
								setObjectAt(_function.upVal(b2).value, this._base + a);
								continue;
							}
						
						case OP_GETGLOBAL:
							rb = k[ARGBx(i)];
							// assert rb instance of String;
							this._savedpc = pc; // Protect
							vmGettable(_function.env, rb, this._stack[this._base + a] as Slot);
							continue;
						
						case OP_GETTABLE:
							{
								this._savedpc = pc; // Protect
								var h:Object = (this._stack[this._base + ARGB(i)] as Slot).asObject();
								if (D)
								{
									trace("OP_GETTABLE index = " + (this._base + ARGB(i)) + 
										", size = " + this._stack.length +
										", h = " + h);
								}
								vmGettable(h, RK(k, ARGC(i)), this._stack[this._base + a] as Slot);
								continue;
							}
						
						case OP_SETUPVAL:
							{
								var uv:UpVal = _function.upVal(ARGB(i));
								uv.value = objectAt(this._base + a);
								continue;
							}
						
						case OP_SETGLOBAL:
							this._savedpc = pc; // Protect
							// :todo: consider inlining objectAt
							vmSettable(_function.env, k[ARGBx(i)] as Slot,
								objectAt(this._base + a));
							continue;
						
						case OP_SETTABLE:
							{
								this._savedpc = pc; // Protect
								var t:Object = this._stack[this._base + a].asObject();
								vmSettable(t, RK(k, ARGB(i)), RK(k, ARGC(i)).asObject());
								continue;
							}
						
						case OP_NEWTABLE:
							{
								var b3:int = ARGB(i);
								var c:int = ARGC(i);
								(this._stack[this._base + a] as Slot).r = new LuaTable();
								((this._stack[this._base + a] as Slot).r as LuaTable).init(oFb2int(b3), oFb2int(c));
								continue;
							}
						
						case OP_SELF:
							{
								var b4:int = ARGB(i);
								rb = this._stack[this._base + b4];
								(this._stack[this._base + a + 1] as Slot).r = rb.r;
								(this._stack[this._base + a + 1] as Slot).d = rb.d;
								this._savedpc = pc; // Protect
								vmGettable(rb.asObject(), RK(k, ARGC(i)), (this._stack[this._base + a] as Slot));
								continue;
							}
						
						case OP_ADD:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var sum:Number = rb.d + rc.d;
								(this._stack[this._base+a] as Slot).d = sum;
								(this._stack[this._base+a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var sum2:Number = NUMOP[0] + NUMOP[1];
								(this._stack[this._base + a] as Slot).d = sum2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__add"))
							{
								gAritherror(rb, rc);
							}
							continue;
						
						case OP_SUB:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var difference:Number = rb.d - rc.d;
								(this._stack[this._base + a] as Slot).d = difference;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var difference2:Number = (NUMOP[0] as Number) - (NUMOP[1] as Number);
								(this._stack[this._base + a] as Slot).d = difference2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__sub"))
							{
								gAritherror(rb, rc);
							}
							continue;
						
						case OP_MUL:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var product:Number = rb.d * rc.d;
								(this._stack[this._base + a] as Slot).d = product;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var product2:Number = (NUMOP[0] as Number) * (NUMOP[1] as Number);
								(this._stack[this._base + a] as Slot).d = product2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__mul"))
							{
							  	gAritherror(rb, rc);
							}
							continue;
						
						case OP_DIV:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var quotient:Number = rb.d / rc.d;
								(this._stack[this._base + a] as Slot).d = quotient;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var quotient2:Number = (NUMOP[0] as Number) / (NUMOP[1] as Number);
								(this._stack[this._base + a] as Slot).d = quotient2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__div"))
							{
								gAritherror(rb, rc);
							}
							continue;
							
						case OP_MOD:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var modulus:Number = __modulus(rb.d, rc.d);
								(this._stack[this._base + a] as Slot).d = modulus;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var modulus2:Number = __modulus(NUMOP[0] as Number, NUMOP[1] as Number);
								(this._stack[this._base + a] as Slot).d = modulus2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__mod"))
							{
								gAritherror(rb, rc);
							}
							continue;
						
						case OP_POW:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (rb.r == NUMBER && rc.r == NUMBER)
							{
								var result:Number = iNumpow(rb.d, rc.d);
								(this._stack[this._base + a] as Slot).d = result;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (toNumberPair(rb, rc, NUMOP))
							{
								var result2:Number = iNumpow(NUMOP[0], NUMOP[1]);
								(this._stack[this._base + a] as Slot).d = result2;
								(this._stack[this._base + a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rc, this._stack[this._base + a] as Slot, "__pow"))
							{
								gAritherror(rb, rc);
							}
							continue;
							
						case OP_UNM:
							rb = this._stack[this._base + ARGB(i)] as Slot;
							if (rb.r == NUMBER)
							{
								(this._stack[this._base+a] as Slot).d = -rb.d;
								(this._stack[this._base+a] as Slot).r = NUMBER;
							}
							else if (Lua.tonumber(rb, NUMOP))
							{
								(this._stack[this._base+a] as Slot).d = -(NUMOP[0] as Number);
								(this._stack[this._base+a] as Slot).r = NUMBER;
							}
							else if (!call_binTM(rb, rb, this._stack[this._base + a] as Slot, "__unm"))
							{
								gAritherror(rb, rb);
							}
							continue;
						
						case OP_NOT:
							{
								// All numbers are treated as true, so no need to examine
								// the .d field.
								var ra:Object = (this._stack[this._base + ARGB(i)] as Slot).r;
								(this._stack[this._base+a] as Slot).r = valueOfBoolean(isFalse(ra));
								continue;
							}
							
						case OP_LEN:
							rb = this._stack[this._base + ARGB(i)] as Slot;
							if (rb.r is LuaTable)
							{
								var t2:LuaTable = rb.r as LuaTable;
								(this._stack[this._base + a] as Slot).d = t2.getn();
								(this._stack[this._base + a] as Slot).r = NUMBER;
								continue;
							}
							else if (rb.r is String)
							{
								var s:String = rb.r as String;
								(this._stack[this._base + a] as Slot).d = s.length;
								(this._stack[this._base + a] as Slot).r = NUMBER;
								continue;
							}
							this._savedpc = pc; // Protect
							if (!call_binTM(rb, rb, this._stack[this._base + a] as Slot, "__len"))
							{
								gTypeerror(rb, "get length of");
							}
							continue;
						
						case OP_CONCAT:
							{
								var b_CONCAT:int = ARGB(i);
								var c_CONCAT:int = ARGC(i);
								this._savedpc = pc; // Protect
								// :todo: The compiler assumes that all
								// stack locations _above_ b end up with junk in them.  In
								// which case we can improve the speed of vmConcat (by not
								// converting each stack slot, but simply using
								// StringBuffer.append on whatever is there).
								vmConcat(c_CONCAT - b_CONCAT + 1, c_CONCAT);
								(this._stack[this._base + a] as Slot).r = (this._stack[this._base + b_CONCAT] as Slot).r;
								(this._stack[this._base + a] as Slot).d = (this._stack[this._base + b_CONCAT] as Slot).d;
								continue;
							}
						
						case OP_JMP:
							// dojump
							pc += ARGsBx(i);
							continue;
						
						case OP_EQ:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							if (vmEqual(rb, rc) == (a != 0))
							{
								// dojump
								pc += ARGsBx(code[pc] as int);
							}
							++pc;
							continue;
						
						case OP_LT:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							this._savedpc = pc; // Protect
							if (vmLessthan(rb, rc) == (a != 0))
							{
								// dojump
								pc += ARGsBx(code[pc] as int);
							}
							++pc;
							continue;
						
						case OP_LE:
							rb = RK(k, ARGB(i));
							rc = RK(k, ARGC(i));
							this._savedpc = pc; // Protect
							if (vmLessequal(rb, rc) == (a != 0))
							{
								// dojump
								pc += ARGsBx(code[pc] as int);
							}
							++pc;
							continue;
						
						case OP_TEST:
							if (isFalse((this._stack[this._base + a] as Slot).r) != (ARGC(i) != 0))
							{
								// dojump
								pc += ARGsBx(code[pc] as int);
							}
							++pc;
							continue;
						
						case OP_TESTSET:
							rb = this._stack[this._base + ARGB(i)] as Slot;
							if (isFalse(rb.r) != (ARGC(i) != 0))
							{
								(this._stack[this._base + a] as Slot).r = rb.r;
								(this._stack[this._base + a] as Slot).d = rb.d;
							  	// dojump
							  	pc += ARGsBx(code[pc] as int);
							}
							++pc;
							continue;
						
						case OP_CALL:
							{
								var b_CALL:int = ARGB(i);
								var nresults:int = ARGC(i) - 1;
								if (b != 0)
								{
									stacksetsize(this._base + a + b);
								}
								this._savedpc = pc;
								switch (vmPrecall(this._base + a, nresults))
								{
									case PCRLUA:
										nexeccalls++;
										continue reentry;
									
									case PCRJ:
										// Was Java function called by precall, adjust result
										if (nresults >= 0)
										{
											stacksetsize(__ci().top);
										}
										continue;
									
									default:
										return; // yield
								}
							}
							
						case OP_TAILCALL:
							{
								var b_TAILCALL:int = ARGB(i);
								if (b_TAILCALL != 0)
								{
									stacksetsize(this._base + a + b_TAILCALL);
								}
								this._savedpc = pc;
								// assert ARGC(i) - 1 == MULTRET
								switch (vmPrecall(this._base + a, MULTRET))
								{
									case PCRLUA:
										{
											// tail call: put new frame in place of previous one.
											var ci:CallInfo = this._civ.elementAt(this._civ.size - 2) as CallInfo;
											var func:int = ci.func;
											var fci:CallInfo = __ci();    // Fresh CallInfo
											var pfunc:int = fci.func;
											fClose(ci.base);
											this._base = func + (fci.base - pfunc);
											var aux:int;        // loop index is used after loop ends
											for (aux=0; pfunc + aux < this._stackSize; ++aux)
											{
												// move frame down
												(this._stack[func + aux] as Slot).r = (this._stack[pfunc + aux] as Slot).r;
												(this._stack[func + aux] as Slot).d = (this._stack[pfunc + aux] as Slot).d;
											}
											stacksetsize(func + aux);        // correct top
											// assert stackSize == base + ((LuaFunction)stack[func]).proto().maxstacksize();
											ci.tailcall(this._base, this._stackSize);
											dec_ci();       // remove new frame.
											continue reentry;
										}
									
									case PCRJ:        // It was a Java function
										{
											continue;
										}
									
									default:
										{
											return; // yield
										}
								}
							}
						
						case OP_RETURN:
							{
								fClose(this._base);
								var b_RETURN:int = ARGB(i);
								if (b_RETURN != 0)
								{
									var top:int = a + b_RETURN - 1;
									stacksetsize(this._base + top);
								}
								this._savedpc = pc;
								// 'adjust' replaces aliased 'b' in PUC-Rio code.
								var adjust:Boolean = vmPoscall(this._base + a);
								if (--nexeccalls == 0)
								{
									return;
								}
								if (adjust)
								{
									stacksetsize(__ci().top);
								}
								continue reentry;
							}
						
						case OP_FORLOOP:
							{
								var step:Number = (this._stack[this._base + a + 2] as Slot).d;
								var idx:Number = (this._stack[this._base + a] as Slot).d + step;
								var limit:Number = (this._stack[this._base + a + 1] as Slot).d;
								if ((0 < step && idx <= limit) ||
									(step <= 0 && limit <= idx))
								{
									// dojump
									pc += ARGsBx(i);
									(this._stack[this._base + a] as Slot).d = idx;    // internal index
									(this._stack[this._base + a] as Slot).r = NUMBER;
									(this._stack[this._base + a + 3] as Slot).d = idx;  // external index
									(this._stack[this._base + a + 3] as Slot).r = NUMBER;
								}
								continue;
							}
						
						case OP_FORPREP:
							{
								var init:int = this._base + a;
								var plimit:int = this._base + a + 1;
								var pstep:int = this._base + a + 2;
								this._savedpc = pc;       // next steps may throw errors
								if (!tonumber(init))
								{
									gRunerror("'for' initial value must be a number");
								}
								else if (!tonumber(plimit))
								{
									gRunerror("'for' limit must be a number");
								}
								else if (!tonumber(pstep))
								{
									gRunerror("'for' step must be a number");
								}
								var step_FORPREP:Number = (this._stack[pstep] as Slot).d;
								var idx_FORPREP:Number = (this._stack[init] as Slot).d - step_FORPREP;
								(this._stack[init] as Slot).d = idx_FORPREP;
								(this._stack[init] as Slot).r = NUMBER;
								// dojump
								pc += ARGsBx(i);
								continue;
							}
							
						
						case OP_TFORLOOP:
							{
								var cb:int = this._base+a+3;  // call base
								(this._stack[cb + 2] as Slot).r = (this._stack[this._base + a + 2] as Slot).r;
								(this._stack[cb + 2] as Slot).d = (this._stack[this._base + a + 2] as Slot).d;
								(this._stack[cb + 1] as Slot).r = (this._stack[this._base + a + 1] as Slot).r;
								(this._stack[cb + 1] as Slot).d = (this._stack[this._base + a + 1] as Slot).d;
								(this._stack[cb] as Slot).r = (this._stack[this._base + a] as Slot).r;
								(this._stack[cb] as Slot).d = (this._stack[this._base + a] as Slot).d;
								stacksetsize(cb + 3);
								this._savedpc = pc; // Protect
								vmCall(cb, ARGC(i));
								stacksetsize(__ci().top);
								if (NIL != (this._stack[cb] as Slot).r)     // continue loop
								{
									(this._stack[cb - 1] as Slot).r = (this._stack[cb] as Slot).r;
									(this._stack[cb - 1] as Slot).d = (this._stack[cb] as Slot).d;
									// dojump
									pc += ARGsBx(code[pc] as int);
								}
								++pc;
								continue;
							}
						
						case OP_SETLIST:
							{
								var n:int = ARGB(i);
								var c_SETLIST:int = ARGC(i);
								var setstack:Boolean = false;
								if (0 == n)
								{
									n = (this._stackSize - (this._base + a)) - 1;
									setstack = true;
								}
								if (0 == c_SETLIST)
								{
									c_SETLIST = code[pc++] as int;
								}
								var t3:LuaTable = (this._stack[this._base+a] as Slot).r as LuaTable;
								var last:int = ((c_SETLIST - 1) * LFIELDS_PER_FLUSH) + n;
								// :todo: consider expanding space in table
								for (; n > 0; n--)
								{
									var val:Object = objectAt(this._base + a + n);
									t3.putnum(last--, val);
								}
								if (setstack)
								{
									stacksetsize(__ci().top);
								}
								continue;
							}
						
						case OP_CLOSE:
							fClose(this._base + a);
							continue;
						
						case OP_CLOSURE:
							{
								var p:Proto = _function.proto.proto[ARGBx(i)] as Proto;
								var nup:int = p.nups;
								var up:Array = new Array(nup); //UpVal[] 
								for (var j:int = 0; j < nup; j++, pc++)
								{
									var _in:int = code[pc] as int;
									if (OPCODE(_in) == OP_GETUPVAL)
									{
										up[j] = _function.upVal(ARGB(_in)) as UpVal;
									}
									else
									{
										// assert OPCODE(in) == OP_MOVE;
										up[j] = fFindupval(this._base + ARGB(_in)) as UpVal;
									}	
								}
								var nf:LuaFunction = new LuaFunction(p, up, _function.env);
								//up = null;
								(this._stack[this._base + a] as Slot).r = nf;
								continue;
							}
						
						case OP_VARARG:
							{
								var b_VARARG:int = ARGB(i) - 1;
								var n_VARARG:int = (this._base - __ci().func) -
									_function.proto.numparams - 1;
								if (b_VARARG == MULTRET)
								{
									// :todo: Protect
									// :todo: check stack
									b_VARARG = n_VARARG;
									stacksetsize(this._base + a + n_VARARG);
								}
								for (var j_VARARG:int = 0; j_VARARG < b; ++j_VARARG)
								{
									if (j_VARARG < n_VARARG)
									{
										var src:Slot = this._stack[this._base - n_VARARG + j_VARARG] as Slot;
										(this._stack[this._base + a + j_VARARG] as Slot).r = src.r;
										(this._stack[this._base + a + j_VARARG] as Slot).d = src.d;
									}
									else
									{
										(this._stack[this._base + a + j_VARARG] as Slot).r = NIL;
									}
								}
								continue;
							}							
					} /* switch */
				} /* while */
			} /* reentry: while */
		}

		public static function iNumpow(a:Number, b:Number):Number
		{
			// :todo: this needs proper checking for boundary cases
			// EG, is currently wrong for (-0)^2.
			var invert:Boolean = b < 0.0;
			if (invert) 
				b = -b;
			if (a == 0.0)
				return invert ? NaN : a ;
			var result:Number = 1.0 ;
			var ipow:int = b as int;
			b -= ipow ;
			var t:Number = a ;
			while (ipow > 0)
			{
				if ((ipow & 1) != 0)
					result *= t ;
				ipow >>= 1 ;
				t = t*t ;
			}
			if (b != 0.0) // integer only case, save doing unnecessary work
			{
				if (a < 0.0)  // doesn't work if a negative (complex result!)
					return NaN;
				t = Math.sqrt(a);
				var half:Number = 0.5;
				while (b > 0.0)
				{
					if (b >= half)
					{
						result = result * t ;
						b -= half ;
					}
					b = b+b ;
					t = Math.sqrt(t) ;
					if (t == 1.0)
						break ;
				}
			}
			return invert ?  1.0 / result : result ;
		}

		/** Equivalent of luaV_gettable. */
		private function vmGettable(t:Object, key:Slot, val:Slot):void
		{
			var tm:Object;
			for (var loop:int = 0; loop < MAXTAGLOOP; ++loop)
			{
				if (t is LuaTable)        // 't' is a table?
				{
					var h:LuaTable = t as LuaTable;
					h.__getlua(key, SPARE_SLOT);

					if (SPARE_SLOT.r != NIL)
					{
						val.r = SPARE_SLOT.r;
						val.d = SPARE_SLOT.d;
						return;
					}
					tm = tagmethod(h, "__index");
					if (tm == NIL)
					{
						val.r = NIL;
						return;
					}
					// else will try the tag method
				}
				else
				{
					tm = tagmethod(t, "__index");
					if (tm == NIL)
						gTypeerror(t, "index");
				}
				if (isFunction(tm))
				{
					SPARE_SLOT.setObject(t);
					callTMres(val, tm, SPARE_SLOT, key);
					return;
				}
				t = tm;     // else repeat with 'tm'
			}
			gRunerror("loop in gettable");
		}

		/** Equivalent of luaV_lessthan. */
		private function vmLessthan(l:Slot, r:Slot):Boolean
		{
			//TODO:
			//if (l.r.getClass() != r.r.getClass())
			if(getQualifiedClassName(l.r) != getQualifiedClassName(r.r))
			{
				gOrdererror(l, r);
			}
			else if (l.r == NUMBER)
			{
				return l.d < r.d;
			}
			else if (l.r is String)
			{
				// :todo: PUC-Rio use strcoll, maybe we should use something
				// equivalent.
				return (l.r as String) < (r.r as String); //TODO:compareTo
			}
			var res:int = call_orderTM(l, r, "__lt");
			if (res >= 0)
			{
				return res != 0;
			}
			return gOrdererror(l, r);
		}

		/** Equivalent of luaV_lessequal. */
		private function vmLessequal(l:Slot, r:Slot):Boolean
		{
			//TODO:
			//if (l.r.getClass() != r.r.getClass())
			if(getQualifiedClassName(l.r) != getQualifiedClassName(r.r))
			{
				gOrdererror(l, r);
			}
			else if (l.r == NUMBER)
			{
				return l.d <= r.d;
			}
			else if (l.r is String)
			{
				return (l.r as String) <= (r.r as String); //TODO: CompareTo
			}
			var res:int = call_orderTM(l, r, "__le");       // first try 'le'
			if (res >= 0)
			{
				return res != 0;
			}
			res = call_orderTM(r, l, "__lt");   // else try 'lt'
			if (res >= 0)
			{
				return res == 0;
			}
			return gOrdererror(l, r);
		}

	    /**
	     * Equivalent of luaD_poscall.
	     * @param firstResult  stack index (absolute) of the first result
	     */
		private function vmPoscall(firstResult:int):Boolean
		{
			// :todo: call hook
			var lci:CallInfo; // local copy, for faster access
			lci = dec_ci();
			// Now (as a result of the dec_ci call), lci is the CallInfo record
			// for the current function (the function executing an OP_RETURN
			// instruction), and this.ci is the CallInfo record for the function
			// we are returning to.
			var res:int = lci.res();
			var wanted:int = lci.nresults;        // Caution: wanted could be == MULTRET
			var cci:CallInfo = __ci();        // Continuation CallInfo
			this._base = cci.base;
			this._savedpc = cci.savedpc;
			// Move results (and pad with nils to required number if necessary)
			var i:int = wanted;
			var top:int = this._stackSize;
			// The movement is always downwards, so copying from the top-most
			// result first is always correct.
			while (i != 0 && firstResult < top)
			{
				(this._stack[res] as Slot).r = (this._stack[firstResult] as Slot).r;
				(this._stack[res] as Slot).d = (this._stack[firstResult] as Slot).d;
				++res;
				++firstResult;
				i--;
			}
			if (i > 0)
			{
				stacksetsize(res+i);
			}
			// :todo: consider using two stacksetsize calls to nil out
			// remaining required results.
			while (i-- > 0)
			{
				(this._stack[res++] as Slot).r = NIL;
			}
			stacksetsize(res);
			return wanted != MULTRET;
		}

		/**
		* Equivalent of LuaD_precall.  This method expects that the arguments
		* to the function are placed above the function on the stack.
		* @param func  absolute stack index of the function to call.
		* @param r     number of results expected.
		*/
		private function vmPrecall(func:int, r:int):int
		{
			var faso:Object;        // Function AS Object
			faso = (this._stack[func] as Slot).r;
			if (!isFunction(faso))
			{
				faso = tryfuncTM(func);
			}
			__ci().savedpc = this._savedpc;
			if (faso is LuaFunction)
			{
				var f:LuaFunction = faso as LuaFunction;
				var p:Proto = f.proto;
				// :todo: ensure enough stack
				
				if (!p.isVararg)
				{
					this._base = func + 1;
					if (this._stackSize > this._base + p.numparams)
					{
						// trim stack to the argument list
						stacksetsize(this._base + p.numparams);
					}
				}
				else
				{
					var nargs:int = (this._stackSize - func) - 1;
					this._base = adjust_varargs(p, nargs);
				}

				var top:int = this._base + p.maxstacksize;
				inc_ci(func, this._base, top, r);
			
				this._savedpc = 0;
				// expand stack to the function's max stack size.
				stacksetsize(top);
				// :todo: implement call hook.
				return PCRLUA;
			}
			else if (faso is LuaJavaCallback)
			{
				var fj:LuaJavaCallback = faso as LuaJavaCallback;
				// :todo: checkstack (not sure it's necessary)
				this._base = func + 1;
				inc_ci(func, this._base, this._stackSize + MINSTACK, r);
				// :todo: call hook
				var n:int = 99;
				try
				{
					n = fj.luaFunction(this);
				}
				catch (e:Error)
				{
					if (e is LuaError)
					{
						var e1:LuaError = e as LuaError;
						trace(e1.getStackTrace());
						throw e1;
					}
					else if (e is RuntimeException)
					{
						var e2:RuntimeException = e as RuntimeException; 
						trace(e2.getStackTrace());
						yield(0);
						throw e2;
					}
				}
				if (n < 0)        // yielding?
				{
					return PCRYIELD;
				}
				else
				{
					vmPoscall(this._stackSize - n);
					return PCRJ;
				}
			}
			
			throw new IllegalArgumentException();
		}

		/** Equivalent of luaV_settable. */
		private function vmSettable(t:Object, key:Slot, val:Object):void
		{
			for (var loop:int = 0; loop < MAXTAGLOOP; ++loop)
			{
				var tm:Object;
				if (t is LuaTable) // 't' is a table
				{
					var h:LuaTable = t as LuaTable;
					h.__getlua(key, SPARE_SLOT);
					if (SPARE_SLOT.r != NIL)   // result is not nil?
					{
						h.putluaSlot(this, key, val);
						return;
					}
					tm = tagmethod(h, "__newindex");
					if (tm == NIL)  // or no TM?
					{
						h.putluaSlot(this, key, val);
						return;
					}
					// else will try the tag method
				}
				else
				{
					tm = tagmethod(t, "__newindex");
					if (tm == NIL)
						gTypeerror(t, "index");
				}
				if (isFunction(tm))
				{
					callTM(tm, t, key, val);
					return;
				}
				t = tm;     // else repeat with 'tm'
			}
			gRunerror("loop in settable");
		}

		/**
		* Printf format item used to convert numbers to strings (in {@link
		* #vmTostring}).  The initial '%' should be not specified.
		*/
		private static const NUMBER_FMT:String = ".14g";

		private static function vmTostring(o:Object):String
		{
			if (o is String)
			{
				return o as String;
			}
			if (!(o is Number))
			{
				return null;
			}
			// Convert number to string.  PUC-Rio abstracts this operation into
			// a macro, lua_number2str.  The macro is only invoked from their
			// equivalent of this code.
			// Formerly this code used Double.toString (and remove any trailing
			// ".0") but this does not give an accurate emulation of the PUC-Rio
			// behaviour which Intuwave require.  So now we use "%.14g" like
			// PUC-Rio.
			// :todo: consider optimisation of making FormatItem an immutable
			// class and keeping a static reference to the required instance
			// (which never changes).  A possible half-way house would be to
			// create a copied instance from an already create prototype
			// instance which would be faster than parsing the format string
			// each time.
			var f:FormatItem = new FormatItem(null, NUMBER_FMT);
			var b:StringBuffer = new StringBuffer();
			var d:Number = o as Number;
			f.formatFloat(b, d as Number);
			return b.toString();
		}

		/** Equivalent of adjust_varargs in "ldo.c". */
		private function adjust_varargs(p:Proto, actual:int):int
		{
			var nfixargs:int = p.numparams;
			for (; actual < nfixargs; ++actual)
			{
				stackAdd(NIL);
			}
			// PUC-Rio's LUA_COMPAT_VARARG is not supported here.

			// Move fixed parameters to final position
			var fixed:int = this._stackSize - actual;  // first fixed argument
			var newbase:int = this._stackSize; // final position of first argument
			for (var i:int = 0; i < nfixargs; ++i)
			{
				// :todo: arraycopy?
				pushSlot(this._stack[fixed + i] as Slot);
				(this._stack[fixed + i] as Slot).r = NIL;
			}
			return newbase;
		}

		/**
		 * Does not modify contents of p1 or p2.  Modifies contents of res.
		 * @param p1  left hand operand.
		 * @param p2  right hand operand.
		 * @param res absolute stack index of result.
		 * @return false if no tagmethod, true otherwise
		 */
		private function call_binTM(p1:Slot, p2:Slot, res:Slot, event:String):Boolean
		{
			var tm:Object = tagmethod(p1.asObject(), event);        // try first operand
			if (isNil(tm))
			{
				tm = tagmethod(p2.asObject(), event);     // try second operand
			}
			if (!isFunction(tm))
			{
				return false;
			}
			callTMres(res, tm, p1, p2);
			return true;
		}

		/**
		* @return -1 if no tagmethod, 0 false, 1 true
		*/
		private function call_orderTM(p1:Slot, p2:Slot, event:String):int
		{
			var tm1:Object = tagmethod(p1.asObject(), event);
			if (tm1 == NIL)     // not metamethod
			{
				return -1;
			}
			var tm2:Object = tagmethod(p2.asObject(), event);
			if (!oRawequal(tm1, tm2))   // different metamethods?
			{
				return -1;
			}
			var s:Slot = new Slot();
			callTMres(s, tm1, p1, p2);
			return isFalse(s.r) ? 0 : 1;
		}

		private function callTM(f:Object, p1:Object, p2:Slot, p3:Object):void 
		{
			pushObject(f);
			pushObject(p1);
			pushSlot(p2);
			pushObject(p3);
			vmCall(this._stackSize - 4, 0);
		}

		private function callTMres(res:Slot, f:Object, p1:Slot, p2:Slot):void
		{
			pushObject(f);
			pushSlot(p1);
			pushSlot(p2);
			vmCall(this._stackSize - 3, 1);
			res.r = (this._stack[this._stackSize - 1] as Slot).r;
			res.d = (this._stack[this._stackSize - 1] as Slot).d;
			pop(1);
		}

		/**
		 * Overloaded version of callTMres used by {@link #vmEqualRef}.
		 * Textuall identical, but a different (overloaded) push method is
		 * invoked.
		 */
		private function __callTMres(res:Slot, f:Object, p1:Object, p2:Object):void
		{
			pushObject(f);
			pushObject(p1);
			pushObject(p2);
			vmCall(this._stackSize - 3, 1);
			res.r = (this._stack[this._stackSize - 1] as Slot).r;
			res.d = (this._stack[this._stackSize - 1] as Slot).d;
			pop(1);
		}

		private function get_compTM(mt1:LuaTable, mt2:LuaTable, event:String):Object
		{
			if (mt1 == null)
			{
				return NIL;
			}
			var tm1:Object = mt1.getlua(event);
			if (isNil(tm1))
			{
				return NIL;       // no metamethod
			}
			if (mt1 == mt2)
			{
				return tm1;       // same metatables => same metamethods
			}
			if (mt2 == null)
			{
				return NIL;
			}
			var tm2:Object = mt2.getlua(event);
			if (isNil(tm2))
			{
				return NIL;       // no metamethod
			}
			if (oRawequal(tm1, tm2))    // same metamethods?
			{
				return tm1;
			}
			return NIL;
		}

		/**
		* Gets tagmethod for object.
		* @return method or nil.
		*/
		private function tagmethod(o:Object, event:String):Object
		{
			return getMetafield(o, event);
		}

		/** @deprecated DO NOT CALL */
		private function __tagmethod(o:Slot, event:String):Object
		{
			throw new IllegalArgumentException("tagmethod called");
		}

		/**
		* Computes the result of Lua's modules operator (%).  Note that this
		* modulus operator does not match Java's %.
		*/
		private static function __modulus(x:Number, y:Number):Number
		{
			return x - Math.floor(x / y) * y;
		}

		/**
		* Changes the stack size, padding with NIL where necessary, and
		* allocate a new stack array if necessary.
		*/
		private function stacksetsize(n:int):void
		{
			if(n == 3)
			{
				if (Lua.D)
				{
					trace("stacksetsize:" + n);
				}
			}
			// It is absolutely critical that when the stack changes sizes those
			// elements that are common to both old and new stack are unchanged.

			// First implementation of this simply ensures that the stack array
			// has at least the required size number of elements.
			// :todo: consider policies where the stack may also shrink.
			var old:int = this._stackSize;
			if (n > this._stack.length)
			{
				//以2倍速度增加堆栈的深度
				var newLength:int = Math.max(n, 2 * this._stack.length);
				var newStack:Array = new Array(newLength); //Slot[] 
				// Currently the stack only ever grows, so the number of items to
				// copy is the length of the old stack.
				var toCopy:int = this._stack.length;
				SystemUtil.arraycopy(this._stack, 0, newStack, 0, toCopy);
				//trace(newStack[0]);
				this._stack = newStack;
			}
			this._stackSize = n;
			// Nilling out.  The VM requires that fresh stack slots allocated
			// for a new function activation are initialised to nil (which is
			// Lua.NIL, which is not Java null).
			// There are basically two approaches: nil out when the stack grows,
			// or nil out when it shrinks.  Nilling out when the stack grows is
			// slightly simpler, but nilling out when the stack shrinks means
			// that semantic garbage is not retained by the GC.
			// We nil out slots when the stack shrinks, but we also need to make
			// sure they are nil initially.
			// In order to avoid nilling the entire array when we allocate one
			// we maintain a stackhighwater which is 1 more than that largest
			// stack slot that has been nilled.  We use this to nil out stacks
			// slow when we grow.
			if (n <= old)
			{
				// when shrinking
				for (var i:int = n; i < old; ++i)
				{
					(this._stack[i] as Slot).r = NIL;
				}
			}
			if (n > this._stackhighwater)
			{
				// when growing above stackhighwater for the first time
				for (i = this._stackhighwater; i < n; ++i)
				{
					this._stack[i] = new Slot();
					(this._stack[i] as Slot).r = NIL;
				}
				this._stackhighwater = n;
			}
		}
			
		/**
		 * Pushes a Lua value onto the stack.
		 * 压入一个Lua值进堆栈
		 */
		private function stackAdd(o:Object):void
		{
			var i:int = this._stackSize;
			stacksetsize(i + 1);
			(this._stack[i] as Slot).setObject(o);
		}

		/**
		 * Copies a slot into a new space in the stack.
		 */
		private function pushSlot(p:Slot):void 
		{
			var i:int = this._stackSize;
			stacksetsize(i + 1);
			(this._stack[i] as Slot).r = p.r;
			(this._stack[i] as Slot).d = p.d;
		}

		private function stackInsertAt(o:Object, i:int):void 
		{
			var n:int = this._stackSize - i;
			stacksetsize(this._stackSize + 1);
			// Copy each slot N into its neighbour N+1.  Loop proceeds from high
			// index slots to lower index slots.
			// A loop from n to 1 copies n slots.
			for (var j:int = n; j >= 1; --j)
			{
				(this._stack[i + j] as Slot).r = (this._stack[i + j - 1] as Slot).r;
				(this._stack[i + j] as Slot).d = (this._stack[i + j - 1] as Slot).d;
			}
			(this._stack[i] as Slot).setObject(o);
		}

		/**
		* Equivalent of macro in ldebug.h.
		*/
		private function resethookcount():void
		{
			this._hookcount = this._basehookcount;
		}

		/**
		 * Equivalent of traceexec in lvm.c.
		 */
		private function traceexec(pc:int):void
		{
			var mask:int = this._hookmask;
			var oldpc:int = this._savedpc;
			this._savedpc = pc;
			if (mask > MASKLINE)        // instruction-hook set?
			{
				if (this._hookcount == 0)
				{
					resethookcount();
					dCallhook(HOOKCOUNT, -1);
				}
			}
			// :todo: line hook.
		}

		/**
		* Convert to number.  Returns true if the argument <var>o</var> was
		* converted to a number.  Converted number is placed in <var>out[0]</var>.
		* Returns
		* false if the argument <var>o</var> could not be converted to a number.
		* Overloaded.
		*/
		private static function tonumber(o:Slot, out:Array /*double[] */):Boolean
		{
			if (o.r == NUMBER)
			{
				out[0] = o.d;
				return true;
			}
			if (!(o.r is String))
			{
				return false;
			}
			if (oStr2d(o.r as String, out))
			{
				return true;
			}
			return false;
		}

		/**
		 * Converts a stack slot to number.  Returns true if the element at
		 * the specified stack slot was converted to a number.  False
		 * otherwise.  Note that this actually modifies the element stored at
		 * <var>idx</var> in the stack (in faithful emulation of the PUC-Rio
		 * code).  Corrupts <code>NUMOP[0]</code>.  Overloaded.
		 * @param idx  absolute stack slot.
		 */
		private function tonumber(idx:int):Boolean
		{
			if (Lua.tonumber(this._stack[idx] as Slot, NUMOP))
			{
				(this._stack[idx] as Slot).d = NUMOP[0] as Number;
				(this._stack[idx] as Slot).r = NUMBER;
				return true;
			}
			return false;
		}

		/**
		 * Convert a pair of operands for an arithmetic opcode.  Stores
		 * converted results in <code>out[0]</code> and <code>out[1]</code>.
		 * @return true if and only if both values converted to number.
		 */
		private static function toNumberPair(x:Slot, y:Slot, out:Array /*double[] */):Boolean
		{
			if (tonumber(y, out))
			{
				out[1] = out[0];
				if (tonumber(x, out))
				{
					return true;
				}
			}
			return false;
		}

		/**
		 * Convert to string.  Returns true if element was number or string
		 * (the number will have been converted to a string), false otherwise.
		 * Note this actually modifies the element stored at <var>idx</var> in
		 * the stack (in faithful emulation of the PUC-Rio code), and when it
		 * returns <code>true</code>, <code>stack[idx].r instanceof String</code>
		 * is true.
		 */
		private function tostring(idx:int):Boolean 
		{
			// :todo: optimise
			var o:Object = objectAt(idx);
			var s:String = vmTostring(o);
			if (s == null)
			{
				return false;
			}
			(this._stack[idx] as Slot).r = s;
			return true;
		}

		/**
		 * Equivalent to tryfuncTM from ldo.c.
		 * @param func  absolute stack index of the function object.
		 */
		private function tryfuncTM(func:int):Object
		{
			var tm:Object = tagmethod((this._stack[func] as Slot).asObject(), "__call");
			if (!isFunction(tm))
			{
				gTypeerror(this._stack[func] as Slot, "call");
			}
			stackInsertAt(tm, func);
			return tm;
		}

		/** Lua's is False predicate. */
		private function isFalse(o:Object):Boolean
		{
			return o == NIL || o == false;
		}

		/** @deprecated DO NOT CALL. */
		private function __isFalse(o:Slot):Boolean
		{
			throw new IllegalArgumentException("isFalse called");
		}

		/** Make new CallInfo record. */
		private function inc_ci(func:int, baseArg:int, top:int, nresults:int):CallInfo
		{
			var ci:CallInfo = new CallInfo();
			ci.init(func, baseArg, top, nresults);
			this._civ.addElement(ci);
			return ci;
		}

		/** Pop topmost CallInfo record and return it. */
		private function dec_ci():CallInfo
		{
			var ci:CallInfo = this._civ.pop() as CallInfo;
			return ci;
		}

		/** Equivalent to resume_error from ldo.c */
		private function resume_error(msg:String):int
		{
			stacksetsize(__ci().base);
			stackAdd(msg);
			return ERRRUN;
		}

		/**
		 * Return the stack element as an Object.  Converts double values into
		 * Double objects.
		 * @param idx  absolute index into stack (0 <= idx < stackSize).
		 */
		private function objectAt(idx:int):Object
		{
			var r:Object = (this._stack[idx] as Slot).r;
			if (r != NUMBER)
			{
				return r;
			}
			return new Number((this._stack[idx] as Slot).d);
		}

		/**
		 * Sets the stack element.  Double instances are converted to double.
		 * @param o  Object to store.
		 * @param idx  absolute index into stack (0 <= idx < stackSize).
		 */
		private function setObjectAt(o:Object, idx:int):void
		{
			if (o is Number)
			{
				(this._stack[idx] as Slot).r = NUMBER;
				(this._stack[idx] as Slot).d = o as Number;
				return;
			}
			if (D)
			{
				trace("setObjectAt(o, " + idx + ") from " + _stack);
			}
			(this._stack[idx] as Slot).r = o;
		}

		/**
		 * Corresponds to ldump's luaU_dump method, but with data gone and writer
		 * replaced by OutputStream.
		 */
		public static function uDump(f:Proto, writer:OutputStream, strip:Boolean):int
			//throws IOException
		{
			var d:DumpState = new DumpState(new DataOutputStream(writer), strip) ;
			d.DumpHeader();
			d.DumpFunction(f, null);
			d.writer.flush();
			return 0;   // Any errors result in thrown exceptions.
		}
		
		public function get global():LuaTable
		{
			return this._global;
		}
		
		public function set global(global:LuaTable):void
		{
			this._global = global;
		}
		
		//新增
		public function set nCcalls(nCcalls:int):void
		{
			this._nCcalls = nCcalls;
		}
		
		//新增
		public function get nCcalls():int
		{
			return this._nCcalls;
		}
		
	}
}