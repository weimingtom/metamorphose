package com.iteye.weimingtom.metamorphose.java 
{
	import flash.utils.ByteArray;
	
	/**
	 * 
	 * 此抽象类是表示字节输入流的所有类的超类。
	 * 需要定义 InputStream 的子类的应用程序
	 * 必须始终提供返回下一个输入字节的方法。
	 * 
	 */
	public class InputStream
	{
		public function InputStream() 
		{
			
		}	
		
		public function readBytes(bytes:ByteArray):int
		{
			throwError("InputStream.readBytes() not implement");	
			return 0;
		}
		
		//从输入流读取下一个数据字节。
		public function read():int
		{
			throwError("InputStream.readChar() not implement");	
			return 0;
		}
		
		public function reset():void
		{
			throwError("InputStream.reset() not implement");				
		}
		
		public function mark(i:int):void
		{
			throwError("InputStream.mark() not implement");			
		}
		
		public function markSupported():Boolean
		{
			throwError("InputStream.markSupported() not implement");	
			return false;
		}	
		
		public function close():void
		{
			throwError("InputStream.close() not implement");			
		}
		
		public function available():int
		{
			throwError("InputStream.available() not implement");
			return 0;
		}
		
		public function skip(n:int):int
		{
			throwError("InputStream.skip() not implement");
			return 0;
		}
		
		public function readMultiBytes(bytes:ByteArray,  off:int, len:int):int
		{
			throwError("InputStream.readBytes() not implement");	
			return 0;
		}
		
		public function throwError(str:String):void
		{
			trace(str);
			throw new Error(str);
		}
	}
}