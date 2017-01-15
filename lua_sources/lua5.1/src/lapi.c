/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define lapi_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";



#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))

#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)

#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}



static TValue *index2adr (lua_State *L, int idx) {
  if (idx > 0) {
    TValue *o = L->base + (idx - 1);
    api_check(L, idx <= L->ci->top - L->base);
    if (o >= L->top) return cast(TValue *, luaO_nilobject);
    else return o;
  }
  else if (idx > LUA_REGISTRYINDEX) {
    api_check(L, idx != 0 && -idx <= L->top - L->base);
    return L->top + idx;
  }
  else switch (idx) {  /* pseudo-indices */
    case LUA_REGISTRYINDEX: return registry(L);
    case LUA_ENVIRONINDEX: {
      Closure *func = curr_func(L);
      sethvalue(L, &L->env, func->c.env);
      return &L->env;
    }
    case LUA_GLOBALSINDEX: return gt(L);
    default: {
      Closure *func = curr_func(L);
      idx = LUA_GLOBALSINDEX - idx;
      return (idx <= func->c.nupvalues)
                ? &func->c.upvalue[idx-1]
                : cast(TValue *, luaO_nilobject);
    }
  }
}


static Table *getcurrenv (lua_State *L) {
  if (L->ci == L->base_ci)  /* no enclosing function? */
    return hvalue(gt(L));  /* use global table as environment */
  else {
    Closure *func = curr_func(L);
    return func->c.env;
  }
}


void luaA_pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top, o);
  api_incr_top(L);
}

/*
lua_checkstack
[-0, +0, m] 
int lua_checkstack (lua_State *L, int extra);
Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot grow the stack to that size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged. 

lua_checkstack
[-0, +0, m] 
int lua_checkstack (lua_State *L, int extra);
ȷ��ջ�д�������extra������ջ��λ�����ջ�����������Ǹ��ߴ��򷵻ؼ١��������Ӳ���Сջ�����ջ�Ѿ����³ߴ�����ޱ仯�� 
*/
LUA_API int lua_checkstack (lua_State *L, int size) {
  int res = 1;
  lua_lock(L);
  if (size > LUAI_MAXCSTACK || (L->top - L->base + size) > LUAI_MAXCSTACK)
    res = 0;  /* stack overflow */
  else if (size > 0) {
    luaD_checkstack(L, size);
    if (L->ci->top < L->top + size)
      L->ci->top = L->top + size;
  }
  lua_unlock(L);
  return res;
}

/*
lua_xmove
[-?, +?, -] 
	void lua_xmove (lua_State *from, lua_State *to, int n);
	Exchange values between different threads of the same global state. 
	This function pops n values from the stack from, and pushes them onto the stack to. 

lua_xmove
[-?, +?, -] 
	void lua_xmove (lua_State *from, lua_State *to, int n);
	������ͬȫ��״̬�в�ͬ�̵߳�ֵ��
	��������Ӷ�ջfrom�е���n��ֵ��Ȼ�������ѹ���ջto��
*/
LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
  int i;
  if (from == to) return;
  lua_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to));
  api_check(from, to->ci->top - to->top >= n);
  from->top -= n;
  for (i = 0; i < n; i++) {
    setobj2s(to, to->top++, from->top + i);
  }
  lua_unlock(to);
}


LUA_API void lua_setlevel (lua_State *from, lua_State *to) {
  to->nCcalls = from->nCcalls;
}

/*
lua_atpanic
[-0, +0, -]
	lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);
	Sets a new panic function and returns the old one. 
	If an error happens outside any protected environment, Lua calls a panic function and then calls exit(EXIT_FAILURE), thus exiting the host application. Your panic function can avoid this exit by never returning (e.g., doing a long jump). 
	The panic function can access the error message at the top of the stack. 

lua_atpanic
[-0, +0, -] 
	lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf);
	�����µ�Ӧ����panic������������ǰһ���� 
	������κ��ܱ����Ļ������淢���˴���Lua����Ӧ���������ŵ���exit(EXIT_FAILURE)���Ӷ��˳������������Ӧ��������ͨ���������أ�����ִ��һ�γ���ת���Ա�������˳��� 
	Ӧ�������ɷ���ջ���Ĵ�����Ϣ�� 
*/
LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}

/*
lua_newthread
[-0, +1, m] 
	lua_State *lua_newthread (lua_State *L);
	Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new state returned by this function shares with the original state all global objects (such as tables), but has an independent execution stack. 
	There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object. 

lua_newthread
[-0, +1, m] 
	lua_State *lua_newthread (lua_State *L);
	�������̣߳�����ѹջ��������ָ��lua_State��ָ�룬����ʾ�����̡߳����������ص���״̬�����ʼ״̬����������ȫ�ֶ���������������ж�����ִ��ջ�� 
	û�йرջ������̵߳���ʽ���������κ�Lua����һ�����߳��������ռ���֧�䡣 
*/
LUA_API lua_State *lua_newthread (lua_State *L) {
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  L1 = luaE_newthread(L);
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  lua_unlock(L);
  luai_userstatethread(L, L1);
  return L1;
}



/*
** basic stack manipulation
*/

/*
lua_gettop
[-0, +0, -] 
	int lua_gettop (lua_State *L);
	Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack). 

lua_gettop
[-0, +0, -] 
	int lua_gettop (lua_State *L);
	����ջ��Ԫ�ص���������Ϊ������1��ʼ���ý������ջ��Ԫ�ص�����������0��ʾ��ջ���� 
*/
LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}

/*
lua_settop
[-?, +?, -] 
	void lua_settop (lua_State *L, int index);
	Accepts any acceptable index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed. 

lua_settop
[-?, +?, -] 
	void lua_settop (lua_State *L, int index);
	��������ɽ��ܵ�����������0����ջ�������ڴ������ϡ�
	����µ�ջ������ԭ�еģ���ô�´�����Ԫ�ر����Ϊ�ա�
	�������Ϊ0����ô���ж�ջԪ�ض��ᱻɾ���� 
*/
LUA_API void lua_settop (lua_State *L, int idx) {
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - L->base);
    while (L->top < L->base + idx)
      setnilvalue(L->top++);
    L->top = L->base + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - L->base));
    L->top += idx+1;  /* `subtract' index (index is negative) */
  }
  lua_unlock(L);
}

/*
lua_remove
[-1, +0, -] 
	void lua_remove (lua_State *L, int index);
	Removes the element at the given valid index, shifting down the elements above this index to fill the gap. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

lua_remove
[-1, +0, -] 
	void lua_remove (lua_State *L, int index);
	�Ƴ���������Ч��������Ԫ�أ����������������Ԫ������������϶��������α�������ã���Ϊα����������ʵ��ջλ�á� 
*/
LUA_API void lua_remove (lua_State *L, int idx) {
  StkId p;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  while (++p < L->top) setobjs2s(L, p-1, p);
  L->top--;
  lua_unlock(L);
}

/*
lua_insert
[-1, +1, -] 
	void lua_insert (lua_State *L, int index);
	Moves the top element into the given valid index, shifting up the elements above this index to open space. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position. 

lua_insert
[-1, +1, -] 
	void lua_insert (lua_State *L, int index);
	��ջ��Ԫ�������������Ч���������������������Ԫ�����������ſռ䡣������α�������ã���Ϊα����������ʵ��ջλ�á� 
*/
LUA_API void lua_insert (lua_State *L, int idx) {
  StkId p;
  StkId q;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
  setobjs2s(L, p, L->top);
  lua_unlock(L);
}

