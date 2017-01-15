/*
** $Id: ldebug.c,v 2.29.1.6 2008/05/08 16:56:26 roberto Exp $
** Debug Interface
** See Copyright Notice in lua.h
*/


#include <stdarg.h>
#include <stddef.h>
#include <string.h>


#define ldebug_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"



static const char *getfuncname (lua_State *L, CallInfo *ci, const char **name);


static int currentpc (lua_State *L, CallInfo *ci) {
  if (!isLua(ci)) return -1;  /* function is not a Lua function? */
  if (ci == L->ci)
    ci->savedpc = L->savedpc;
  return pcRel(ci->savedpc, ci_func(ci)->l.p);
}


static int currentline (lua_State *L, CallInfo *ci) {
  int pc = currentpc(L, ci);
  if (pc < 0)
    return -1;  /* only active lua functions have current-line information */
  else
    return getline(ci_func(ci)->l.p, pc);
}


/*
lua_sethook
[-0, +0, -] 
	int lua_sethook (lua_State *L, lua_Hook f, int mask, int count);
	Sets the debugging hook function. 
	Argument f is the hook function. mask specifies on which events the hook will be called: it is formed by a bitwise or of the constants LUA_MASKCALL, LUA_MASKRET, LUA_MASKLINE, and LUA_MASKCOUNT. The count argument is only meaningful when the mask includes LUA_MASKCOUNT. For each event, the hook is called as explained below: 
	The call hook: is called when the interpreter calls a function. The hook is called just after Lua enters the new function, before the function gets its arguments. 
	The return hook: is called when the interpreter returns from a function. The hook is called just before Lua leaves the function. You have no access to the values to be returned by the function. 
	The line hook: is called when the interpreter is about to start the execution of a new line of code, or when it jumps back in the code (even to the same line). (This event only happens while Lua is executing a Lua function.) 
	The count hook: is called after the interpreter executes every count instructions. (This event only happens while Lua is executing a Lua function.) 
	A hook is disabled by setting mask to zero. 

lua_sethook
[-0, +0, -] 
	设置调试的hook函数。 
	参数f是hook函数。mask规定发生哪个事件时hook将被调用：
	它由常量LUA_MASKCALL、LUA_MASKRET、LUA_MASKLINE和LUA_MASKCOUNT按位或组成。
	只当掩码含有LUA_MASKCOUNT时，count参数才有意义。
	对于每个事件，hook如下面解释的那样被调用： 
	call hook: 当解释器调用函数时被调用。hook在Lua刚进入新函数后、
			在函数得到其参数前被调用。 
	return hook: 当解释器从函数返回时被调用。hook在Lua正要离开函数前被调用。
			你不能访问要被函数返回的值。 
	line hook: 当解释器将要开始新行代码的执行或跳回到代码中（甚至是同一行）时被调用。
			（该事件只在Lua正执行Lua函数时发生。） 
	count hook: 在解释器执行每count条指令后被调用。（该事件只在Lua正执行Lua函数时发生。） 
			通过设定mask为0禁用hook。
*/
/*
** this function can be called asynchronous (e.g. during a signal)
*/
LUA_API int lua_sethook (lua_State *L, lua_Hook func, int mask, int count) {
  if (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  return 1;
}

/*
lua_gethook
[-0, +0, -] 
	lua_Hook lua_gethook (lua_State *L);
	Returns the current hook function. 

lua_gethook
[-0, +0, -] 
	lua_Hook lua_gethook (lua_State *L);
	返回当前钩子函数。 
*/
LUA_API lua_Hook lua_gethook (lua_State *L) {
  return L->hook;
}

/*
lua_gethookmask
[-0, +0, -] 
	int lua_gethookmask (lua_State *L);
	Returns the current hook mask. 

lua_gethookmask
[-0, +0, -] 
	int lua_gethookmask (lua_State *L);
	返回当前钩子掩码。 
*/
LUA_API int lua_gethookmask (lua_State *L) {
  return L->hookmask;
}

/*
lua_gethookcount
[-0, +0, -] 
	int lua_gethookcount (lua_State *L);
	Returns the current hook count. 

lua_gethookcount
[-0, +0, -] 
	int lua_gethookcount (lua_State *L);
	返回当前钩子个数
*/
LUA_API int lua_gethookcount (lua_State *L) {
  return L->basehookcount;
}

/*
lua_getstack
[-0, +0, -] 
	int lua_getstack (lua_State *L, int level, lua_Debug *ar);
	Get information about the interpreter runtime stack. 
	This function fills parts of a lua_Debug structure with an identification of the activation record of the function executing at a given level. Level 0 is the current running function, whereas level n+1 is the function that has called level n. When there are no errors, lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0. 

lua_getstack
[-0, +0, -] 
	int lua_getstack (lua_State *L, int level, lua_Debug *ar);
	得到关于解释器运行时的栈信息。 
	本函数用执行于给定级别的函数的活动记录的标识符填充部分lua_Debug结构。0级是当前运行的函数，而n+1级是调用n级的函数。无错误时lua_getstack返回1；当以超过栈深度的级别调用时返回0。 
*/
LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar) {
  int status;
  CallInfo *ci;
  lua_lock(L);
  for (ci = L->ci; level > 0 && ci > L->base_ci; ci--) {
    level--;
    if (f_isLua(ci))  /* Lua function? */
      level -= ci->tailcalls;  /* skip lost tail calls */
  }
  if (level == 0 && ci > L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = cast_int(ci - L->base_ci);
  }
  else if (level < 0) {  /* level is of a lost tail call? */
    status = 1;
    ar->i_ci = 0;
  }
  else status = 0;  /* no such level */
  lua_unlock(L);
  return status;
}


