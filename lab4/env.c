#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "env.h"

/*Lab4: Your implementation of lab4*/

E_enventry E_VarEntry(Ty_ty ty)
{
	E_enventry x = checked_malloc(sizeof(*x));
	x->kind = E_varEntry;
	x->u.var.ty = ty;
	return x;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
	E_enventry x = checked_malloc(sizeof(*x));
	x->kind = E_funEntry;
	x->u.fun.formals = formals;
	x->u.fun.result = result;
	return x;
}

S_table E_base_tenv(void)
{
	S_table t = S_empty();
	S_enter(t, S_Symbol("int"), Ty_Int());
	S_enter(t, S_Symbol("string"), Ty_String());
	return t;
}

S_table E_base_venv(void)
{
	S_table t = S_empty();
	return t;
}
