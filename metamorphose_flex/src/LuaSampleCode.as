package
{
	import flash.utils.ByteArray;
	
	import mx.collections.ArrayCollection;
	
	public class LuaSampleCode
	{
		[Embed(source="../assets/accept-basic/bisect.lua", mimeType="application/octet-stream")]
		private static var _bisectClass:Class;
		[Embed(source="../assets/accept-basic/cf.lua", mimeType="application/octet-stream")]
		private static var _cfClass:Class;
		[Embed(source="../assets/accept-basic/echo.lua", mimeType="application/octet-stream")]
		private static var _echoClass:Class; //miss
		[Embed(source="../assets/accept-basic/env.lua", mimeType="application/octet-stream")]
		private static var _envClass:Class;
		[Embed(source="../assets/accept-basic/factorial.lua", mimeType="application/octet-stream")]
		private static var _factorialClass:Class;
		[Embed(source="../assets/accept-basic/fib.lua", mimeType="application/octet-stream")]
		private static var _fibClass:Class;
		[Embed(source="../assets/accept-basic/fibfor.lua", mimeType="application/octet-stream")]
		private static var _fibforClass:Class;
		[Embed(source="../assets/accept-basic/globals.lua", mimeType="application/octet-stream")]
		private static var _globalsClass:Class; //miss
		[Embed(source="../assets/accept-basic/hello.lua", mimeType="application/octet-stream")]
		private static var _helloClass:Class; //miss
		[Embed(source="../assets/accept-basic/life.lua", mimeType="application/octet-stream")]
		private static var _lifeClass:Class;
		[Embed(source="../assets/accept-basic/luac.lua", mimeType="application/octet-stream")]
		private static var _luacClass:Class; //miss
		[Embed(source="../assets/accept-basic/printf.lua", mimeType="application/octet-stream")]
		private static var _printfClass:Class;
		[Embed(source="../assets/accept-basic/readonly.lua", mimeType="application/octet-stream")]
		private static var _readonlyClass:Class;
		[Embed(source="../assets/accept-basic/sieve.lua", mimeType="application/octet-stream")]
		private static var _sieveClass:Class;
		[Embed(source="../assets/accept-basic/sort.lua", mimeType="application/octet-stream")]
		private static var _sortClass:Class;
		[Embed(source="../assets/accept-basic/table.lua", mimeType="application/octet-stream")]
		private static var _tableClass:Class; //miss
		[Embed(source="../assets/accept-basic/trace-calls.lua", mimeType="application/octet-stream")]
		private static var _traceCallsClass:Class; //miss
		[Embed(source="../assets/accept-basic/trace-globals.lua", mimeType="application/octet-stream")]
		private static var _traceGlobalsClass:Class; //miss
		[Embed(source="../assets/accept-basic/xd.lua", mimeType="application/octet-stream")]
		private static var _xdClass:Class; //miss

		private static var _embeddedLuaFiles:Array = [
			{label:"Bisection method for solving non-linear equations", asset:_bisectClass, filename:"assets/accept-basic/bisect.lua"},
			{label:"Temperature conversion table (celsius to farenheit)", asset:_cfClass, filename:"assets/accept-basic/cf.lua"},
			{label:"Echo command line arguments", asset:_echoClass, filename:"assets/accept-basic/echo.lua"}, //miss
			{label:"Environment variables as automatic global variables", asset:_envClass, filename:"assets/accept-basic/env.lua"},
			{label:"Factorial without recursion", asset:_factorialClass, filename:"assets/accept-basic/factorial.lua"},
			{label:"Fibonacci function with cache", asset:_fibClass, filename:"assets/accept-basic/fib.lua"},
			{label:"Fibonacci numbers with coroutines and generators", asset:_fibforClass, filename:"assets/accept-basic/fibfor.lua"},
			{label:"Report global variable usage", asset:_globalsClass, filename:"assets/accept-basic/globals.lua"}, //miss
			{label:"The first program in every language", asset:_helloClass, filename:"assets/accept-basic/hello.lua"}, //miss
			{label:"Conway's Game of Life", asset:_lifeClass, filename:"assets/accept-basic/life.lua"},
			{label:"Bare-bones luac", asset:_luacClass, filename:"assets/accept-basic/luac.lua"}, //miss
			{label:"An implementation of printf", asset:_printfClass, filename:"assets/accept-basic/printf.lua"},
			{label:"The sieve of of Eratosthenes programmed with coroutines", asset:_readonlyClass, filename:"assets/accept-basic/readonly.lua"},
			{label:"Make global variables readonly", asset:_sieveClass, filename:"assets/accept-basic/sieve.lua"},
			{label:"Two implementations of a sort function", asset:_sortClass, filename:"assets/accept-basic/sort.lua"},
			{label:"Make table, grouping all data for the same item", asset:_tableClass, filename:"assets/accept-basic/table.lua"}, //miss
			{label:"Trace calls", asset:_traceCallsClass, filename:"assets/accept-basic/traceCalls.lua"}, //miss
			{label:"Trace assigments to global variables", asset:_traceGlobalsClass, filename:"assets/accept-basic/traceGlobals.lua"}, //miss
			{label:"Hex dump", asset:_xdClass, filename:"assets/accept-basic/xd.lua"},//miss
		];
		
		[Bindable]
		public static var sampleCode:ArrayCollection = new ArrayCollection();
		
		public static function init():void
		{
			var luaFile:Object;
			sampleCode.removeAll();
			for each (luaFile in _embeddedLuaFiles)
			{
				//import mx.core.ByteArrayAsset;
				//var luaAsset:ByteArrayAsset = ByteArrayAsset(new luaFile.asset());
				//var luaString:String = luaAsset.toString();
				var luaString:String = clsObjToUTF8(new luaFile.asset());
				sampleCode.addItem({label:luaFile.label, code:luaString, filename:luaFile.filename});
			}
		}
		
		private static function clsObjToUTF8(clsObj:Object):String
		{
			var bytes:ByteArray = ByteArray(clsObj);
			var str:String = bytes.readUTFBytes(bytes.bytesAvailable);
			var result:String = str.replace(/\r\n/g, "\n");
			return result;
		}
	}
}