static Proto *getluaproto (CallInfo *ci) {
  return (isLua(ci) ? ci_func(ci)->l.p : NULL);
}


static const char *findlocal (lua_State *L, CallInfo *ci, int n) {
  const char *name;
  Proto *fp = getluaproto(ci);
  if (fp && (name = luaF_getlocalname(fp, n, currentpc(L, ci))) != NULL)
    return name;  /* is a local variable in a Lua function */
  else {
    StkId limit = (ci == L->ci) ? L->top : (ci+1)->func;
    if (limit - ci->base >= n && n > 0)  /* is 'n' inside 'ci' stack? */
      return "(*temporary)";
    else
      return NULL;
  }
}

/*
lua_getlocal
[-0, +(0|1), -] 
	const char *lua_getlocal (lua_State *L, lua_Debug *ar, int n);
	Gets information about a local variable of a given activation record. The parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). The index n selects which local variable to inspect (1 is the first parameter or active local variable, and so on, until the last active local variable). lua_getlocal pushes the variable's value onto the stack and returns its name. 
	Variable names starting with '(' (open parentheses) represent internal variables (loop control variables, temporaries, and C function locals). 
	Returns NULL (and pushes nothing) when the index is greater than the number of active local variables. 

lua_getlocal
[-0, +(0|1), -] 
	const char *lua_getlocal (lua_State *L, lua_Debug *ar, int n);
	获得给定的活动记录的一个局部变量的信息。参数ar必须是有效的活动记录，它被之前的lua_getstack调用填充，或者作为hook的参数（见lua_Hook）。索引n选取要检阅哪个局部变量（1是第一个参数或活动的局部变量，以此类推，直到最后一个活动的局部变量）。lua_getlocal将变量的值压栈并返回其名字。 
	‘(’（开圆括号）开头的变量表示内部变量（循环控制变量、临时变量和C函数局部变量）。 
	当索引比活动的局部变量的数量大时返回NULL（且不压栈任何东西）。 
*/
LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n) {
  CallInfo *ci = L->base_ci + ar->i_ci;
  const char *name = findlocal(L, ci, n);
  lua_lock(L);
  if (name)
      luaA_pushobject(L, ci->base + (n - 1));
  lua_unlock(L);
  return name;
}

