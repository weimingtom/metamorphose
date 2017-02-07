package com.iteye.weimingtom.metamorphose.java
{
	public final class PrintStream
	{
		public static var OutputArr:Array;
		
		public static function init():void
		{
			OutputArr = new Array();
			OutputArr.push("");
		}
		
		public function PrintStream()
		{
			init();
		}
		
		//TODO:
		public function print(str:String):void
		{
			OutputArr[OutputArr.length - 1] += str;
			trace(str);
		}
		
		//TODO:
		public function println():void
		{
			OutputArr.push("");
			trace("\n");
		}
	}
}