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
	import com.iteye.weimingtom.metamorphose.java.DataOutputStream;
	
	import flash.utils.ByteArray;
	
	public final class DumpState
	{
		private var _writer:DataOutputStream;
		private var _strip:Boolean;

		public function DumpState(writer:DataOutputStream, strip:Boolean)
		{
			this._writer = writer;
			this._strip = strip;
		}
		
		//////////////// dumper ////////////////////

		public function DumpHeader():void  // throws IOException
		{
			/*
			 * In order to make the code more compact the dumper re-uses the
			 * header defined in Loader.java.  It has to fix the endianness byte
			 * first.
			 */
			Loader.HEADER[6] = 0;
			//TODO:Java to AS3
			var b:ByteArray = new ByteArray();
			var len:int = Loader.HEADER.length;
			for (var i:int = 0; i < len; ++i)
			{
				b.writeByte(Loader.HEADER[i]);
			}
			this._writer.write(b);
		}
		
		private function DumpInt(i:int):void  // throws IOException
		{
			this._writer.writeInt(i);        // big-endian
		}
		
		private function DumpNumber(d:Number):void //throws IOException
		{
			this._writer.writeDouble(d);     // big-endian
		}
		
		public function DumpFunction(f:Proto, p:String):void  // throws IOException
		{
			DumpString((f.source == p || this._strip) ? null : f.source);
			DumpInt(f.linedefined);
			DumpInt(f.lastlinedefined);
			this._writer.writeByte(f.nups);
			this._writer.writeByte(f.numparams);
			this._writer.writeBoolean(f.isVararg);
			this._writer.writeByte(f.maxstacksize);
			DumpCode(f);
			DumpConstants(f);
			DumpDebug(f);
		}
		
		private function DumpCode(f:Proto):void // throws IOException
		{
			var n:int = f.sizecode;
			var code:Array = f.code; //int [] 
			DumpInt(n);
			for (var i:int = 0; i < n ; i++)
				DumpInt(code[i]);
		}
		
		private function DumpConstants(f:Proto):void // throws IOException
		{
			var n:int = f.sizek;
			var k:Array = f.k; //Slot[]
			DumpInt(n);
			for (var i:int = 0 ; i < n ; i++)
			{
				var o:Object = (k[i] as Slot).r;
				if (o == Lua.NIL)
				{
					this._writer.writeByte(Lua.TNIL);
				}
				else if (o is Boolean)
				{
					this._writer.writeByte(Lua.TBOOLEAN);
					this._writer.writeBoolean(o as Boolean);
				}
				else if (o == Lua.NUMBER)
				{
					this._writer.writeByte(Lua.TNUMBER);
					DumpNumber((k[i] as Slot).d);
				}
				else if (o is String)
				{
					this._writer.writeByte(Lua.TSTRING);
					DumpString(o as String);
				}
				else
				{
					//# assert false
				}
			}
			n = f.sizep ;
			DumpInt(n) ;
			for (i = 0 ; i < n ; i++)
			{
				var subfunc:Proto = f.p[i];
				DumpFunction(subfunc, f.source);
			}
		}
		
		private function DumpString(s:String):void // throws IOException
		{
			if (s == null)
			{
				DumpInt(0);
			}
			else
			{
				/*
				 * Strings are dumped by converting to UTF-8 encoding.  The MIDP
				 * 2.0 spec guarantees that this encoding will be supported (see
				 * page 9 of midp-2_0-fr-spec.pdf).  Nonetheless, any
				 * possible UnsupportedEncodingException is left to be thrown
				 * (it's a subclass of IOException which is declared to be thrown).
				 */
				//TODO: Java to AS3
				var contents:ByteArray = new ByteArray();// s.getBytes("UTF-8"); //byte []
				contents.writeUTFBytes(s);
				var size:int = contents.length;
				DumpInt(size+1) ;
				this._writer.write(contents, 0, size);
				this._writer.writeByte(0);
			}
		}
		
		private function DumpDebug(f:Proto):void // throws IOException
		{
			if (this._strip)
			{
				DumpInt(0) ;
				DumpInt(0) ;
				DumpInt(0) ;
				return ;
			}
		
			var n:int = f.sizelineinfo;
			DumpInt(n);
			for (var i:int = 0; i < n; i++)
				DumpInt(f.lineinfo[i]);
			
			n = f.sizelocvars;
			DumpInt(n);
			for (i = 0; i < n; i++)
			{
				var locvar:LocVar = f.locvars[i];
				DumpString(locvar.varname);
				DumpInt(locvar.startpc);
				DumpInt(locvar.endpc);
			}
			
			n = f.sizeupvalues;
			DumpInt(n);
			for (i = 0; i < n; i++)
				DumpString(f.upvalues[i]);
		}
		
		//新增
		public function get writer():DataOutputStream
		{
			return this._writer;
		}
	}
}