/*
lua_setlocal
[-(0|1), +0, -] 
	const char *lua_setlocal (lua_State *L, lua_Debug *ar, int n);
	Sets the value of a local variable of a given activation record. Parameters ar and n are as in lua_getlocal (see lua_getlocal). lua_setlocal assigns the value at the top of the stack to the variable and returns its name. It also pops the value from the stack. 
	Returns NULL (and pops nothing) when the index is greater than the number of active local variables. 

lua_setlocal
[-(0|1), +0, -] 
	const char *lua_setlocal (lua_State *L, lua_Debug *ar, int n);
	设置给定的活动记录的一个局部变量的值。参数ar和n与lua_getlocal中一样（见lua_getlocal）。lua_setlocal把栈顶的值赋给变量并返回其名字。它也将值从栈中弹出。 
	当索引超过活动的局部变量的数量时返回NULL（且不出栈任何东西）。 
*/
LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n) {
  CallInfo *ci = L->base_ci + ar->i_ci;
  const char *name = findlocal(L, ci, n);
  lua_lock(L);
  if (name)
      setobjs2s(L, ci->base + (n - 1), L->top - 1);
  L->top--;  /* pop value */
  lua_unlock(L);
  return name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  if (cl->c.isC) {
    ar->source = "=[C]";
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  else {
    ar->source = getstr(cl->l.p->source);
    ar->linedefined = cl->l.p->linedefined;
    ar->lastlinedefined = cl->l.p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
}


static void info_tailcall (lua_Debug *ar) {
  ar->name = ar->namewhat = "";
  ar->what = "tail";
  ar->lastlinedefined = ar->linedefined = ar->currentline = -1;
  ar->source = "=(tail call)";
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
  ar->nups = 0;
}


static void collectvalidlines (lua_State *L, Closure *f) {
  if (f == NULL || f->c.isC) {
    setnilvalue(L->top);
  }
  else {
    Table *t = luaH_new(L, 0, 0);
    int *lineinfo = f->l.p->lineinfo;
    int i;
    for (i=0; i<f->l.p->sizelineinfo; i++)
      setbvalue(luaH_setnum(L, t, lineinfo[i]), 1);
    sethvalue(L, L->top, t); 
  }
  incr_top(L);
}


static int auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                    Closure *f, CallInfo *ci) {
  int status = 1;
  if (f == NULL) {
    info_tailcall(ar);
    return status;
  }
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci) ? currentline(L, ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = f->c.nupvalues;
        break;
      }
      case 'n': {
        ar->namewhat = (ci) ? getfuncname(L, ci, &ar->name) : NULL;
        if (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        break;
      }
      case 'L':
      case 'f':  /* handled by lua_getinfo */
        break;
      default: status = 0;  /* invalid option */
    }
  }
  return status;
}

/*
lua_getinfo
[-(0|1), +(0|1|2), m] 
	int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
	Returns information about a specific function or function invocation. 
	To get information about a function invocation, the parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). 
	To get information about a function you push it onto the stack and start the what string with the character '>'. (In that case, lua_getinfo pops the function in the top of the stack.) For instance, to know in which line a function f was defined, you can write the following code: 
		 lua_Debug ar;
		 lua_getfield(L, LUA_GLOBALSINDEX, "f");   get global 'f' 
		 lua_getinfo(L, ">S", &ar);
		 printf("%d\n", ar.linedefined);
	Each character in the string what selects some fields of the structure ar to be filled or a value to be pushed on the stack: 
	'n': fills in the field name and namewhat; 
	'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what; 
	'l': fills in the field currentline; 
	'u': fills in the field nups; 
	'f': pushes onto the stack the function that is running at the given level; 
	'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid on the function. (A valid line is a line with some associated code, that is, a line where you can put a break point. Non-valid lines include empty lines and comments.) 
	This function returns 0 on error (for instance, an invalid option in what). 

lua_getinfo
[-(0|1), +(0|1|2), m] 
	int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar);
	返回关于特定函数或函数调用的信息。
	要获得函数调用的信息，参数ar必须是被前一个调用lua_getstack填充的合法的激活记录，或者是作为参数传给hook（参考lua_Hook）。
	为了获得函数信息，你可以把它压入堆栈然后启动带字符'>'开头的字符串what。
	（这种情况下，lua_getinfo会弹出栈顶的函数。）
	例如，为了指导哪一行定义了函数f，你可以写如下代码：
	lua_Debug ar;
	lua_getfield(L, LUA_GLOBALSINDEX, "f");   获取全局'f' 
	lua_getinfo(L, ">S", &ar);
	printf("%d\n", ar.linedefined);
	字符串what的每个字符选择了结构体ar要填充的域或者是压入堆栈的值：
	'n': 填充域name和namewhat； 
	'S': 填充域source, short_src, linedefined, lastlinedefined, 和what； 
	'l': 填充域currentline； 
	'u': 填充域nups；
	'f': 把运行于所给层的函数压入栈顶； 
	'L': 把一个索引是函数合法行号的表压入堆栈（一个合法行是带有关联代码的行，所以一行中可以放置一个断点。不合法的行包括空行和注释。）
	这个函数在错误时返回0（例如，what的一个不合法选项）
*/
LUA_API int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  int status;
  Closure *f = NULL;
  CallInfo *ci = NULL;
  lua_lock(L);
  if (*what == '>') {
    StkId func = L->top - 1;
    luai_apicheck(L, ttisfunction(func));
    what++;  /* skip the '>' */
    f = clvalue(func);
    L->top--;  /* pop function */
  }
  else if (ar->i_ci != 0) {  /* no tail call? */
    ci = L->base_ci + ar->i_ci;
    lua_assert(ttisfunction(ci->func));
    f = clvalue(ci->func);
  }
  status = auxgetinfo(L, what, ar, f, ci);
  if (strchr(what, 'f')) {
    if (f == NULL) setnilvalue(L->top);
    else setclvalue(L, L->top, f);
    incr_top(L);
  }
  if (strchr(what, 'L'))
    collectvalidlines(L, f);
  lua_unlock(L);
  return status;
}


