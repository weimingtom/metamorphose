/*  $Header: //info.ravenbrook.com/project/jili/version/1.1/code/mnj/lua/BlockCnt.java#1 $
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
	public final class Slot
	{
		private var _r:Object;
		private var _d:Number;
		
		public function Slot()
		{
		  
		}
		
		public function init1(s:Slot):void
		{
			this._r = s._r;
			this._d = s._d;
		}
		
		//TODO:
		public function init2(o:Object):void
		{
			this.setObject(o);
		}
		
		public function asObject():Object
		{
			if (this._r == Lua.NUMBER)
			{
				return new Number(this._d);
			}
			return this._r;
		}
		
		public function setObject(o:Object):void
		{
			//trace("setObject:", o.toString());
			this._r = o;
			if (o is Number)
			{
				this._r = Lua.NUMBER;
				this._d = o as Number;
			}
		}
		
		//新增
		public function set r(r:Object):void
		{
			this._r = r;
		}
		
		//新增
		public function get r():Object
		{
			return this._r;
		}
		
		//新增
		public function set d(d:Number):void
		{
			this._d = d;
		}
		
		//新增
		public function get d():Number
		{
			return this._d;
		}
	}
}