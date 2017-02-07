/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/BaseLibReader.java#1 $
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
	import com.iteye.weimingtom.metamorphose.java.IOException;
	import com.iteye.weimingtom.metamorphose.java.Reader;
	
	import flash.utils.ByteArray;
	
	/**
	 * Extends {@link java.io.Reader} to create a Reader from a Lua
	 * function.  So that the <code>load</code> function from Lua's base
	 * library can be implemented.
	 */
	public final class BaseLibReader extends Reader
	{
		private var _s:String = "";
		private var _i:int;        // = 0;
		private var _mark:int = -1;
		private var _L:Lua;
		private var _f:Object;
  		
		public function BaseLibReader(L:Lua, f:Object) 
		{
			this._L = L;
			this._f = f;
		}
		
		
		override public function close():void
		{
			this._f = null;
		}

		override public function mark(l:int):void
		{
			if (l > 1)
			{
				throw new IOException("Readahead must be <= 1");
			}
			this._mark = this._i;
		}

		override public function markSupported():Boolean
		{
			return true;
		}

		override public function read():int
		{
			if (this._i >= this._s.length)
			{
				this._L.pushObject(this._f);
				this._L.call(0, 1);
				if (Lua.isNil(this._L.value(-1)))
				{
					return -1;
				}
				else if(Lua.isString(this._L.value(-1)))
				{
					this._s = this._L.toString(this._L.value(-1));
					if (this._s.length == 0)
					{
						return -1;
					}
					if (this._mark == this._i)
					{
						this._mark = 0;
					}
					else
					{
						this._mark = -1;
					}
					this._i = 0;
				}
				else
				{
					this._L.error("reader function must return a string");
				}
			}
			return this._s.charCodeAt(this._i++);
		}

		override public function readMultiBytes(cbuf:ByteArray, off:int, len:int):int 
		{
			var j:int = 0;  // loop index required after loop
			for (j = 0; j < len; ++j)
			{
				var c:int = read();
				if (c == -1)
				{
					if (j == 0)
					{
						return -1;
					}
					else
					{
						return j;
					}
				}
				cbuf[off+j] = c as uint;
			}
			return j;
		}

		override public function reset():void
		{
			if (this._mark < 0)
			{
				throw new IOException("reset() not supported now");
			}
			this._i = this._mark;
		}
	}

}