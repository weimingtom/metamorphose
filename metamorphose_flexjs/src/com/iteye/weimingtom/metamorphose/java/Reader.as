package com.iteye.weimingtom.metamorphose.java 
{
	/**
	 *	用于读取字符流的抽象类。
	 *	子类必须实现的方法只有 read(char[], int, int) 和 close()。
	 *	但是，多数子类将重写此处定义的一些方法，
	 *	以提供更高的效率和/或其他功能。
	 */
	public class Reader extends Object
	{
		public function Reader() 
		{
			
		}	
		
		public function close():void
		{
			throwError("Reader.close() not implement");				
		}
		
		public function mark(readAheadLimit:int):void
		{
			throwError("Reader.mark() not implement");			
		}
		
		public function markSupported():Boolean
		{
			throwError("Reader.markSupported() not implement");
			return false;
		}
		
		public function read():int
		{
			throwError("Reader.read() not implement");
			return 0;
		}
		
		public function readBytes(cbuf:ByteArray/*char[]*/):int
		{
			throwError("Reader.readBytes() not implement");
			return 0;
		}
		
		public function readMultiBytes(cbuf:ByteArray/*char[] */, off:int, len:int):int
		{
			throwError("Reader.readMultiBytes() not implement");
			return 0;
		}
		
		public function ready():Boolean
		{
			throwError("Reader.ready() not implement");
			return false;
		}
		
		public function reset():void
		{
			throwError("Reader.reset() not implement");			
		}
		
		public function skip(n:int):int
		{
			throwError("Reader.skip() not implement");
			return 0;
		}
		
		//新增
		private function throwError(str:String):void
		{
			trace(str);
			throw new Error(str);
		}
	}
}