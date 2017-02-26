package com.iteye.weimingtom.metamorphose.java 
{
	/**
	 *  InputStreamReader 是字节流通向字符流的桥梁：
	 * 	它使用指定的 charset 读取字节并将其解码为字符。
	 * 	它使用的字符集可以由名称指定或显式给定，
	 * 	否则可能接受平台默认的字符集。
	 * 	每次调用 InputStreamReader 中的一个 read() 方法都会导致从基础输入流读取一个或多个字节。
	 * 	要启用从字节到字符的有效转换，可以提前从基础流读取更多的字节，
	 * 	使其超过满足当前读取操作所需的字节。
	 * 	为了达到最高效率，可要考虑在 BufferedReader 内包装 InputStreamReader。
	 */
	public class InputStreamReader extends Reader
	{
		private var _i:InputStream;
		private var _charsetName:String;
		//见LuaInternal，创建一个带字符集（UTF8）的读出器
		//i可能是DumpedInput
		//charsetName可能是"UTF8"
		public function InputStreamReader(i:InputStream, charsetName:String) 
		{
			this._i = i;
			this._charsetName = charsetName;
		}
		
		override public function close():void
		{
			this._i.close();		
		}
		
		override public function mark(readAheadLimit:int):void
		{
			this._i.mark(readAheadLimit);		
		}
		
		override public function markSupported():Boolean
		{
			return this._i.markSupported();
		}
		
		override public function read():int
		{
			return this._i.read();
		}
		
		override public function readBytes(cbuf:ByteArray/*char[]*/):int
		{
			return this._i.readBytes(cbuf);
		}
		
		//本工程未使用
		override public function readMultiBytes(cbuf:ByteArray/*char[] */, off:int, len:int):int
		{
			return this._i.readMultiBytes(cbuf, off, len);
		}
		
		//TODO:?
		override public function ready():Boolean
		{
			return true;
		}
		
		override public function reset():void
		{
			this._i.reset();		
		}
		
		//本工程未使用
		override public function skip(n:int):int
		{
			return this._i.skip(n);
		}
	}
}