/*
lua_replace
[-1, +0, -] 
	void lua_replace (lua_State *L, int index);
	Moves the top element into the given position (and pops it), without shifting any element (therefore replacing the value at the given position). 

lua_replace
[-1, +0, -] 
	void lua_replace (lua_State *L, int index);
	��ջ��Ԫ���ƶ��������������У������������������ƶ��κ�Ԫ�أ�����滻����λ�õ�ֵ���� 
*/
LUA_API void lua_replace (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  /* explicit test for incompatible code */
  if (idx == LUA_ENVIRONINDEX && L->ci == L->base_ci)
    luaG_runerror(L, "no calling environment");
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  if (idx == LUA_ENVIRONINDEX) {
    Closure *func = curr_func(L);
    api_check(L, ttistable(L->top - 1)); 
    func->c.env = hvalue(L->top - 1);
    luaC_barrier(L, func, L->top - 1);
  }
  else {
    setobj(L, o, L->top - 1);
    if (idx < LUA_GLOBALSINDEX)  /* function upvalue? */
      luaC_barrier(L, curr_func(L), L->top - 1);
  }
  L->top--;
  lua_unlock(L);
}

/*
lua_pushvalue
[-0, +1, -] 
	void lua_pushvalue (lua_State *L, int index);
	Pushes a copy of the element at the given valid index onto the stack. 

lua_pushvalue
[-0, +1, -] 
	void lua_pushvalue (lua_State *L, int index);
	�������Ϸ�index����Ԫ�صĿ���ѹ��ջ�ڡ�
*/
LUA_API void lua_pushvalue (lua_State *L, int idx) {
  lua_lock(L);
  setobj2s(L, L->top, index2adr(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/

/*
lua_type
[-0, +0, -] 
	int lua_type (lua_State *L, int index);
	Returns the type of the value in the given acceptable index, or LUA_TNONE for a non-valid index (that is, an index to an "empty" stack position). The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA. 

lua_type
[-0, +0, -] 
	int lua_type (lua_State *L, int index);
	���ظ������Ͽɵ���������ֵ�����ͣ����߶Բ��Ϸ�����������LUA_TNONE����ָ�򡰿ա�ջλ�õ���������lua_type���ص�������lua.h�ж��壬������Ϊ����ĳ����� LUA_TNIL��LUA_TNUMBER��LUA_TBOOLEAN��LUA_TSTRING��LUA_TTABLE��LUA_TFUNCTION��LUA_TUSERDATA��LUA_TTHREAD��LUA_TLIGHTUSERDATA�� 
*/
LUA_API int lua_type (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}

/*
lua_typename
[-0, +0, -] 
	const char *lua_typename  (lua_State *L, int tp);
	Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type. 

lua_typename
[-0, +0, -] 
	const char *lua_typename  (lua_State *L, int tp);
	������ֵtp�������������tp������lua_type�ķ���ֵ������һ���� 
*/
LUA_API const char *lua_typename (lua_State *L, int t) {
  UNUSED(L);
  return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}

/*
lua_iscfunction
[-0, +0, -] 
	int lua_iscfunction (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a C function, and 0 otherwise. 

lua_iscfunction
[-0, +0, -] 
	int lua_iscfunction (lua_State *L, int index);
	��������ɽ�����������ֵΪC�����򷵻�1�����򷵻�0��
*/
LUA_API int lua_iscfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return iscfunction(o);
}

/*
lua_isnumber
[-0, +0, -] 
	int lua_isnumber (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a number or a string convertible to a number, and 0 otherwise. 

lua_isnumber
[-0, +0, -] 
	int lua_isnumber (lua_State *L, int index);
	��������ɽ�����������ֵ�������߿�ת��Ϊ�����ַ����򷵻�1�����򷵻�0��
*/
LUA_API int lua_isnumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  return tonumber(o, &n);
}

/*
lua_isstring
[-0, +0, -] 
	int lua_isstring (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a string or a number (which is always convertible to a string), and 0 otherwise. 

lua_isstring
[-0, +0, -] 
	int lua_isstring (lua_State *L, int index);
	��������ɽ�����������ֵ���ַ��������������ǿ���ת��Ϊ�ַ������򷵻�1�����򷵻�0��
*/
LUA_API int lua_isstring (lua_State *L, int idx) {
  int t = lua_type(L, idx);
  return (t == LUA_TSTRING || t == LUA_TNUMBER);
}

/*
lua_isuserdata
[-0, +0, -] 
	int lua_isuserdata (lua_State *L, int index);
	Returns 1 if the value at the given acceptable index is a userdata (either full or light), and 0 otherwise. 

lua_isuserdata
[-0, +0, -] 
	int lua_isuserdata (lua_State *L, int index);
	��������ɽ�����������ֵ��һ��userdata����ȫ�������������򷵻�1�����򷵻�0��
*/
LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return (ttisuserdata(o) || ttislightuserdata(o));
}

/*
lua_rawequal
[-0, +0, -] 
	int lua_rawequal (lua_State *L, int index1, int index2);
	Returns 1 if the two values in acceptable indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid. 

lua_rawequal
[-0, +0, -] 
	int lua_rawequal (lua_State *L, int index1, int index2);
	������������յ�����index1��index2����ֵԭ����ȣ���������Ԫ���������򷵻�1��
	���򣬷���0��
	��������������Ϸ���Ҳ����0��
*/
LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  StkId o1 = index2adr(L, index1);
  StkId o2 = index2adr(L, index2);
  return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaO_rawequalObj(o1, o2);
}

/*
lua_equal
[-0, +0, e] 
int lua_equal (lua_State *L, int index1, int index2);
Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

lua_equal
[-0, +0, e] 
int lua_equal (lua_State *L, int index1, int index2);
����Lua��==�����������壨�����ܵ���Ԫ���������Ƚ��ڿɽ�������index1��index2�е�����ֵ���������򷵻�1�����򷵻�0������κ�������ЧҲ����0�� 
*/
LUA_API int lua_equal (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
  lua_unlock(L);
  return i;
}

/*
lua_lessthan
[-0, +0, e] 
	int lua_lessthan (lua_State *L, int index1, int index2);
	Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid. 

lua_lessthan
[-0, +0, e] 
	int lua_lessthan (lua_State *L, int index1, int index2);
	��������ɽ�������index1����ֵС������index2����ֵ�򷵻�1����ѭLua��<����������壨�������ܵ���Ԫ��������
	���򷵻�0��
	��������������Ϸ�Ҳ�᷵��0��
*/
LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
       : luaV_lessthan(L, o1, o2);
  lua_unlock(L);
  return i;
}

/*
lua_tonumber
[-0, +0, -] 
	lua_Number lua_tonumber (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to the C type lua_Number (see lua_Number). The Lua value must be a number or a string convertible to a number (see ��2.2.1); otherwise, lua_tonumber returns 0. 

lua_tonumber
[-0, +0, -] 
	lua_Number lua_tonumber (lua_State *L, int index);
	�������ɽ�����������ֵת��ΪC����lua_Number���ο�lua_Number����
	Luaֵ����Ϊһ���������ת��Ϊ�����ַ������ο���2.2.1��������lua_tonumber����0��
*/
LUA_API lua_Number lua_tonumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  if (tonumber(o, &n))
    return nvalue(o);
  else
    return 0;
}

