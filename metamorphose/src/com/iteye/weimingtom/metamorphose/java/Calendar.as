package com.iteye.weimingtom.metamorphose.java
{
	public final class Calendar extends Object
	{
		public static const SECOND:int = 1;
		public static const MINUTE:int = 2;
		public static const HOUR:int = 3;
		public static const DAY_OF_MONTH:int = 4;
		public static const MONTH:int = 5;
		public static const YEAR:int = 6;
		public static const DAY_OF_WEEK:int = 7;

		public static const SUNDAY:int = 8;
		public static const MONDAY:int = 9;
		public static const TUESDAY:int = 10;
		public static const WEDNESDAY:int = 11;
		public static const THURSDAY:int = 12;
		public static const FRIDAY:int = 13;
		public static const SATURDAY:int = 14;
		
		public static const JANUARY:int = 15;
		public static const FEBRUARY:int = 16;
		public static const MARCH:int = 17;
		public static const APRIL:int = 18;
		public static const MAY:int = 19;
		public static const JUNE:int = 20;
		public static const JULY:int = 21;
		public static const AUGUST:int = 22;
		public static const SEPTEMBER:int = 23;
		public static const OCTOBER:int = 24;
		public static const NOVEMBER:int = 25;
		public static const DECEMBER:int = 26;
		
		private static var _instance:Calendar = new Calendar();
		private var _date:Date;
		
		public function Calendar()
		{
		}
		
		public function _get(field:int):int
		{
			switch(field)
			{
				case SECOND:
					return this._date.seconds;
				
				case MINUTE:
					return this._date.minutes;
				
				case HOUR:
					return this._date.hours;
					
				case MONTH:
					return this._date.month;
					
				case YEAR:
					return this._date.fullYear;
					
				case DAY_OF_WEEK:
					trace("DAY_OF_WEEK not implement");
					return 0;
					
				case DAY_OF_MONTH:
					return this._date.day;
			}
			
			trace("Calendar._get(): field not implement");
			return 0;
		}
		
		public function _set(field:int, value:int):void
		{
			switch(field)
			{
				case SECOND:
					this._date.seconds = value;
					return;
					
				case MINUTE:
					this._date.minutes = value;
					return;
					
				case HOUR:
					this._date.hours = value;
					return;
					
				case MONTH:
					this._date.month = value;
					return;
					
				case YEAR:
					this._date.fullYear = value;
					return;
			}
			
			trace("Calendar._set(): field not implement");
		}
		
		public static function getInstance(t:TimeZone = null):Calendar
		{
			return Calendar._instance;
		}
		
		public function setTime(d:Date):void
		{
			this._date = d;
		}
		
		public function getTime():Date
		{
			return this._date;
		}
	}
}