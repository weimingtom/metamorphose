package com.iteye.weimingtom.metamorphose.java
{
	public final class MathUtil
	{
		//see http://codesnipp.it/code/939
		public function MathUtil()
		{
			
		}
		
		//弧度转换为角度
		// convert radians to degrees  		
		public static function toDegrees(rad:Number):Number 
		{
			return (rad / 180 * Math.PI);
		}  
		
		// convert degrees to radians  
		//角度转换为弧度
		public static function toRadians(deg:Number):Number 
		{  
			return (deg * Math.PI / 180);  
		}  
	}
}