/*
lua_tointeger
[-0, +0, -] 
	lua_Integer lua_tointeger (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to the signed integral type lua_Integer. The Lua value must be a number or a string convertible to a number (see ��2.2.1); otherwise, lua_tointeger returns 0. 
	If the number is not an integer, it is truncated in some non-specified way. 

lua_tointeger
[-0, +0, -] 
	lua_Integer lua_tointeger (lua_State *L, int index);
	�������ɽ�����������ֵת��Ϊ�����ŵ�����lua_Integer��
	Luaֵ������һ�������߿�תΪ�����ַ������ο���2.2.1��������lua_tointeger����0��
	������������������Բ�ȷ���ķ�ʽ�����С�
*/
LUA_API lua_Integer lua_tointeger (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  if (tonumber(o, &n)) {
    lua_Integer res;
    lua_Number num = nvalue(o);
    lua_number2integer(res, num);
    return res;
  }
  else
    return 0;
}

/*
lua_toboolean
[-0, +0, -] 
	int lua_toboolean (lua_State *L, int index);
	Converts the Lua value at the given acceptable index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for any Lua value different from false and nil; otherwise it returns 0. It also returns 0 when called with a non-valid index. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.) 

lua_toboolean
[-0, +0, -] 
	int lua_toboolean (lua_State *L, int index);
	�������ɽ��ܵ���������Luaֵת��ΪC�Ĳ���ֵ(0��1)��
	����Lua�����в��������������κβ���false��nil��ֵlua_toboolean����1�����򷵻�0��
	�����÷Ƿ���������ʱҲ����0��
	�������ֻ�����ʵ�ʵĲ���ֵ��ʹ��lua_isboolean�����ֵ���ͣ�
*/
LUA_API int lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return !l_isfalse(o);
}

/*
lua_tolstring
[-0, +0, m] 
	const char *lua_tolstring (lua_State *L, int index, size_t *len);
	Converts the Lua value at the given acceptable index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.) 
	lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack. 

lua_tolstring
[-0, +0, m] 
	const char *lua_tolstring (lua_State *L, int index, size_t *len);
	�������ɽ�����������ֵת��ΪC���ַ�����
	���len����NULL��Ҳ��������*lenΪ�ַ������ȡ�
	Luaֵ������һ���ַ������������������������NULL��
	���ֵ��������ôlua_tolstring����ı��ջ��ʵ��ֵΪ�ַ�����
	����lua_tolstringӦ�õ�������ļ��У����ָı�����lua_next����
	lua_tolstring����һ����ȫ�����ָ��Lua״̬�ڲ����ַ�����ָ�롣
	��ΪLua���������գ����Բ���֤lua_tolstring�����ص�ָ������Ӧֵ�Ӷ�ջ��ɾ�����ԺϷ���
*/
LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  StkId o = index2adr(L, idx);
  if (!ttisstring(o)) {
    lua_lock(L);  /* `luaV_tostring' may create a new string */
    if (!luaV_tostring(L, o)) {  /* conversion failed? */
      if (len != NULL) *len = 0;
      lua_unlock(L);
      return NULL;
    }
    luaC_checkGC(L);
    o = index2adr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL) *len = tsvalue(o)->len;
  return svalue(o);
}

/*
lua_objlen
[-0, +0, -] 
	size_t lua_objlen (lua_State *L, int index);
	Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0. 

lua_objlen
[-0, +0, -] 
	size_t lua_objlen (lua_State *L, int index);
	���ظ������Ͽɵ���������ֵ�ġ����ȡ��������ַ����������䳤�ȣ����ڱ�����ȡ������������#�����Ľ���������û����ݣ�����Ϊ�������ڴ��ĳߴ磻��������������0�� 
*/
LUA_API size_t lua_objlen (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TSTRING: return tsvalue(o)->len;
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(hvalue(o));
    case LUA_TNUMBER: {
      size_t l;
      lua_lock(L);  /* `luaV_tostring' may create a new string */
      l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
      lua_unlock(L);
      return l;
    }
    default: return 0;
  }
}

/*
lua_tocfunction
[-0, +0, -] 
	lua_CFunction lua_tocfunction (lua_State *L, int index);
	Converts a value at the given acceptable index to a C function. That value must be a C function; otherwise, returns NULL. 

lua_tocfunction
[-0, +0, -] 
	lua_CFunction lua_tocfunction (lua_State *L, int index);
	�������ɽ�����������ֵת��ΪC������
	�Ǹ�ֵ������C���������򷵻�NULL��
*/
LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!iscfunction(o)) ? NULL : clvalue(o)->c.f;
}

/*
lua_touserdata
[-0, +0, -] 
	void *lua_touserdata (lua_State *L, int index);
	If the value at the given acceptable index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL. 

lua_touserdata[-0, +0, -]
	����������Ͽɵ���������ֵ���������û����ݣ��򷵻�����ַ��
	����������û����ݣ�������ָ�롣���򷵻�NULL�� 
*/
LUA_API void *lua_touserdata (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TUSERDATA: return (rawuvalue(o) + 1);
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}

/*
lua_tothread
[-0, +0, -] 
	lua_State *lua_tothread (lua_State *L, int index);
	Converts the value at the given acceptable index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL. 

lua_tothread
[-0, +0, -] 
	lua_State *lua_tothread (lua_State *L, int index);
	�Ѹ������Ͽɵ���������ֵת��ΪLua�̣߳���lua_State*��ʾ������ֵ����Ϊ�̣߳����򣬱���������NULL�� 
*/
LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}

/*
lua_topointer
[-0, +0, -] 
	const void *lua_topointer (lua_State *L, int index);
	Converts the value at the given acceptable index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value. 
	Typically this function is used only for debug information. 

lua_topointer
[-0, +0, -] 
	const void *lua_topointer (lua_State *L, int index);
	�������ɽ�����������ֵת��Ϊ���͵�Cָ�루void*����
	���ֵ�������û��������ݣ����̣߳������Ǻ���������lua_topointer����NULL��
	��ͬ�Ķ���������ͬ��ָ�롣
	û�з������԰�ָ��ת������ԭ��ֵ��
	�ر��������������ڵ�����Ϣ��
*/
LUA_API const void *lua_topointer (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TTABLE: return hvalue(o);
    case LUA_TFUNCTION: return clvalue(o);
    case LUA_TTHREAD: return thvalue(o);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      return lua_touserdata(L, idx);
    default: return NULL;
  }
}



/*
** push functions (C -> stack)
*/

/*
lua_pushnil
[-0, +1, -] 
	void lua_pushnil (lua_State *L);
	Pushes a nil value onto the stack. 

lua_pushnil
[-0, +1, -] 
	void lua_pushnil (lua_State *L);
	ѹ��һ����ֵ����ջ�� 
*/
LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushnumber
[-0, +1, -] 
	void lua_pushnumber (lua_State *L, lua_Number n);
	Pushes a number with value n onto the stack. 

lua_pushnumber
[-0, +1, -] 
	void lua_pushnumber (lua_State *L, lua_Number n);
	ѹ��һ��ӵ��ֵn�����ֽ���ջ��
*/
LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  setnvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushinteger
[-0, +1, -] 
	void lua_pushinteger (lua_State *L, lua_Integer n);
	Pushes a number with value n onto the stack. 

lua_pushinteger
[-0, +1, -] 
	void lua_pushinteger (lua_State *L, lua_Integer n);
	��һ��ֵΪn������ѹջ�� 
