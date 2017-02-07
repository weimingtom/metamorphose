/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/Debug.java#1 $
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

	/**
	 * Equivalent to struct lua_Debug.  This implementation is incomplete
	 * because it is not intended to form part of the public API.  It has
	 * only been implemented to the extent necessary for internal use.
	 */
	public final class Debug
	{	
		// private, no public accessors defined.
		private var _ici:int;

		// public accessors may be defined for these.
		private var _event:int;
		private var _what:String;
		private var _source:String;
		private var _currentline:int;
		private var _linedefined:int;
		private var _lastlinedefined:int;
		private var _shortsrc:String;
		
		/**
		 * @param ici  index of CallInfo record in L.civ
		 */
		public function Debug(ici:int)
		{
			this._ici = ici;
		}
		
		public function set ici(ici:int):void
		{
			this._ici = ici;
		}

		/**
		 * Get ici, index of the {@link CallInfo} record.
		 */
		public function get ici():int
		{
			return _ici;
		}

		/**
		 * Setter for event.
		 */
		public function set event(event:int):void
		{
			this._event = event;
		}

		/**
		 * Sets the what field.
		 */
		public function set what(what:String):void
		{
			this._what = what;
		}
			
		/**
		 * Sets the source, and the shortsrc.
		 */
		public function set source(source:String):void
		{
			this._source = source;
			this._shortsrc = Lua.oChunkid(source);
		}

		/**
		 * Gets the current line.  May become public.
		 */
		public function get currentline():int
		{
			return _currentline;
		}

		/**
		 * Set currentline.
		 */
		public function set currentline(currentline:int):void
		{
			this._currentline = currentline;
		}
		
		/**
		 * Get linedefined.
		 */
		public function get linedefined():int
		{
			return this._linedefined;
		}

		/**
		 * Set linedefined.
		 */
		public function set linedefined(linedefined:int):void
		{
			this._linedefined = linedefined;
		}

		/**
		 * Set lastlinedefined.
		 */
		public function set lastlinedefined(lastlinedefined:int):void
		{
			this._lastlinedefined = lastlinedefined;
		}

		/**
		 * Gets the "printable" version of source, for error messages.
		 * May become public.
		 */
		public function get shortsrc():String
		{
			return _shortsrc;
		}  
	}
}