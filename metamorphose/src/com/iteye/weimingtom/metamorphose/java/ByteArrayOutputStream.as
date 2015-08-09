package com.iteye.weimingtom.metamorphose.java
{
	import flash.utils.ByteArray;

	public final class ByteArrayOutputStream extends OutputStream
	{
		private var _bytes:ByteArray = new ByteArray();
		
		public function ByteArrayOutputStream()
		{
			super();
		}
		
		public function toByteArray():ByteArray
		{
			return this._bytes;
		}
		
		override public function close():void
		{
			this._bytes.clear();
		}
		
		override public function flush():void
		{
			
		}
		
		override public function write(b:ByteArray):void
		{
			this._bytes.writeBytes(b);
		}
		
		override public function writeBytes(b:ByteArray, off:int, len:int):void
		{
			this._bytes.writeBytes(b, off, len);
		}
		
		//TODO: 这个方法有待修改
		//Writes a char to the underlying output stream as a 2-byte value, high byte first
		override public function writeChar(b:int):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeMultiByte(String.fromCharCode(b), "");
			this._bytes.writeBytes(bytes);
		}
	}
}