*/
LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setnvalue(L->top, cast_num(n));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushlstring
[-0, +1, m] 
	void lua_pushlstring (lua_State *L, const char *s, size_t len);
	Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain embedded zeros. 

lua_pushlstring
[-0, +1, m] 
	void lua_pushlstring (lua_State *L, const char *s, size_t len);
	��sָ��ĳߴ�Ϊlen���ַ���ѹջ��Lua���죨�����ã������ַ������ڲ����������Ժ������غ�s���ڴ����̿ɱ��ͷŻ����á��ַ����ɺ�����Ƕ��0�� 
*/
LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len) {
  lua_lock(L);
  luaC_checkGC(L);
  setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushstring
[-0, +1, m] 
	void lua_pushstring (lua_State *L, const char *s);
	Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string cannot contain embedded zeros; it is assumed to end at the first zero. 

lua_pushstring
[-0, +1, m]
void lua_pushstring (lua_State *L, const char *s);
	��sָ�����0��β���ַ���ѹջ��Lua���죨�����ã������ַ������ڲ�������
	���Ժ������غ�s���ڴ����̿ɱ��ͷŻ����á��ַ������ɺ�����Ƕ��0��
	�ٶ������׸�0�������� 
*/
LUA_API void lua_pushstring (lua_State *L, const char *s) {
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlstring(L, s, strlen(s));
}

/*
lua_pushvfstring
[-0, +1, m] 
	const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);
	Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments. 

lua_pushvfstring
[-0, +1, m] 
	const char *lua_pushvfstring (lua_State *L,
								  const char *fmt,
								  va_list argp);
	��Ч��lua_pushfstring�����˽���һ��va_list�Դ������������
*/
LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  lua_unlock(L);
  return ret;
}

/*
lua_pushfstring
[-0, +1, m] 
	const char *lua_pushfstring (lua_State *L, const char *fmt, ...);
	Pushes onto the stack a formatted string and returns a pointer to this string. It is similar to the C function sprintf, but has some important differences: 
	You do not have to allocate space for the result: the result is a Lua string and Lua takes care of memory allocation (and deallocation, through garbage collection). 
	The conversion specifiers are quite restricted. There are no flags, widths, or precisions. The conversion specifiers can only be '%%' (inserts a '%' in the string), '%s' (inserts a zero-terminated string, with no size restrictions), '%f' (inserts a lua_Number), '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an int), and '%c' (inserts an int as a character). 

lua_pushfstring
[-0, +1, m] 
	const char *lua_pushfstring (lua_State *L, const char *fmt, ...);
	����ʽ�����ַ���ѹջ������ָ������ָ�롣����C����sprintf���ƣ���Ҳ��һЩ��Ҫ������ 
	�㲻��ҪΪ�������ռ䣺�����Lua�ַ�����Lua���տ��ڴ���䣨�Լ�ͨ�������ռ�������䣩�� 
	ת��˵�����ǳ������ޡ�û�б�ǡ���Ȼ򾫶ȡ�ת��˵����ֻ����'%%' �����ַ����в���һ��'%'����'%s' ������һ����0��β���ַ�����û�гߴ����ƣ���'%f' ������һ��lua_Number����'%p' ������һ��ָ����Ϊʮ������������'%d' ������һ��int�����Լ�'%c' ������һ��int��Ϊ�ַ����� 
*/
LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}

/*
lua_pushcclosure
[-n, +1, m] 
	void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);
	Pushes a new C closure onto the stack. 
	When a C function is created, it is possible to associate some values with it, thus creating a C closure (see ��3.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack. 
	The maximum value for n is 255. 

lua_pushcclosure
[-n, +1, m] 
	void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n);
	���µ�C�հ�ѹջ�� 
	��C����������ʱ�������԰�һЩֵ���Լ������������ʹ�����C�հ�������3.4�������������ۺ�ʱ�������ã���Щֵ�Ըú������ǿɷ��ʵġ�Ҫ��ֵ��C����������������ЩֵӦ����ѹջ�����ж��ֵʱ��һ��ֵ����ѹջ����Ȼ���ò���n����lua_pushcclosure������C����������ѹջ��n����Ӧ���Ѷ���ֵ�������ú�����lua_pushcclosureҲ�Ὣ��Щֵ��ջ�е����� 
	n�����ֵ��255�� 
*/
LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  Closure *cl;
  lua_lock(L);
  luaC_checkGC(L);
  api_checknelems(L, n);
  cl = luaF_newCclosure(L, n, getcurrenv(L));
  cl->c.f = fn;
  L->top -= n;
  while (n--)
    setobj2n(L, &cl->c.upvalue[n], L->top+n);
  setclvalue(L, L->top, cl);
  lua_assert(iswhite(obj2gco(cl)));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushboolean
[-0, +1, -] 
	void lua_pushboolean (lua_State *L, int b);
	Pushes a boolean value with value b onto the stack. 

lua_pushboolean
[-0, +1, -] 
	void lua_pushboolean (lua_State *L, int b);
	��ֵb��Ϊ����ֵѹջ�� 
*/
LUA_API void lua_pushboolean (lua_State *L, int b) {
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushlightuserdata
[-0, +1, -] 
	void lua_pushlightuserdata (lua_State *L, void *p);
	Pushes a light userdata onto the stack. 
	Userdata represent C values in Lua. A light userdata represents a pointer. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address. 

lua_pushlightuserdata
[-0, +1, -] 
	void lua_pushlightuserdata (lua_State *L, void *p);
	�������û�����ѹջ�� 
	��Lua���û����ݱ�ʾCֵ�������û����ݱ�ʾһ��ָ�롣���Ǹ�ֵ���������֣����㲻�ô���������û�е�����Ԫ�����������ᱻ���գ���ͬ�Ӳ�����������������ͬ��C��ַ�������û�������ȡ� 
*/
LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_pushthread
[-0, +1, -] 
	int lua_pushthread (lua_State *L);
	Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state. 

lua_pushthread
[-0, +1, -] 
	int lua_pushthread (lua_State *L);
	��L������߳�ѹ��ջ�С�
	�������߳���״̬�����߳��򷵻�1��
*/
LUA_API int lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/

/*
lua_gettable
[-1, +1, e] 
void lua_gettable (lua_State *L, int index);
Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack. 
This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event (see ��2.8). 

lua_gettable
[-1, +1, e] 
void lua_gettable (lua_State *L, int index);
��ֵt[k]ѹջ������t��ָ������Ч��������ֵ������k��ջ����ֵ�� 
������������ջ�������ֵ��������λ�ã���ͬLua��һ�������������ܴ������ڡ�index���¼���Ԫ����������2.8���� 
*/
LUA_API void lua_gettable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_gettable(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
}

/*
lua_getfield
[-0, +1, e] 
void lua_getfield (lua_State *L, int index, const char *k);
Pushes onto the stack the value t[k], where t is the value at the given valid index. As in Lua, this function may trigger a metamethod for the "index" event (see ��2.8). 

lua_getfield
[-0, +1, e] 
void lua_getfield (lua_State *L, int index, const char *k);
��t[k]��ֵѹջ������t�Ǹ�������Ч��������ֵ��ͬLua��һ�������������ܴ������ڡ�index���¼���Ԫ����������2.8���� 
*/
LUA_API void lua_getfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_gettable(L, t, &key, L->top);
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_rawget
[-1, +1, -] 
	void lua_rawget (lua_State *L, int index);
	Similar to lua_gettable, but does a raw access (i.e., without metamethods). 

lua_rawget
[-1, +1, -] 
	void lua_rawget (lua_State *L, int index);
	������lua_gettable����ִ��ԭ������(���磬û��Ԫ������ 
*/
LUA_API void lua_rawget (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
  lua_unlock(L);
}

/*
lua_rawgeti
[-0, +1, -] 
	void lua_rawgeti (lua_State *L, int index, int n);
	Pushes onto the stack the value t[n], where t is the value at the given valid index. The access is raw; that is, it does not invoke metamethods. 

lua_rawgeti
[-0, +1, -] 
	void lua_rawgeti (lua_State *L, int index, int n);
	��t[n]��ֵѹ��ջ�У�����t�������Ϸ�Index����ֵ��
	������ԭ���ģ���ִ��Ԫ����
*/
LUA_API void lua_rawgeti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_createtable
[-0, +1, m] 
void lua_createtable (lua_State *L, int narr, int nrec);
Creates a new empty table and pushes it onto the stack. The new table has space pre-allocated for narr array elements and nrec non-array elements. This pre-allocation is useful when you know exactly how many elements the table will have. Otherwise you can use the function lua_newtable. 

lua_createtable
[-0, +1, m] 
void lua_createtable (lua_State *L, int narr, int nrec);
�����µĿձ�����ѹջ���±�Ԥ����narr������Ԫ�غ�nrec��������Ԫ�صĿ��пռ䡣����ȷ�е�֪�����ɶ��ٸ�Ԫ��ʱ��Ԥ�����Ƿǳ����õġ���������ú���lua_newtable�� 
*/
LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
  lua_lock(L);
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, narray, nrec));
  api_incr_top(L);
  lua_unlock(L);
}

