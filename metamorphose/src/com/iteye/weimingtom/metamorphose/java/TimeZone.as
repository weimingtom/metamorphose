package com.iteye.weimingtom.metamorphose.java
{
	public final class TimeZone
	{
		private var _id:String;
		private static var tz:TimeZone = new TimeZone();
		private static var tzGMT:TimeZone = new TimeZone();
		
		public function TimeZone()
		{
			
		}
		
		//Flash自动调整夏令时
		public function useDaylightTime():Boolean
		{
			return true;
		}
		
		//获取本地时间
		public static function getDefault():TimeZone
		{
			if(tz._id == null)
				tz._id = "default";
			return tz;
		}
		
		//获取GMT时间
		public static function getTimeZone(ID:String):TimeZone
		{
			if(ID != "GMT")
			{
				trace("TimeZone.getTimeZone(): not support name");
				throw new Error("TimeZone.getTimeZone(): not support name");
				return tz;
			}
			if(tzGMT._id == null)
				tzGMT._id = "GMT";
			return tzGMT;
		}
		
		//时区字符串
		public function getID():String
		{
			return this._id;
		}
	}
}