/*
** {======================================================
** Symbolic Execution and code checker
** =======================================================
*/

#define check(x)		if (!(x)) return 0;

#define checkjump(pt,pc)	check(0 <= pc && pc < pt->sizecode)

#define checkreg(pt,reg)	check((reg) < (pt)->maxstacksize)



static int precheck (const Proto *pt) {
  check(pt->maxstacksize <= MAXSTACK);
  check(pt->numparams+(pt->is_vararg & VARARG_HASARG) <= pt->maxstacksize);
  check(!(pt->is_vararg & VARARG_NEEDSARG) ||
              (pt->is_vararg & VARARG_HASARG));
  check(pt->sizeupvalues <= pt->nups);
  check(pt->sizelineinfo == pt->sizecode || pt->sizelineinfo == 0);
  check(pt->sizecode > 0 && GET_OPCODE(pt->code[pt->sizecode-1]) == OP_RETURN);
  return 1;
}


#define checkopenop(pt,pc)	luaG_checkopenop((pt)->code[(pc)+1])

int luaG_checkopenop (Instruction i) {
  switch (GET_OPCODE(i)) {
    case OP_CALL:
    case OP_TAILCALL:
    case OP_RETURN:
    case OP_SETLIST: {
      check(GETARG_B(i) == 0);
      return 1;
    }
    default: return 0;  /* invalid instruction after an open call */
  }
}


static int checkArgMode (const Proto *pt, int r, enum OpArgMask mode) {
  switch (mode) {
    case OpArgN: check(r == 0); break;
    case OpArgU: break;
    case OpArgR: checkreg(pt, r); break;
    case OpArgK:
      check(ISK(r) ? INDEXK(r) < pt->sizek : r < pt->maxstacksize);
      break;
  }
  return 1;
}


