package com.iteye.weimingtom.metamorphose.launcher
{
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.net.URLRequest;
	import flash.events.ProgressEvent;
	import flash.display.Loader;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFieldType;

	/**
	 * ...
	 * @author 
	 */
	[SWF(width=640, height = 520)]
	public class Preloader extends Sprite 
	{
		private var loader:Loader;
		private var _tfOutput:TextField = new TextField();
		
		public function Preloader() 
		{
			if (stage) init();
			else addEventListener(Event.ADDED_TO_STAGE, init);
		}
		
		private function init(e:Event = null):void 
		{
			removeEventListener(Event.ADDED_TO_STAGE, init);
			// entry point
			
			_tfOutput.x = 250;
			_tfOutput.y = 240;
			_tfOutput.restrict = null;
			_tfOutput.multiline = false;
			_tfOutput.wordWrap = false;
			_tfOutput.border = false;
			_tfOutput.background = true;
			_tfOutput.backgroundColor = 0xFFFFFF;
			_tfOutput.textColor = 0xFF0000;
			_tfOutput.autoSize = TextFieldAutoSize.LEFT;
			_tfOutput.type = TextFieldType.DYNAMIC;
			_tfOutput.text = "NOW LOADING... 0%";
			this.addChild(_tfOutput);
			
			loader = new Loader();
			loader.contentLoaderInfo.addEventListener(ProgressEvent.PROGRESS, loadProgress);
			loader.contentLoaderInfo.addEventListener(Event.COMPLETE, loadComplete);
			loader.load(new URLRequest("Launcher.swf"));
		}
		
		private function loadProgress(event:ProgressEvent):void
		{
			var percentLoaded:Number = event.bytesLoaded/event.bytesTotal ;
			percentLoaded = Math.round(percentLoaded * 100);
			trace("Loading: " + percentLoaded + "%");
			_tfOutput.text = "NOW LOADING... " + percentLoaded + "%";
		}
		
		private function loadComplete(event:Event):void
		{
			trace("Complete");
			this.removeChild(_tfOutput);
			event.target.removeEventListener( Event.COMPLETE, loadComplete );
			event.target.removeEventListener( ProgressEvent.PROGRESS, loadProgress );
			this.addChild(loader);
		}
	}
	
}