/*
lua_getmetatable
[-0, +(0|1), -] 
int lua_getmetatable (lua_State *L, int index);
Pushes onto the stack the metatable of the value at the given acceptable index. If the index is not valid, or if the value does not have a metatable, the function returns 0 and pushes nothing on the stack. 

lua_getmetatable
[-0, +(0|1), -] 
int lua_getmetatable (lua_State *L, int index);
���������Ͽɵ���������ֵ��Ԫ��ѹջ�����������Ч�����������ֵû��Ԫ������������0�Ҳ���ѹջ�κζ����� 
*/
LUA_API int lua_getmetatable (lua_State *L, int objindex) {
  const TValue *obj;
  Table *mt = NULL;
  int res;
  lua_lock(L);
  obj = index2adr(L, objindex);
  switch (ttype(obj)) {
    case LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(obj)];
      break;
  }
  if (mt == NULL)
    res = 0;
  else {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}

/*
lua_getfenv
[-0, +1, -] 
void lua_getfenv (lua_State *L, int index);
Pushes onto the stack the environment table of the value at the given index. 

lua_getfenv
[-0, +1, -] 
void lua_getfenv (lua_State *L, int index);
��������������ֵ�Ļ�����ѹջ�� 
*/
LUA_API void lua_getfenv (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      sethvalue(L, L->top, clvalue(o)->c.env);
      break;
    case LUA_TUSERDATA:
      sethvalue(L, L->top, uvalue(o)->env);
      break;
    case LUA_TTHREAD:
      setobj2s(L, L->top,  gt(thvalue(o)));
      break;
    default:
      setnilvalue(L->top);
      break;
  }
  api_incr_top(L);
  lua_unlock(L);
}


/*
** set functions (stack -> Lua)
*/

/*
lua_settable
[-2, +0, e] 
	void lua_settable (lua_State *L, int index);
	Does the equivalent to t[k] = v, where t is the value at the given valid index, v is the value at the top of the stack, and k is the value just below the top. 
	This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see ��2.8). 

lua_settable
[-2, +0, e] 
	void lua_settable (lua_State *L, int index);
	ִ��t[k] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ��k������ջ�������ֵ�� 
	������������ֵ������ջ��ͬLua��һ�������������ܴ�����newindex���¼���Ԫ����������2.8���� 
*/
LUA_API void lua_settable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_settable(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}

/*
lua_setfield
[-1, +0, e] 
	void lua_setfield (lua_State *L, int index, const char *k);
	Does the equivalent to t[k] = v, where t is the value at the given valid index and v is the value at the top of the stack. 
	This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see ��2.8). 

lua_setfield
[-1, +0, e] 
	void lua_setfield (lua_State *L, int index, const char *k);
	ִ��t[k] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ�� 
	��������ջ�е���ֵ��ͬLua��һ�������������ܴ�����newindex���¼���Ԫ����������2.8���� 
*/
LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_settable(L, t, &key, L->top - 1);
  L->top--;  /* pop value */
  lua_unlock(L);
}

/*
lua_rawset
[-2, +0, m] 
	void lua_rawset (lua_State *L, int index);
	Similar to lua_settable, but does a raw assignment (i.e., without metamethods). 

lua_rawset
[-2, +0, m] 
	void lua_rawset (lua_State *L, int index);
	����lua_settable������ִ��һ��ԭ����ֵ��Ҳ���ǲ���Ԫ�������� 
*/
LUA_API void lua_rawset (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
  luaC_barriert(L, hvalue(t), L->top-1);
  L->top -= 2;
  lua_unlock(L);
}

/*
lua_rawseti
[-1, +0, m] 
	void lua_rawseti (lua_State *L, int index, int n);
	Does the equivalent of t[n] = v, where t is the value at the given valid index and v is the value at the top of the stack. 
	This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods. 

lua_rawseti
[-1, +0, m] 
	void lua_rawseti (lua_State *L, int index, int n);
	ִ��t[n] = v�ĵȼ۲���������t�Ǹ�������Ч��������ֵ��v��ջ����ֵ�� 
	��������ֵ��ջ����ֵ��ԭ���ģ���������Ԫ������ 
*/
LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
  luaC_barriert(L, hvalue(o), L->top-1);
  L->top--;
  lua_unlock(L);
}

/*
lua_setmetatable
[-1, +0, -] 
	int lua_setmetatable (lua_State *L, int index);
	Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index. 

lua_setmetatable
[-1, +0, -] 
	int lua_setmetatable (lua_State *L, int index);
	��ջ�е���һ����������Ϊ�������Ͽɵ���������ֵ����Ԫ�� 
*/
LUA_API int lua_setmetatable (lua_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checknelems(L, 1);
  obj = index2adr(L, objindex);
  api_checkvalidindex(L, obj);
  if (ttisnil(L->top - 1))
    mt = NULL;
  else {
    api_check(L, ttistable(L->top - 1));
    mt = hvalue(L->top - 1);
  }
  switch (ttype(obj)) {
    case LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarriert(L, hvalue(obj), mt);
      break;
    }
    case LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarrier(L, rawuvalue(obj), mt);
      break;
    }
    default: {
      G(L)->mt[ttype(obj)] = mt;
      break;
    }
  }
  L->top--;
  lua_unlock(L);
  return 1;
}