static Instruction symbexec (const Proto *pt, int lastpc, int reg) {
  int pc;
  int last;  /* stores position of last instruction that changed `reg' */
  last = pt->sizecode-1;  /* points to final return (a `neutral' instruction) */
  check(precheck(pt));
  for (pc = 0; pc < lastpc; pc++) {
    Instruction i = pt->code[pc];
    OpCode op = GET_OPCODE(i);
    int a = GETARG_A(i);
    int b = 0;
    int c = 0;
    check(op < NUM_OPCODES);
    checkreg(pt, a);
    switch (getOpMode(op)) {
      case iABC: {
        b = GETARG_B(i);
        c = GETARG_C(i);
        check(checkArgMode(pt, b, getBMode(op)));
        check(checkArgMode(pt, c, getCMode(op)));
        break;
      }
      case iABx: {
        b = GETARG_Bx(i);
        if (getBMode(op) == OpArgK) check(b < pt->sizek);
        break;
      }
      case iAsBx: {
        b = GETARG_sBx(i);
        if (getBMode(op) == OpArgR) {
          int dest = pc+1+b;
          check(0 <= dest && dest < pt->sizecode);
          if (dest > 0) {
            int j;
            /* check that it does not jump to a setlist count; this
               is tricky, because the count from a previous setlist may
               have the same value of an invalid setlist; so, we must
               go all the way back to the first of them (if any) */
            for (j = 0; j < dest; j++) {
              Instruction d = pt->code[dest-1-j];
              if (!(GET_OPCODE(d) == OP_SETLIST && GETARG_C(d) == 0)) break;
            }
            /* if 'j' is even, previous value is not a setlist (even if
               it looks like one) */
            check((j&1) == 0);
          }
        }
        break;
      }
    }
    if (testAMode(op)) {
      if (a == reg) last = pc;  /* change register `a' */
    }
    if (testTMode(op)) {
      check(pc+2 < pt->sizecode);  /* check skip */
      check(GET_OPCODE(pt->code[pc+1]) == OP_JMP);
    }
    switch (op) {
      case OP_LOADBOOL: {
        if (c == 1) {  /* does it jump? */
          check(pc+2 < pt->sizecode);  /* check its jump */
          check(GET_OPCODE(pt->code[pc+1]) != OP_SETLIST ||
                GETARG_C(pt->code[pc+1]) != 0);
        }
        break;
      }
      case OP_LOADNIL: {
        if (a <= reg && reg <= b)
          last = pc;  /* set registers from `a' to `b' */
        break;
      }
      case OP_GETUPVAL:
      case OP_SETUPVAL: {
        check(b < pt->nups);
        break;
      }
      case OP_GETGLOBAL:
      case OP_SETGLOBAL: {
        check(ttisstring(&pt->k[b]));
        break;
      }
      case OP_SELF: {
        checkreg(pt, a+1);
        if (reg == a+1) last = pc;
        break;
      }
      case OP_CONCAT: {
        check(b < c);  /* at least two operands */
        break;
      }
      case OP_TFORLOOP: {
        check(c >= 1);  /* at least one result (control variable) */
        checkreg(pt, a+2+c);  /* space for results */
        if (reg >= a+2) last = pc;  /* affect all regs above its base */
        break;
      }
      case OP_FORLOOP:
      case OP_FORPREP:
        checkreg(pt, a+3);
        /* go through */
      case OP_JMP: {
        int dest = pc+1+b;
        /* not full check and jump is forward and do not skip `lastpc'? */
        if (reg != NO_REG && pc < dest && dest <= lastpc)
          pc += b;  /* do the jump */
        break;
      }
      case OP_CALL:
      case OP_TAILCALL: {
        if (b != 0) {
          checkreg(pt, a+b-1);
        }
        c--;  /* c = num. returns */
        if (c == LUA_MULTRET) {
          check(checkopenop(pt, pc));
        }
        else if (c != 0)
          checkreg(pt, a+c-1);
        if (reg >= a) last = pc;  /* affect all registers above base */
        break;
      }
      case OP_RETURN: {
        b--;  /* b = num. returns */
        if (b > 0) checkreg(pt, a+b-1);
        break;
      }
      case OP_SETLIST: {
        if (b > 0) checkreg(pt, a + b);
        if (c == 0) {
          pc++;
          check(pc < pt->sizecode - 1);
        }
        break;
      }
      case OP_CLOSURE: {
        int nup, j;
        check(b < pt->sizep);
        nup = pt->p[b]->nups;
        check(pc + nup < pt->sizecode);
        for (j = 1; j <= nup; j++) {
          OpCode op1 = GET_OPCODE(pt->code[pc + j]);
          check(op1 == OP_GETUPVAL || op1 == OP_MOVE);
        }
        if (reg != NO_REG)  /* tracing? */
          pc += nup;  /* do not 'execute' these pseudo-instructions */
        break;
      }
      case OP_VARARG: {
        check((pt->is_vararg & VARARG_ISVARARG) &&
             !(pt->is_vararg & VARARG_NEEDSARG));
        b--;
        if (b == LUA_MULTRET) check(checkopenop(pt, pc));
        checkreg(pt, a+b-1);
        break;
      }
      default: break;
    }
  }
  return pt->code[last];
}

#undef check
#undef checkjump
#undef checkreg

/* }====================================================== */


int luaG_checkcode (const Proto *pt) {
  return (symbexec(pt, pt->sizecode, NO_REG) != 0);
}


static const char *kname (Proto *p, int c) {
  if (ISK(c) && ttisstring(&p->k[INDEXK(c)]))
    return svalue(&p->k[INDEXK(c)]);
  else
    return "?";
}


