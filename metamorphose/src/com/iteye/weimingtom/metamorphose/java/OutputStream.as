package com.iteye.weimingtom.metamorphose.java 
{
	import flash.utils.ByteArray;

	/**
	 * 此抽象类是表示输出字节流的所有类的超类。
	 * 输出流接受输出字节并将这些字节发送到某个接收器。
	 * 需要定义 OutputStream 子类的应用程序必须始终提供
     * 至少一种可写入一个输出字节的方法。
	 * 
	 * 这个类不应该实例化
	 * 略加修改，让所有写方法都可以返回写入字节数
     */ 
	public class OutputStream
	{		
		public function OutputStream() 
		{
			
		}
		
		public function close():void
		{
			throwError("OutputStream.close() not implement");
		}
		
		public function flush():void
		{
			throwError("OutputStream.flush() not implement");			
		}
		
		public function write(b:ByteArray):void
		{
			throwError("OutputStream.write() not implement");
		}
		
		public function writeBytes(b:ByteArray, off:int, len:int):void
		{
			throwError("OutputStream.writeBytes() not implement");
		}
		
		public function writeChar(b:int):void
		{
			throwError("OutputStream.writeChar() not implement");				
		}
		
		private function throwError(str:String):void
		{
			trace(str);
			throw Error(str);
		}
	}
}