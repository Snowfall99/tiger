#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "helper.h"
#include "env.h"
#include "semant.h"

/*Lab4: Your implementation of lab4*/

typedef void *Tr_exp;
struct expty
{
	Tr_exp exp;
	Ty_ty ty;
};

//In Lab4, the first argument exp should always be **NULL**.
struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;

	e.exp = exp;
	e.ty = ty;

	return e;
}

Ty_ty actual_ty(Ty_ty ty)
{
	if (ty->kind == Ty_name)
	{
		return ty->u.name.ty;
	}
	else
	{
		return ty;
	}
}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind)
	{
	case A_simpleVar:
	{
		E_enventry x = S_look(venv, v->u.simple);
		if (x && x->kind == E_varEntry)
			return expTy(NULL, actual_ty(x->u.var.ty));
		else
		{
			EM_error(v->pos, "undefined variable %s", S_name(get_simplevar_sym(v)));
			return expTy(NULL, Ty_Int());
		}
	}
	case A_fieldVar:
	{
		struct expty e = transVar(venv, tenv, get_fieldvar_var(v));
		if (get_expty_kind(e) == Ty_record)
		{
			for (Ty_fieldList f = get_record_fieldlist(e); f != NULL; f = f->tail)
			{
				if (f->head->name == get_fieldvar_sym(v))
				{
					return expTy(NULL, actual_ty(f->head->ty));
				}
			}

			EM_error(get_fieldvar_var(v)->pos, "field %s does not exist", S_name(get_fieldvar_sym(v)));
			return expTy(NULL, Ty_Int());
		}
		else
		{
			EM_error(get_fieldvar_var(v)->pos, "not a record type");
			return expTy(NULL, Ty_Int());
		}
	}
	case A_subscriptVar:
	{
		struct expty variable = transVar(venv, tenv, get_subvar_var(v));
		struct expty subscript = transExp(venv, tenv, get_subvar_exp(v));
		if (get_expty_kind(variable) == Ty_array)
		{
			return expTy(NULL, actual_ty(get_array(variable)));
		}
		else
		{
			EM_error(get_subvar_var(v)->pos, "array type required");
			return expTy(NULL, Ty_Int());
		}

		if (get_expty_kind(subscript) != Ty_int)
		{
			EM_error(get_subvar_exp(v)->pos, "integer subscript required");
			return expTy(NULL, NULL);
		}
	}
	default:
	{
		EM_error(v->pos, "transVar error");
		return expTy(NULL, NULL);
	}
	}
}

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->kind)
	{
	case A_varExp:
		return transVar(venv, tenv, a->u.var);
	case A_nilExp:
		return expTy(NULL, Ty_Nil());
	case A_intExp:
		return expTy(NULL, Ty_Int());
	case A_stringExp:
		return expTy(NULL, Ty_String());
	case A_callExp:
	{
	}
	case A_opExp:
	{
		A_oper oper = get_opexp_oper(a);
		struct expty left = transExp(venv, tenv, get_opexp_left(a));
		struct expty right = transExp(venv, tenv, get_opexp_right(a));
		if (oper == A_plusOp || oper == A_minusOp || oper == A_timesOp || oper == A_divideOp)
		{
			if (left.ty->kind != Ty_int)
			{
				EM_error(get_opexp_leftpos(a), "integer required");
			}
			if (right.ty->kind != Ty_int)
			{
				EM_error(get_opexp_rightpos(a), "integer required");
			}
			return expTy(NULL, Ty_Int());
		}
		else if (oper == A_eqOp || oper == A_neqOp || oper == A_ltOp || oper == A_leOp || oper == A_gtOp || oper == A_geOp)
		{
			if (left.ty->kind != right.ty->kind)
			{
				EM_error(get_opexp_leftpos(a), "same type required");
			}
			return expTy(NULL, Ty_Int());
		}
		else
		{
			EM_error(get_opexp_leftpos(a), "wrong oper");
		}
	}
	case A_recordExp:
	{
		Ty_ty x = S_look(tenv, get_recordexp_typ(a));
		if (x == NULL)
		{
			EM_error(a->pos, "undefined type %s", S_name(get_recordexp_typ(a)));
			return expTy(NULL, Ty_Record(NULL));
		}
		return expTy(NULL, x);
	}
	case A_seqExp:
	{
		struct expty e;
		for (A_expList expList = get_seqexp_seq(a); expList != NULL; expList = expList->tail)
		{
			e = transExp(venv, tenv, expList->head);
		}
		return e;
	}
	case A_assignExp:
	{
		struct expty exp = transExp(venv, tenv, get_assexp_exp(a));
		struct expty var = transVar(venv, tenv, get_assexp_var(a));
		if (exp.ty != var.ty)
		{
			EM_error(a->pos, "unmatched assign exp");
		}

		if (get_assexp_var(a)->kind == A_simpleVar)
		{
			E_enventry x = S_look(venv, get_simplevar_sym(get_assexp_var(a)));
			if (x && x->kind == E_varEntry)
			{
				if (get_varentry_type(x)->kind == Ty_name)
				{
					EM_error(a->pos, "loop variable can't be assigned");
				}
			}
		}
		return expTy(NULL, Ty_Void());
	}
	case A_ifExp:
	{
		transExp(venv, tenv, get_ifexp_test(a));
		struct expty if_then = transExp(venv, tenv, get_ifexp_then(a));
		struct expty if_else = transExp(venv, tenv, get_ifexp_else(a));
		if (get_expty_kind(if_else) == Ty_nil)
		{
			if (get_expty_kind(if_then) != Ty_void)
			{
				EM_error(a->pos, "if-then exp's body must produce no value");
			}
			return expTy(NULL, Ty_Void());
		}
		else
		{
			if (if_else.ty != if_then.ty)
			{
				EM_error(a->pos, "then exp and else exp type mismatch");
			}
			return if_then;
		}
	}
	case A_whileExp:
	{
		transExp(venv, tenv, get_whileexp_test(a));
		struct expty e = transExp(venv, tenv, get_whileexp_body(a));
		if (get_expty_kind(e) != Ty_void)
		{
			EM_error(a->pos, "while body must produce no value");
		}
		return expTy(NULL, Ty_Void());
	}
	case A_forExp:
	{
		struct expty lo = transExp(venv, tenv, get_forexp_lo(a));
		struct expty hi = transExp(venv, tenv, get_forexp_hi(a));
		if (get_expty_kind(lo) != Ty_int)
		{
			EM_error(get_forexp_lo(a)->pos, "for exp's range type is not integer");
		}
		if (get_expty_kind(hi) != Ty_int)
		{
			EM_error(get_forexp_hi(a)->pos, "for exp's range type is not integer");
		}
		S_beginScope(venv);
		S_enter(venv, get_forexp_var(a), E_VarEntry(Ty_Name(get_forexp_var(a), Ty_Int())));
		struct expty body = transExp(venv, tenv, get_forexp_body(a));
		S_endScope(venv);
		if (get_expty_kind(body) != Ty_void)
		{
			EM_error(a->pos, "for body must produce no value");
		}
		return expTy(NULL, Ty_Void());
	}
	case A_breakExp:
	{
		return expTy(NULL, Ty_Void());
	}
	case A_letExp:
	{
		struct expty e;
		A_decList d;

		S_beginScope(venv);
		S_beginScope(tenv);
		for (d = get_letexp_decs(a); d; d = d->tail)
		{
			transDec(venv, tenv, d->head);
		}
		e = transExp(venv, tenv, get_letexp_body(a));
		S_endScope(tenv);
		S_endScope(venv);

		return e;
	}
	case A_arrayExp:
	{
		Ty_ty t = S_look(tenv, get_arrayexp_typ(a));
		struct expty e = transExp(venv, tenv, get_arrayexp_init(a));
		if (e.ty != actual_ty(t)->u.array)
		{
			EM_error(a->pos, "type mismatch");
		}
		return expTy(NULL, actual_ty(t));
	}
	default:
	{
		EM_error(a->pos, "transExp error");
		return expTy(NULL, NULL);
	}
	}
}