/*
lua_setfenv
[-1, +0, -] 
	int lua_setfenv (lua_State *L, int index);
	Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1. 

lua_setfenv
[-1, +0, -] 
	int lua_setfenv (lua_State *L, int index);
	��ջ�е���һ����������Ϊ������������ֵ���»��������������������ֵ�Ȳ��Ǻ����ֲ����߳�Ҳ�����û����ݣ�lua_setfenv����0�����򷵻�1�� 
*/
LUA_API int lua_setfenv (lua_State *L, int idx) {
  StkId o;
  int res = 1;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  api_check(L, ttistable(L->top - 1));
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      clvalue(o)->c.env = hvalue(L->top - 1);
      break;
    case LUA_TUSERDATA:
      uvalue(o)->env = hvalue(L->top - 1);
      break;
    case LUA_TTHREAD:
      sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
      break;
    default:
      res = 0;
      break;
  }
  if (res) luaC_objbarrier(L, gcvalue(o), hvalue(L->top - 1));
  L->top--;
  lua_unlock(L);
  return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))
	
/*
lua_call
[-(nargs + 1), +nresults, e] 
void lua_call (lua_State *L, int nargs, int nresults);
Calls a function. 
To call a function you must use the following protocol: first, the function to be called is pushed onto the stack; then, the arguments to the function are pushed in direct order; that is, the first argument is pushed first. Finally you call lua_call; nargs is the number of arguments that you pushed onto the stack. All arguments and the function value are popped from the stack when the function is called. The function results are pushed onto the stack when the function returns. The number of results is adjusted to nresults, unless nresults is LUA_MULTRET. In this case, all results from the function are pushed. Lua takes care that the returned values fit into the stack space. The function results are pushed onto the stack in direct order (the first result is pushed first), so that after the call the last result is on the top of the stack. 
Any error inside the called function is propagated upwards (with a longjmp). 
The following example shows how the host program can do the equivalent to this Lua code: 
     a = f("how", t.x, 14)
Here it is in C: 
     lua_getfield(L, LUA_GLOBALSINDEX, "f");  function to be called 
     lua_pushstring(L, "how");                         1st argument 
     lua_getfield(L, LUA_GLOBALSINDEX, "t");    table to be indexed 
     lua_getfield(L, -1, "x");         push result of t.x (2nd arg) 
     lua_remove(L, -2);                   remove 't' from the stack 
     lua_pushinteger(L, 14);                           3rd argument 
     lua_call(L, 3, 1);      call 'f' with 3 arguments and 1 result 
     lua_setfield(L, LUA_GLOBALSINDEX, "a");         set global 'a' 
Note that the code above is "balanced": at its end, the stack is back to its original configuration. This is considered good programming practice. 

lua_call
[-(nargs + 1), +nresults, e] 
void lua_call (lua_State *L, int nargs, int nresults);
����һ�������� 
Ҫ���ú�������ʹ�������Э�飺���ȣ���Ҫ�����õĺ���ѹջ��Ȼ�󣬽���������˳��ѹջ������һ��������ѹջ����󣬵���lua_call��nargs����ѹջ�Ĳ���������������������ʱ���в����Լ�����ֵ������ջ������������ʱ������ѹջ�����������Ϊnresults��������nresults��LUA_MULTRET���ڸ�����£����Ժ��������н����ѹջ��Lua��ȷ������ֵ�ʺ�ջ�ռ䡣���������˳��ѹջ����һ���������ѹջ����������ú����һ�������ջ���� 
�����ú����ڵ��κδ���ᱻ���ϴ�����ʹ��longjmp���� 
�����������ʾ������������ִ�����Lua����ȼ۵Ĳ����� 
     a = f("how", t.x, 14)
����C�еĴ��룺 
     lua_getfield(L, LUA_GLOBALSINDEX, "f");           Ҫ�����õĺ��� 
     lua_pushstring(L, "how");                              ��1������ 
     lua_getfield(L, LUA_GLOBALSINDEX, "t");             Ҫ�������ı� 
     lua_getfield(L, -1, "x");            ��t.x�Ľ��ѹջ����2�������� 
     lua_remove(L, -2);                   ��ջ��ɾ����t�� 
     lua_pushinteger(L, 14);                                ��3������ 
     lua_call(L, 3, 1);                ��3���������á�f��������1����� 
     lua_setfield(L, LUA_GLOBALSINDEX, "a");         ����ȫ�ֱ�����a�� 

ע������Ĵ����ǡ��ԳƵġ��������β��ջ�ص����ʼ���á�����һ�����õı��ϰ�ߡ� 
*/
LUA_API void lua_call (lua_State *L, int nargs, int nresults) {
  StkId func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  luaD_call(L, func, nresults);
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}

/*
lua_pcall
[-(nargs + 1), +(nresults|1), -] 
	int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);
	Calls a function in protected mode. 
	Both nargs and nresults have the same meaning as in lua_call. If there are no errors during the call, lua_pcall behaves exactly like lua_call. However, if there is any error, lua_pcall catches it, pushes a single value on the stack (the error message), and returns an error code. Like lua_call, lua_pcall always removes the function and its arguments from the stack. 
	If errfunc is 0, then the error message returned on the stack is exactly the original error message. Otherwise, errfunc is the stack index of an error handler function. (In the current implementation, this index cannot be a pseudo-index.) In case of runtime errors, this function will be called with the error message and its return value will be the message returned on the stack by lua_pcall. 
	Typically, the error handler function is used to add more debug information to the error message, such as a stack traceback. Such information cannot be gathered after the return of lua_pcall, since by then the stack has unwound. 
	The lua_pcall function returns 0 in case of success or one of the following error codes (defined in lua.h): 
	LUA_ERRRUN: a runtime error. 
	LUA_ERRMEM: memory allocation error. For such errors, Lua does not call the error handler function. 
	LUA_ERRERR: error while running the error handler function. 

lua_pcall
[-(nargs + 1), +(nresults|1), -]
int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc);
	�ڱ���ģʽ�е��ú����� 
	nargs��nresultsͬlua_call�е�����ͬ�ĺ��塣��������ڼ�û�д���
	lua_pcall����Ϊ��ȫ����lua_call��Ȼ������������κδ���lua_pcall��׽����
	������ֵѹջ��������Ϣ���������ش�����롣ͬlua_callһ����
	lua_pcall���Ǵ�ջ���Ƴ�������������� 
	���errfunc��0������ջ�Ϸ��صĴ�����Ϣ����ԭʼ�Ĵ�����Ϣ��
	����,errfunc���Ǵ�������������error handler function����ջ������
	����ǰʵ���У�������������α���������ڷ�������ʱ����ʱ���ú������ô�����Ϣ���ã�
	�����䷵��ֵ�����ջ�ϱ�lua_pcall���ص���Ϣ�� 
	���͵أ������������������������Ϣ�����Ӹ��������Ϣ������ջ���ݡ�
	������Ϣ������lua_pcall���غ��Ѽ�����Ϊ��ʱ��ջ���ͷš� 
	�ɹ�ʱlua_pcall��������0����������Ĵ������֮һ����lua.h�ж��壩�� 
	LUA_ERRRUN: ����ʱ���� 
	LUA_ERRMEM: �ڴ������󡣶����������Lua������ô������������� 
	LUA_ERRERR: �����д�����������ʱ�Ĵ��� 
*/
LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
  struct CCallS *c = cast(struct CCallS *, ud);
  Closure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->c.f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
  luaD_call(L, L->top - 2, 0);
}