static const char *getobjname (lua_State *L, CallInfo *ci, int stackpos,
                               const char **name) {
  if (isLua(ci)) {  /* a Lua function? */
    Proto *p = ci_func(ci)->l.p;
    int pc = currentpc(L, ci);
    Instruction i;
    *name = luaF_getlocalname(p, stackpos+1, pc);
    if (*name)  /* is a local? */
      return "local";
    i = symbexec(p, pc, stackpos);  /* try symbolic execution */
    lua_assert(pc != -1);
    switch (GET_OPCODE(i)) {
      case OP_GETGLOBAL: {
        int g = GETARG_Bx(i);  /* global index */
        lua_assert(ttisstring(&p->k[g]));
        *name = svalue(&p->k[g]);
        return "global";
      }
      case OP_MOVE: {
        int a = GETARG_A(i);
        int b = GETARG_B(i);  /* move from `b' to `a' */
        if (b < a)
          return getobjname(L, ci, b, name);  /* get name for `b' */
        break;
      }
      case OP_GETTABLE: {
        int k = GETARG_C(i);  /* key index */
        *name = kname(p, k);
        return "field";
      }
      case OP_GETUPVAL: {
        int u = GETARG_B(i);  /* upvalue index */
        *name = p->upvalues ? getstr(p->upvalues[u]) : "?";
        return "upvalue";
      }
      case OP_SELF: {
        int k = GETARG_C(i);  /* key index */
        *name = kname(p, k);
        return "method";
      }
      default: break;
    }
  }
  return NULL;  /* no useful name found */
}


static const char *getfuncname (lua_State *L, CallInfo *ci, const char **name) {
  Instruction i;
  if ((isLua(ci) && ci->tailcalls > 0) || !isLua(ci - 1))
    return NULL;  /* calling function is not Lua (or is unknown) */
  ci--;  /* calling function */
  i = ci_func(ci)->l.p->code[currentpc(L, ci)];
  if (GET_OPCODE(i) == OP_CALL || GET_OPCODE(i) == OP_TAILCALL ||
      GET_OPCODE(i) == OP_TFORLOOP)
    return getobjname(L, ci, GETARG_A(i), name);
  else
    return NULL;  /* no useful name can be found */
}


/* only ANSI way to check whether a pointer points to an array */
static int isinstack (CallInfo *ci, const TValue *o) {
  StkId p;
  for (p = ci->base; p < ci->top; p++)
    if (o == p) return 1;
  return 0;
}


void luaG_typeerror (lua_State *L, const TValue *o, const char *op) {
  const char *name = NULL;
  const char *t = luaT_typenames[ttype(o)];
  const char *kind = (isinstack(L->ci, o)) ?
                         getobjname(L, L->ci, cast_int(o - L->base), &name) :
                         NULL;
  if (kind)
    luaG_runerror(L, "attempt to %s %s " LUA_QS " (a %s value)",
                op, kind, name, t);
  else
    luaG_runerror(L, "attempt to %s a %s value", op, t);
}


void luaG_concaterror (lua_State *L, StkId p1, StkId p2) {
  if (ttisstring(p1) || ttisnumber(p1)) p1 = p2;
  lua_assert(!ttisstring(p1) && !ttisnumber(p1));
  luaG_typeerror(L, p1, "concatenate");
}


void luaG_aritherror (lua_State *L, const TValue *p1, const TValue *p2) {
  TValue temp;
  if (luaV_tonumber(p1, &temp) == NULL)
    p2 = p1;  /* first operand is wrong */
  luaG_typeerror(L, p2, "perform arithmetic on");
}


int luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const char *t1 = luaT_typenames[ttype(p1)];
  const char *t2 = luaT_typenames[ttype(p2)];
  if (t1[2] == t2[2])
    luaG_runerror(L, "attempt to compare two %s values", t1);
  else
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
  return 0;
}


static void addinfo (lua_State *L, const char *msg) {
  CallInfo *ci = L->ci;
  if (isLua(ci)) {  /* is Lua code? */
    char buff[LUA_IDSIZE];  /* add file:line information */
    int line = currentline(L, ci);
    luaO_chunkid(buff, getstr(getluaproto(ci)->source), LUA_IDSIZE);
    luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


void luaG_errormsg (lua_State *L) {
  if (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    if (!ttisfunction(errfunc)) luaD_throw(L, LUA_ERRERR);
    setobjs2s(L, L->top, L->top - 1);  /* move argument */
    setobjs2s(L, L->top - 1, errfunc);  /* push function */
    incr_top(L);
    luaD_call(L, L->top - 2, 1);  /* call it */
  }
  luaD_throw(L, LUA_ERRRUN);
}


void luaG_runerror (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  addinfo(L, luaO_pushvfstring(L, fmt, argp));
  va_end(argp);
  luaG_errormsg(L);
}

