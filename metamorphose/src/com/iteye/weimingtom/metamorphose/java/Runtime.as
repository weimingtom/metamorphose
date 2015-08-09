package com.iteye.weimingtom.metamorphose.java 
{
	import flash.system.System;

	public final class Runtime
	{
		private static var _instance:Runtime = new Runtime();
		
		public function Runtime() 
		{
			
		}
		
		public static function getRuntime():Runtime
		{
			return Runtime._instance;
		}
		
		public function totalMemory():uint
		{
			return flash.system.System.totalMemory;
		}
		
		public function freeMemory():uint
		{
			trace("Runtime.freeMemory() not implement");
			return 0;
		}	
	}
}