Ty_tyList makeFormalTyList(S_table tenv, A_fieldList fieldList)
{
	Ty_tyList head = NULL, iter = NULL;
	Ty_ty t;
	for (; fieldList != NULL; fieldList = fieldList->tail)
	{
		t = S_look(tenv, fieldList->head->typ);
		if (t == NULL)
		{
			EM_error(fieldList->head->pos, "undefined type %s", fieldList->head->typ);
			t = Ty_Int();
		}
		if (head == NULL)
		{
			head = iter = Ty_TyList(t, NULL);
		}
		else
		{
			iter->tail = Ty_TyList(t, NULL);
			iter = iter->tail;
		}
	}
	return head;
}

void transDec(S_table venv, S_table tenv, A_dec d)
{
	switch (d->kind)
	{
	case A_functionDec:
	{
		A_fundecList fl, fl2;
		A_fundec f, f2;
		Ty_ty resultTy;
		Ty_tyList formalTys;

		for (fl = get_funcdec_list(d); fl != NULL; fl = fl->tail)
		{
			f = fl->head;
			for (fl2 = get_funcdec_list(d); fl2 != fl; fl2 = fl2->tail)
			{
				f2 = fl2->head;
				if (f2->name == f->name)
				{
					EM_error(f->pos, "two functions have the same name");
					break;
				}
			}
			if (f->result == NULL)
			{
				resultTy = Ty_Void();
			}
			else
			{
				resultTy = S_look(tenv, f->result);
			}
			formalTys = makeFormalTyList(tenv, f->params);
			S_enter(venv, f->name, E_FunEntry(formalTys, resultTy));
		}
		for (fl = get_funcdec_list(d); fl != NULL; fl = fl->tail)
		{
			f = fl->head;
			E_enventry x = S_look(venv, f->name);
			S_beginScope(venv);
			A_fieldList params = f->params;
			Ty_tyList formals = get_func_tylist(x);
			for (; params != NULL; params = params->tail, formals = formals->tail)
			{
				S_enter(venv, params->head->name, E_VarEntry(formals->head));
			}
			struct expty e = transExp(venv, tenv, f->body);
			if (get_func_res(x)->kind == Ty_void && get_expty_kind(e) != Ty_void)
			{
				EM_error(f->pos, "procedure returns value");
			}
			S_endScope(venv);
		}
		break;
	}
	case A_varDec:
	{
		struct expty e = transExp(venv, tenv, get_vardec_init(d));
		if (get_vardec_typ(d) == NULL)
		{
			if (get_expty_kind(e) == Ty_nil)
			{
				EM_error(d->pos, "init should not be nil without type specified");
			}
		}
		else
		{
			Ty_ty t = S_look(tenv, get_vardec_typ(d));
			if (t == NULL)
			{
				EM_error(d->pos, "undefined type %s", S_name(get_vardec_typ(d)));
			}
			if (t != e.ty)
			{
				EM_error(d->pos, "type mismatch");
			}
		}
		S_enter(venv, get_vardec_var(d), E_VarEntry(e.ty));
		break;
	}
	case A_typeDec:
	{
		A_nametyList t, t2;
		for (t = get_typedec_list(d); t != NULL; t = t->tail)
		{
			for (t2 = get_typedec_list(d); t2 != t; t2 = t2->tail)
			{
				if (t2->head->name == t->head->name)
				{
					EM_error(d->pos, "two types have the same name");
					break;
				}
			}
			S_enter(tenv, t->head->name, Ty_Name(t->head->name, NULL));
		}
		for (t = get_typedec_list(d); t; t = t->tail)
		{
			Ty_ty ty = S_look(tenv, t->head->name);
			ty->u.name.ty = transTy(tenv, t->head->ty);
		}
		bool cycle = FALSE;
		for (t = get_typedec_list(d); t; t = t->tail)
		{
			Ty_ty ty = S_look(tenv, t->head->name);
			Ty_ty tmp = ty;
			while (tmp->kind == Ty_name)
			{
				tmp = tmp->u.name.ty;
				if (tmp == ty)
				{
					EM_error(d->pos, "illegal type cycle");
					cycle = TRUE;
					break;
				}
			}
			if (cycle)
				break;
		}
		break;
	}
	default:
	{
		EM_error(d->pos, "transDec error");
	}
	}
}

