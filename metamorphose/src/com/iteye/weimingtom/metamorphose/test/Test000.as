package com.iteye.weimingtom.metamorphose.test
{
	import flash.display.Sprite;
	import flash.utils.Dictionary;
	import flash.utils.getQualifiedClassName;
	
	/**
	 * 这个类用于测试容易与Java混淆的AS3代码
	 */
	public class Test000 extends Sprite
	{

		public function Test000()
		{
			test1();
		}

		//如果switch变量与case变量类型不同会有问题
		public function test1():void
		{
			//=> a
			switch('a')
			{
				default:
					trace("default");
					break;
				
				case 'a':
					trace("a");
					break;
			}
			
			//=> default
			var a:uint = 'a'.charCodeAt();
			switch(a) //错误，应该写成switch(String.fromCharCode(a))
			{
				default:
					trace("default");
					break;
				
				case 'a':
					trace("a");
					break;				
			}
			
			//=> default
			var b:int = 'a' as int; //错误，应该写成var b:int = 'a'.charCodeAt()
			//trace(b);             //int('a')同样是错误的，但int()可以转换数字字符为相应的数字（非ASCII）
			switch(b)               //错误，应该写成switch(String.fromCharCode(a))
			{
				default:
					trace("default");
					break;
				
				case 'a':
					trace("a");
					break;					
			}
			
			//测试相关方法
			var str:String = "test string";
			trace(str.substring(2, 6));  //=>st s
			trace(str.charAt(2));        //=>s    注意返回的是String值，不是int值，
			                             //（用在字符串或switch时才与Java的行为相同）
			trace(str.substr(0, "t".length) == "t"); //=>true
			trace(str.charAt() == "t");              //=>true
			trace(str.indexOf("s"));     //=>2
			trace(str.indexOf("s", 3));  //=>5
			trace("\033".length);        //=>3
			trace(033 == 33);            //=>true AS3不支持8进制的整型字面值，前缀0忽略
			trace(0x33 == 33);           //=>false
			trace("\033" == "33");       //=>false AS3不支持\xxx写法
			trace("\x61" == "a");        //=>true 2位的16进制
			trace("\u0061" == "a");      //=>true 另一种写法 ，针对unicode的4位16进制
			trace(str.lastIndexOf("s")); //=>5
			trace(String.fromCharCode('B'.charCodeAt()).toLowerCase()); //=>b
			trace(String.fromCharCode('t'.charCodeAt()).toLowerCase()); //=>t
			trace(String.fromCharCode('1'.charCodeAt()).toLowerCase()); //=>1
			var ch:String = '1';
			trace('0'.charCodeAt() <= ch.charCodeAt() &&
				ch.charCodeAt() <= '9'.charCodeAt());  //=>true  判断是否数字
			//关于with的问题，优先使用with变量
			var obj:Object = {name:"obj"};
			with(obj)
			{
				trace(name); //=>obj
			}
			
			var name:String = "Oh, I have no name!"
			with(obj)
			{
				trace(name); //=> obj
			}
			
			//测试charAt在switch中的使用
			switch("test".charAt(2))
			{
				default:
					trace("default");
					break;
				
				case 's':
					trace("s");
					break;					
			}
			
			//测试Dictonary
			var dic:Dictionary = new Dictionary();
			var i1:Object = {test:10};
			var i2:Object = {test:14};
			dic[i1] = "hello";
			dic[i2] = "world";
			for(var key:Object in dic)
			{
				trace("key:", key.test);
			}
			
			//测试delete
			var str2:String = "0123456789";
			trace("delete:", str2.substring(0, 3) + str2.substring(5)); //=> 0156789
			//测试insert
			var str3:String = "==========";
			trace("insert:", str3.substring(0, 3) + String(97) + str3.substring(3)); 
			// => ===97=======
			//测试push(null)
			var arr:Array = new Array();
			trace("arr.length=", arr.length);
			arr.push(null);
			trace("arr.length=", arr.length);
			
			//测试concat
			var str4:String = "hello";
			str4.concat(",world!");
			trace(str4); //=>hello
			str4 = str4.concat(",world!");
			trace(str4); //=>hello,world!
			
			//trace(new int(10));
			
			//在AS3中如果使用setter,getter,则对于Array会造成错误
			
			//Lua.push & Lua.__push
			
			//注意检查setter的参数是否同名
			
			trace("this type:" + flash.utils.getQualifiedClassName(this))
			//trace("describeType:" + flash.utils.describeType(this).toString());
			//+ flash.utils.describeType(this).toString());
		}
	}
}