/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/PackageLib.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.InputStream;
	import com.iteye.weimingtom.metamorphose.java.StringBuffer;
	import com.iteye.weimingtom.metamorphose.java.IOException;
	import com.iteye.weimingtom.metamorphose.java.SystemUtil;
	
	/**
	 * Contains Lua's package library.
	 * The library
	 * can be opened using the {@link #open} method.
	 */
	public final class PackageLib extends LuaJavaCallback
	{
		// Each function in the library corresponds to an instance of
		// this class which is associated (the 'which' member) with an integer
		// which is unique within this class.  They are taken from the following
		// set.
		private static const MODULE:int = 1;
		private static const REQUIRE:int = 2;
		private static const SEEALL:int = 3;
		private static const LOADER_PRELOAD:int = 4;
		private static const LOADER_LUA:int = 5;
		
		
		/**
		 * Which library function this object represents.  This value should
		 * be one of the "enums" defined in the class.
		 */
		private var _which:int;

		/**
		 * Module Environment; a reference to the package table so that
		 * package functions can access it without using the global table.
		 * In PUC-Rio this reference is stored in the function's environment.
		 * Not all instances (Lua Java functions) require this member, but
		 * another subclass would be too wasteful.
		 */
		private var _me:LuaTable;

		public function PackageLib(which:int, me:LuaTable = null) 
		{
			this._which = which;
			this._me = me;
		}
		
		//private function __init(which:int, me:LuaTable):void
		//{
		//	this._which = which;
		//	this.me = me;
		//}
		
		
		/**
		* Implements all of the functions in the Lua package library.  Do not
		* call directly.
		* @param L  the Lua state in which to execute.
		* @return number of returned parameters, as per convention.
		*/
		override public function luaFunction(L:Lua):int
		{
			switch (this._which)
			{
				case MODULE:
					return module(L);
		  
				case REQUIRE:
					return require(L);
		  
				case SEEALL:
					return seeall(L);
		  
				case LOADER_LUA:
					return loaderLua(L);
		  
				case LOADER_PRELOAD:
					return loaderPreload(L);
			}
			return 0;
		}

		/**
		 * Opens the library into the given Lua state.  This registers
		 * the symbols of the library in the global table.
		 * @param L  The Lua state into which to open.
		 */
		public static function open(L:Lua):void
		{
			var t:LuaTable = L.__register("package");

			g(L, t, "module", MODULE);
			g(L, t, "require", REQUIRE);

			r(L, "seeall", SEEALL);

			L.setField(t, "loaders", L.newTable());
			p(L, t, LOADER_PRELOAD);
			p(L, t, LOADER_LUA);
			setpath(L, t, "path", PATH_DEFAULT);        // set field 'path'

			// set field 'loaded'
			L.findTable(L.getRegistry(), Lua.LOADED, 1);
			L.setField(t, "loaded", L.value(-1));
			L.pop(1);
			L.setField(t, "preload", L.newTable());
		}

		/** Register a function. */
		private static function r(L:Lua, name:String, which:int):void
		{
			var f:PackageLib = new PackageLib(which);
			L.setField(L.getGlobal("package"), name, f);
		}

		/** Register a function in the global table. */
		private static function g(L:Lua, t:LuaTable, name:String, which:int):void
		{
			var f:PackageLib = new PackageLib(which, t);
			L.setGlobal(name, f);
		}
		
		/** Register a loader in package.loaders. */
		private static function p(L:Lua, t:LuaTable, which:int):void
		{
			var f:PackageLib = new PackageLib(which, t);
			var loaders:Object = L.getField(t, "loaders");
			L.rawSetI(loaders, Lua.objLen(loaders)+1, f);
		}

		private static const DIRSEP:String = "/";
		private static const PATHSEP:String  = ';'; //TODO:
		private static const PATH_MARK:String = "?";
		private static const PATH_DEFAULT:String = "?.lua;?/init.lua";

		private static const SENTINEL:Object = new Object();

		/**
		* Implements the preload loader.  This is conventionally stored
		* first in the package.loaders table.
		*/
		private function loaderPreload(L:Lua):int
		{
			var name:String = L.checkString(1);
			var preload:Object = L.getField(this._me, "preload");
			if (!Lua.isTable(preload))
				L.error("'package.preload' must be a table");
			var loader:Object = L.getField(preload, name);
			if (Lua.isNil(loader))        // not found?
				L.pushString("\n\tno field package.preload['" + name + "']");
			L.pushObject(loader);
			return 1;
		}

		/**
		 * Implements the lua loader.  This is conventionally stored second in
		 * the package.loaders table.
		 */
		private function loaderLua(L:Lua):int
		{
			var name:String = L.checkString(1);
			var filename:String = findfile(L, name, "path");
			if (filename == null)
				return 1; // library not found in this path
			if (L.loadFile(filename) != 0)
				loaderror(L, filename);
			return 1;   // library loaded successfully
		}

		/** Implements module. */
		private function module(L:Lua):int
		{
			var modname:String = L.checkString(1);
			var loaded:Object = L.getField(this._me, "loaded");
			var module:Object = L.getField(loaded, modname);
			if (!Lua.isTable(module))     // not found?
			{
				// try global variable (and create one if it does not exist)
				if (L.findTable(L.getGlobals(), modname, 1) != null)
					return L.error("name conflict for module '" + modname + "'");
				module = L.value(-1);
				L.pop(1);
				// package.loaded = new table
				L.setField(loaded, modname, module);
			}
			// check whether table already has a _NAME field
			if (Lua.isNil(L.getField(module, "_NAME")))
			{
				modinit(L, module, modname);
			}
			setfenv(L, module);
			dooptions(L, module, L.getTop());
			return 0;
		}

		/** Implements require. */
		private function require(L:Lua):int
		{
			var name:String = L.checkString(1);
			L.setTop(1);
			// PUC-Rio's use of lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
			// (package.loaded is kept in the registry in PUC-Rio) is translated
			// into this:
			var loaded:Object = L.getField(this._me, "loaded");
			var module:Object = L.getField(loaded, name);
			if (L.toBoolean(module))    // is it there?
			{
				if (module == SENTINEL)   // check loops
					L.error("loop or previous error loading module '" + name + "'");
				L.pushObject(module);
				return 1;
			}
			// else must load it; iterate over available loaders.
			var loaders:Object = L.getField(this._me, "loaders");
			if (!Lua.isTable(loaders))
				L.error("'package.loaders' must be a table");
			L.pushString("");   // error message accumulator
			for (var i:int = 1; ; ++i)
			{
				var loader:Object = Lua.rawGetI(loaders, i);    // get a loader
				if (Lua.isNil(loader))
					L.error("module '" + name + "' not found:" +
				L.toString(L.value(-1)));
				L.pushObject(loader);
				L.pushString(name);
				L.call(1, 1);     // call it
				if (Lua.isFunction(L.value(-1)))    // did it find module?
					break;  // module loaded successfully
				else if (Lua.isString(L.value(-1))) // loader returned error message?
					L.concat(2);    // accumulate it
				else
					L.pop(1);
			}
			L.setField(loaded, name, SENTINEL); // package.loaded[name] = sentinel
			L.pushString(name); // pass name as argument to module
			L.call(1, 1);       // run loaded module
			if (!Lua.isNil(L.value(-1)))  // non-nil return?
			{
				// package.loaded[name] = returned value
				L.setField(loaded, name, L.value(-1));
			}
			module = L.getField(loaded, name);
			if (module == SENTINEL)  // module did not set a value?
			{
				module = Lua.valueOfBoolean(true);  // use true as result
				L.setField(loaded, name, module); // package.loaded[name] = true
			}
			L.pushObject(module);
			return 1;
		}
		
		/** Implements package.seeall. */
		private static function seeall(L:Lua):int
		{
			L.checkType(1, Lua.TTABLE);
			var mt:LuaTable = L.getMetatable(L.value(1));
			if (mt == null)
			{
				mt = L.createTable(0, 1);
				L.setMetatable(L.value(1), mt);
			}
			L.setField(mt, "__index", L.getGlobals());
			return 0;
		}

		/**
		* Helper for module.  <var>module</var> parameter replaces PUC-Rio
		* use of passing it on the stack.
		*/
		public static function setfenv(L:Lua, module:Object):void
		{
			var ar:Debug = L.getStack(1);
			L.getInfo("f", ar);
			L.setFenv(L.value(-1), module);
			L.pop(1);
		}

		/**
		 * Helper for module.  <var>module</var> parameter replaces PUC-Rio
		 * use of passing it on the stack.
		 */
		private static function dooptions(L:Lua, module:Object, n:int):void
		{
			for (var i:int = 2; i <= n; ++i)
			{
				L.pushValue(i);   // get option (a function)
				L.pushObject(module);
				L.call(1, 0);
			}
		}

		/**
		* Helper for module.  <var>module</var> parameter replaces PUC-Rio
		* use of passing it on the stack.
		*/
		private static function modinit(L:Lua, module:Object, modname:String):void
		{
			L.setField(module, "_M", module);   // module._M = module
			L.setField(module, "_NAME", modname);
			var dot:int = modname.lastIndexOf('.'); // look for last dot in module name
			// Surprisingly, ++dot works when '.' was found and when it wasn't.
			++dot;
			// set _PACKAGE as package name (full module name minus last part)
			L.setField(module, "_PACKAGE", modname.substring(0, dot));
		}

		private static function loaderror(L:Lua, filename:String):void
		{
			L.error("error loading module '" + L.toString(L.value(1)) +
				"' from file '" + filename + "':\n\t" +
				L.toString(L.value(-1)));
		}
	
		private static function readable(filename:String):Boolean
		{
			var f:InputStream = SystemUtil.getResourceAsStream(filename);
			if (f == null)
				return false;
			try
			{
				f.close();
			}
			catch (e_:IOException)
			{
				trace(e_.getStackTrace());
			}
			return true;
		}

		private static function pushnexttemplate(L:Lua, path:String):String
		{
			var i:int = 0;
			// skip seperators
			while (i < path.length && path.substr(i, 1) == PATHSEP) //TODO:
				++i;
			if (i == path.length)
				return null;      // no more templates
			var l:int = path.indexOf(PATHSEP, i);
			if (l < 0)
				l = path.length;
			L.pushString(path.substring(i, l)); // template
			return path.substring(l);
		}

		private function findfile(L:Lua, name:String, pname:String):String
		{
			name = gsub(name, ".", DIRSEP);
			var path:String = L.toString(L.getField(this._me, pname));
			if (path == null)
				L.error("'package." + pname + "' must be a string");
			L.pushString("");   // error accumulator
			while (true)
			{
				path = pushnexttemplate(L, path);
				if (path == null)
					break;
				var filename:String = gsub(L.toString(L.value(-1)), PATH_MARK, name);
				if (readable(filename))   // does file exist and is readable?
					return filename;        // return that file name
				L.pop(1); // remove path template
				L.pushString("\n\tno file '" + filename + "'");
				L.concat(2);
			}
			return null;        // not found
		}

		/** Almost equivalent to luaL_gsub. */
		private static function gsub(s:String, p:String, r:String):String
		{
			var b:StringBuffer = new StringBuffer();
			// instead of incrementing the char *s, we use the index i
			var i:int = 0;
			var l:int = p.length;
			
			while (true)
			{
				var wild:int = s.indexOf(p, i);
				if (wild < 0)
					break;
				b.appendString(s.substring(i, wild));   // add prefix
				b.appendString(r);      // add replacement in place of pattern
				i = wild + l;     // continue after 'p'
			}
			b.appendString(s.substring(i));
			return b.toString();
		}
		
		private static function setpath(L:Lua,
			t:LuaTable,
			fieldname:String,
			def:String):void
		{
			// :todo: consider implementing a user-specified path via
			// javax.microedition.midlet.MIDlet.getAppProperty or similar.
			// Currently we just use a default path defined by Jill.
			L.setField(t, fieldname, def);
		}
	}
}