package com.iteye.weimingtom.metamorphose.java 
{
	import flash.utils.ByteArray;
	
	/**
	 * 数据输出流允许应用程序以适当方式将基本 Java 数据类型写入输出流中。
	 * 然后，应用程序可以使用数据输入流将数据读入。
	 * 
	 * 封装构造函数中的OutputStream，而这个类的特点是统计了写入字节数。
	 * 实现这个类，基本上只用writeByte处理
	 */
	public class DataOutputStream extends Object
	{
		protected var written:int = 0;
		
		private var _writer:OutputStream;
		
		/**
		 * 实际传入的是 ByteArrayOutputStream，见StringLib
		 */
		public function DataOutputStream(writer:OutputStream) 
		{
			this._writer = writer;
		}
		
		public function flush():void
		{
			this._writer.flush();
		}
		
		public function size():int
		{
			return this.written;
		}
		
		public function write(b:ByteArray, off:int=0, len:int=0):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeBytes(b, off, len);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		//public function write(b:int):void
		//{
		//	
		//}
		
		public function writeBoolean(v:Boolean):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeBoolean(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeByte(v:int):void
		{
			//???
			//this._writer.writeChar(v);
			var bytes:ByteArray = new ByteArray();
			bytes.writeByte(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeBytes(s:String):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeMultiByte(s, "");
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		//TODO: 这个方法有待修改
		public function writeChar(v:int):void 
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeMultiByte(String.fromCharCode(v), "");
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		//TODO: 这个方法有待修改
		public function writeChars(s:String):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeMultiByte(s, "");
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeDouble(v:Number):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeDouble(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeFloat(v:Number):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeFloat(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeInt(v:int):void 
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeInt(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		//这里可能有问题
		public function writeLong(v:int):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeInt(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeShort(v:int):void 
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeShort(v);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
		
		public function writeUTF(str:String):void
		{
			var bytes:ByteArray = new ByteArray();
			bytes.writeUTFBytes(str);
			this._writer.write(bytes);
			this.written += bytes.length;
		}
	}
}
