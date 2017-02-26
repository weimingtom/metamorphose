package com.iteye.weimingtom.metamorphose.java 
{
	public final class SystemUtil
	{
		public static var out:PrintStream = new PrintStream();
		
		public function SystemUtil() 
		{
			
		}	
		
		public static function arraycopy(src:Object, srcPos:int, 
			dest:Object, destPos:int, length:int):void	 
		{
			if(src != null && dest != null && src is Array && dest is Array)
			{
				for(var i:int = destPos; i < destPos + length; i++)
				{　
					(dest as Array)[i] = (src as Array)[i]; 
					//trace("arraycopy:", i, (src as Array)[i]); 
				}
			}
		}
		
		public static function gc():void
		{
			
		}
		
		public static function identityHashCode(obj:Object):int
		{
			return 0;
		}
		
		public static function getResourceAsStream(s:String):InputStream
		{
			return null;
		}
		
		public static function currentTimeMillis():Number
		{
			return 0;			
		}
	}
}