/*
lua_cpcall
[-0, +(0|1), -] 
	int lua_cpcall (lua_State *L, lua_CFunction func, void *ud);
	Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded. 

lua_cpcall [-0, +(0|1), -]
	�Ա���ģʽ����C����func��
	func��ջ��ֻ��һ��Ԫ�أ��Ǹ�����ud���������û����ݡ�
	��������ʱ��lua_cpcall����ͬlua_pcallһ���Ĵ�����룬�Լ�ջ���Ĵ������
	���򷵻�0�Ҳ��ı�ջ��func���ص�����ֵ��������

ע������������ud��userdata��������C��������lua_touserdata()ȡ����
*/
LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0);
  lua_unlock(L);
  return status;
}

/*
lua_load
[-0, +1, -] 
int lua_load (lua_State *L,
              lua_Reader reader,
              void *data,
              const char *chunkname);
Loads a Lua chunk. If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack. Otherwise, it pushes an error message. The return values of lua_load are: 
0: no errors; 
LUA_ERRSYNTAX: syntax error during pre-compilation; 
LUA_ERRMEM: memory allocation error. 
This function only loads a chunk; it does not run it. 
lua_load automatically detects whether the chunk is text or binary, and loads it accordingly (see program luac). 
The lua_load function uses a user-supplied reader function to read the chunk (see lua_Reader). The data argument is an opaque value passed to the reader function. 
The chunkname argument gives a name to the chunk, which is used for error messages and in debug information (see ��3.8). 

lua_load
[-0, +1, -] 
int lua_load (lua_State *L,
              lua_Reader reader,
              void *data,
              const char *chunkname);
����һ��Lua�顣
���û�д���lua_load�ѱ���õĿ���Ϊһ��Lua����ѹ��ջ����
�������Ѵ�����Ϣѹ��ջ�С�
lua_load�ķ���ֵ�У�
0: �޴���; 
LUA_ERRSYNTAX: ��Ԥ����ʱ�����﷨���� 
LUA_ERRMEM: �ڴ�������
�������ֵ�ܼ���һ���飻�������������������
lua_load�Զ��������ı���ʽ���Ƕ����Ƹ�ʽ��������Ӧ���м��أ�������luac����
lua_load����ʹ��һ���û��ṩ�Ķ�������������飨��lua_Reader����
data������һ�����ݸ������������Ĳ�͸��ֵ��
chunkname�����������������ڴ�����Ϣ�Լ�������Ϣ������3.8���� 
*/
LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}

/*
lua_dump
[-0, +0, m] 
int lua_dump (lua_State *L, lua_Writer writer, void *data);
Dumps a function as a binary chunk. Receives a Lua function on the top of the stack and produces a binary chunk that, if loaded again, results in a function equivalent to the one dumped. As it produces parts of the chunk, lua_dump calls function writer (see lua_Writer) with the given data to write them. 
The value returned is the error code returned by the last call to the writer; 0 means no errors. 
This function does not pop the Lua function from the stack. 

lua_dump
[-0, +0, m] 
int lua_dump (lua_State *L, lua_Writer writer, void *data);
������ת��Ϊ�����ƴ��뵥Ԫ������ջ����Lua���������������Ƶ�Ԫ��������߱��ٴμ��أ��õ��뱻ת���ĵȼ۵ĺ�������������Ԫ�ĸ�����ʱ��lua_dump�ø�����data���ú���writer����lua_Writer����д�����ǡ� 
����ֵ�����һ�ε��ü�¼����writer�����صĴ�����룻0��ʾû�д��� 
���������ὫLua������ջ�е����� 
*/
LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data) {
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0);
  else
    status = 1;
  lua_unlock(L);
  return status;
}

/*
lua_status
[-0, +0, -] 
	int lua_status (lua_State *L);
	Returns the status of the thread L. 
	The status can be 0 for a normal thread, an error code if the thread finished its execution with an error, or LUA_YIELD if the thread is suspended. 

lua_status
[-0, +0, -] 
	int lua_status (lua_State *L);
	�����߳�L��״ֵ̬��
	�߳�����ʱ״ֵ̬Ϊ0���߳������������������ִ���򷵻�һ�������룬�̹߳�����ͣ���򷵻�LUA_YIELD��
*/
LUA_API int  lua_status (lua_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

/*
lua_gc
[-0, +0, e] 
	int lua_gc (lua_State *L, int what, int data);
	Controls the garbage collector. 
	This function performs several tasks, according to the value of the parameter what: 
	LUA_GCSTOP: stops the garbage collector. 
	LUA_GCRESTART: restarts the garbage collector. 
	LUA_GCCOLLECT: performs a full garbage-collection cycle. 
	LUA_GCCOUNT: returns the current amount of memory (in Kbytes) in use by Lua. 
	LUA_GCCOUNTB: returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024. 
	LUA_GCSTEP: performs an incremental step of garbage collection. The step "size" is controlled by data (larger values mean more steps) in a non-specified way. If you want to control the step size you must experimentally tune the value of data. The function returns 1 if the step finished a garbage-collection cycle. 
	LUA_GCSETPAUSE: sets data as the new value for the pause of the collector (see ��2.10). The function returns the previous value of the pause. 
	LUA_GCSETSTEPMUL: sets data as the new value for the step multiplier of the collector (see ��2.10). The function returns the previous value of the step multiplier. 

lua_gc[-0, +0, e]
	���������ռ����� 
		���������ݲ���what��ִֵ���������� 
		LUA_GCSTOP: ֹͣ�����ռ����� 
		LUA_GCRESTART: �������������ռ����� 
		LUA_GCCOLLECT: ִ��һ�������������ռ������ڡ� 
		LUA_GCCOUNT: ����Lua��ǰռ�õ��ڴ���������KbyteΪ��λ���� 
		LUA_GCCOUNTB: ����Lua��ǰ���ڴ��������ֽ�����1024�������� 
		LUA_GCSTEP: ִ��һ�����������ռ�������������data��δָ���ķ�ʽ���ƣ�ֵԽ����ζ�Ų���Խ�ࣩ��Ҫ���Ʋ����������ʵ���Եص���data��ֵ������ò������һ�������ռ�������������1�� 
		LUA_GCSETPAUSE: ����data��Ϊ�ռ�����pause������2.10������ֵ������pause��ǰһ��ֵ�� 
		LUA_GCSETSTEPMUL: ����data��Ϊ�ռ�����step multiplier������2.10������ֵ������step multiplier��ǰһ��ֵ�� 

*/
LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  global_State *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->GCthreshold = MAX_LUMEM;
      break;
    }
    case LUA_GCRESTART: {
      g->GCthreshold = g->totalbytes;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(g->totalbytes >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(g->totalbytes & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      lu_mem a = (cast(lu_mem, data) << 10);
      if (a <= g->totalbytes)
        g->GCthreshold = g->totalbytes - a;
      else
        g->GCthreshold = 0;
      while (g->GCthreshold <= g->totalbytes) {
        luaC_step(L);
        if (g->gcstate == GCSpause) {  /* end of cycle? */
          res = 1;  /* signal it */
          break;
        }
      }
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      g->gcstepmul = data;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/

/*
lua_error
[-1, +0, v] 
int lua_error (lua_State *L);
Generates a Lua error. The error message (which can actually be a Lua value of any type) must be on the stack top. This function does a long jump, and therefore never returns. (see luaL_error). 

lua_error
[-1, +0, v] 
int lua_error (lua_State *L);
����һ��Lua���󡣴�����Ϣ��ʵ���Ͽ�Ϊ�κ����͵�Luaֵ��������ջ����������ִ�г���ת����˴Ӳ����ء�����luaL_error���� 
*/
LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  api_checknelems(L, 1);
  luaG_errormsg(L);
  lua_unlock(L);
  return 0;  /* to avoid warnings */
}

/*
lua_next
[-1, +(2|0), e] 

int lua_next (lua_State *L, int index);
Pops a key from the stack, and pushes a key-value pair from the table at the given index (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 0 (and pushes nothing). 

A typical traversal looks like this: 

      table is in the stack at index 't' 
     lua_pushnil(L);   first key 
     while (lua_next(L, t) != 0) {
        uses 'key' (at index -2) and 'value' (at index -1) 
       printf("%s - %s\n",
              lua_typename(L, lua_type(L, -2)),
              lua_typename(L, lua_type(L, -1)));
        removes 'value'; keeps 'key' for next iteration 
       lua_pop(L, 1);
     }

While traversing a table, do not call lua_tolstring directly on a key, unless you know that the key is actually a string. Recall that lua_tolstring changes the value at the given index; this confuses the next call to lua_next. 

lua_next
[-1, +(2|0), e] 
int lua_next (lua_State *L, int index);
��ջ�е���һ���������Ӹ������������ı��е���һ����-ֵ�ԣ��������ġ���һ���ԣ����������û�и����Ԫ�أ���lua_next����0���Ҳ���ѹջ�κζ������� 
���͵ı����������������� 
      ����ջ�е�������t���� 
     lua_pushnil(L);   ��һ���� 
     while (lua_next(L, t) != 0) {
        ʹ�á�����������-2���͡�ֵ��(����-1�� 
       printf("%s - %s\n",
              lua_typename(L, lua_type(L, -2)),
              lua_typename(L, lua_type(L, -1)));
        �Ƴ���ֵ����Ϊ�´ε������������� 
       lua_pop(L, 1);
     }
��������ʱ����Ҫֱ�ӶԼ�����lua_tolstring��������֪����ȷʵ���ַ���������һ�£�lua_tolstring��ı������������ֵ����������һ�ε���lua_next�� 
*/
LUA_API int lua_next (lua_State *L, int idx) {
  StkId t;
  int more;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  more = luaH_next(L, hvalue(t), L->top - 1);
  if (more) {
    api_incr_top(L);
  }
  else  /* no more elements */
    L->top -= 1;  /* remove key */
  lua_unlock(L);
  return more;
}

/*
lua_concat
[-n, +1, e] 
	void lua_concat (lua_State *L, int n);
	Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua (see ��2.5.4). 

lua_concat
[-n, +1, e] 
	void lua_concat (lua_State *L, int n);
	����ջ����n��ֵ���������ǲ����������ջ�������n��1��
	�������ջ�ϵĵ���ֵ��������ʲôҲ�����������n��0������ǿ��ַ�����
	���Ӳ�������Lua�ĳ�������ִ�У�����2.5.4���� 
*/
LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
    luaV_concat(L, n, cast_int(L->top - L->base) - 1);
    L->top -= (n-1);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}

/*
lua_getallocf
[-0, +0, -] 
lua_Alloc lua_getallocf (lua_State *L, void **ud);
Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate. 

lua_getallocf
[-0, +0, -] 
lua_Alloc lua_getallocf (lua_State *L, void **ud);
���ظ���״̬�����ڴ���亯�������ud��ΪNULL��Lua��lua_newstate����Ĳ�͸��ָ�����*ud�� 
*/
LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}