Ty_ty transTy(S_table tenv, A_ty a)
{
	switch (a->kind)
	{
	case A_recordTy:
	{
		A_fieldList f;
		Ty_fieldList tf = NULL;
		Ty_ty t;
		for (f = get_ty_record(a); f != NULL; f = f->tail)
		{
			t = S_look(tenv, f->head->typ);
			if (t == NULL)
			{
				EM_error(f->head->pos, "undefined type %s", S_name(f->head->typ));
				t = Ty_Int();
			}
			tf = Ty_FieldList(Ty_Field(f->head->name, t), tf);
		}
		return Ty_Record(tf);
	}
	case A_arrayTy:
	{
		Ty_ty t = S_look(tenv, get_ty_array(a));
		if (t != NULL)
		{
			return Ty_Array(t);
		}
		else
		{
			EM_error(a->pos, "undefined type %s", S_name(get_ty_array(a)));
			return Ty_Array(Ty_Int());
		}
	}
	case A_nameTy:
	{
		Ty_ty t = S_look(tenv, get_ty_name(a));
		if (t != NULL)
		{
			return t;
		}
		else
		{
			EM_error(a->pos, "undefined type %s", S_name(get_ty_name(a)));
			return Ty_Int();
		}
	}
	default:
	{
		EM_error(a->pos, "transTy error");
		return NULL;
	}
	}
}

void SEM_transProg(A_exp exp)
{
	S_table tenv = E_base_tenv();
	S_table venv = E_base_venv();
	transExp(venv, tenv, exp);
}