/*
lua_setallocf
[-0, +0, -] 
	void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);
	Changes the allocator function of a given state to f with user data ud. 

lua_setallocf
[-0, +0, -] 
	void lua_setallocf (lua_State *L, lua_Alloc f, void *ud);
	�Ѹ���״̬���ķ������������ɴ��û�����ud��f�� 
*/
LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}

/*
lua_newuserdata
[-0, +1, m] 
	void *lua_newuserdata (lua_State *L, size_t size);
	This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address. 
	Userdata represent C values in Lua. A full userdata represents a block of memory. It is an object (like a table): you must create it, it can have its own metatable, and you can detect when it is being collected. A full userdata is only equal to itself (under raw equality). 
	When Lua collects a full userdata with a gc metamethod, Lua calls the metamethod and marks the userdata as finalized. When this userdata is collected again then Lua frees its corresponding memory. 

lua_newuserdata
[-0, +1, m] 
	void *lua_newuserdata (lua_State *L, size_t size);
	�����������µĸ����ߴ���ڴ�飬�Կ��ַ�ķ�ʽ���������û�����ѹջ�������ظõ�ַ�� 
	Lua�е��û����ݱ�ʾCֵ���������û����ݱ�ʾһ���ڴ档���Ǹ�������ͬ��������봴���������������Լ���Ԫ�����ҵ����ռ�ʱ�ܱ���⵽���������û�����ֻ�����Լ�������ԭʼ����ȱȽϣ��� 
	��Lua��gcԪ�����ռ��������û�����ʱ��Lua���ø�Ԫ���������û����ݱ��Ϊ��ɵġ������û������ٴα��ռ�ʱ��Lua�ͷ����Ӧ���ڴ档 
*/
LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  Udata *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = luaS_newudata(L, size, getcurrenv(L));
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return u + 1;
}




static const char *aux_upvalue (StkId fi, int n, TValue **val) {
  Closure *f;
  if (!ttisfunction(fi)) return NULL;
  f = clvalue(fi);
  if (f->c.isC) {
    if (!(1 <= n && n <= f->c.nupvalues)) return NULL;
    *val = &f->c.upvalue[n-1];
    return "";
  }
  else {
    Proto *p = f->l.p;
    if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
    *val = f->l.upvals[n-1]->v;
    return getstr(p->upvalues[n-1]);
  }
}

/*
lua_getupvalue
[-0, +(0|1), -] 
	const char *lua_getupvalue (lua_State *L, int funcindex, int n);
	Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.) 
	Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues. 

lua_getupvalue
[-0, +(0|1), -] 
	const char *lua_getupvalue (lua_State *L, int funcindex, int n);
	�õ��հ���һ��upvalue����Ϣ��������Lua������upvalue�Ǻ����õ��������������հ��ڵ��ⲿ�ֲ���������lua_getupvalue�õ�����n����upvalue������ֵѹջ������������funcindexָ��ջ�еıհ�����upvalueû���ض���˳����Ϊ���������������д�ͷ��β���ǻ�ģ���������������˳�򱻱�š��� 
	��������upvalue��������ʱ����NULL���Ҳ�ѹջ�κζ�����������C�������������ÿմ�""��Ϊ����upvalue�����֡� 
*/
LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  lua_lock(L);
  name = aux_upvalue(index2adr(L, funcindex), n, &val);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}

/*
lua_setupvalue
[-(0|1), +0, -] 
	const char *lua_setupvalue (lua_State *L, int funcindex, int n);
	Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue). 
	Returns NULL (and pops nothing) when the index is greater than the number of upvalues. 

lua_setupvalue
[-(0|1), +0, -] 
	const char *lua_setupvalue (lua_State *L, int funcindex, int n);
	���ñհ���upvalue��ֵ������ջ����ֵ����upvalue�����������֡���Ҳ��ֵ��ջ�е���������funcindex��nͬlua_getupvalue��һ������lua_getupvalue���� 
	����������upvalue������ʱ����NULL���Ҳ���ջ�κζ������� 
*/
LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  StkId fi;
  lua_lock(L);
  fi = index2adr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    luaC_barrier(L, clvalue(fi), L->top);
  }
  lua_unlock(L);
  return name;
}

