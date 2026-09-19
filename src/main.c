#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { EOF_T, NUM, ID, INT, RETURN, IF, ELSE, WHILE, ADD, SUB, MUL, DIV, ASSIGN, EQ, NE, LT, LE, GT, GE, LP, RP, LB, RB, SEMI } Kind;
typedef struct { Kind k; long v; const char *s; size_t n; } Tok;
typedef struct { const char *s; size_t p; Tok t; } Parser;
typedef enum { E_NUM, E_VAR, E_BIN, E_NEG, E_ASSIGN } EK;
typedef struct Expr Expr;
struct Expr { EK k; int op; long v; char *name; Expr *a,*b; };
typedef enum { S_RETURN, S_EXPR, S_DECL, S_IF, S_WHILE, S_BLOCK } SK;
typedef struct Stmt Stmt;
struct Stmt { SK k; Expr *e,*cond; char *name; Stmt *a,*b; Stmt **items; size_t n; int off; };
typedef struct { char *name; int off; } Sym;
static Sym *syms; static size_t nsym, capsym; static int stack_bytes, label_id;
static void die(const char *m){fprintf(stderr,"error: %s\n",m);exit(1);}
static void diep(Parser *p,const char *m){fprintf(stderr,"error near byte %zu: %s\n",p->p,m);exit(1);}
static void *xmalloc(size_t n){void *p=malloc(n?n:1);if(!p)die("out of memory");return p;}
static char *xstr(const char*s,size_t n){char*p=xmalloc(n+1);memcpy(p,s,n);p[n]=0;return p;}
static Tok lex(Parser*p){
  const char*s=p->s; size_t i=p->p;
  for(;;){while(isspace((unsigned char)s[i]))i++; if(s[i]=='/'&&s[i+1]=='/'){while(s[i]&&s[i]!='\n')i++;}else break;}
  const char*st=s+i; char c=s[i]; if(!c){p->p=i;return(Tok){EOF_T,0,st,0};}
  if(isdigit((unsigned char)c)){char*e;long v=strtol(s+i,&e,10);p->p=(size_t)(e-s);return(Tok){NUM,v,st,(size_t)(e-st)};}
  if(isalpha((unsigned char)c)||c=='_'){i++;while(isalnum((unsigned char)s[i])||s[i]=='_')i++;size_t n=i-(size_t)(st-s);
    Kind k=ID; if(n==3&&!strncmp(st,"int",3))k=INT; else if(n==6&&!strncmp(st,"return",6))k=RETURN; else if(n==2&&!strncmp(st,"if",2))k=IF; else if(n==4&&!strncmp(st,"else",4))k=ELSE; else if(n==5&&!strncmp(st,"while",5))k=WHILE;
    p->p=i;return(Tok){k,0,st,n}; }
  p->p=i+1;
  if(c=='='&&s[i+1]=='='){p->p++;return(Tok){EQ,0,st,2};} if(c=='!'&&s[i+1]=='='){p->p++;return(Tok){NE,0,st,2};}
  if(c=='<'&&s[i+1]=='='){p->p++;return(Tok){LE,0,st,2};} if(c=='>'&&s[i+1]=='='){p->p++;return(Tok){GE,0,st,2};}
  Kind k; switch(c){case'+':k=ADD;break;case'-':k=SUB;break;case'*':k=MUL;break;case'/':k=DIV;break;case'=':k=ASSIGN;break;case'<':k=LT;break;case'>':k=GT;break;case'(':k=LP;break;case')':k=RP;break;case'{':k=LB;break;case'}':k=RB;break;case';':k=SEMI;break;default:diep(p,"unexpected character");} return(Tok){k,0,st,1};
}
static void next(Parser*p){p->t=lex(p);} static int take(Parser*p,Kind k){if(p->t.k!=k)return 0;next(p);return 1;} static void want(Parser*p,Kind k){if(!take(p,k))diep(p,"unexpected token");}
static Expr* ex(EK k){Expr*e=xmalloc(sizeof*e);memset(e,0,sizeof*e);e->k=k;return e;} static Expr*num(long v){Expr*e=ex(E_NUM);e->v=v;return e;} static Expr*var(char*n){Expr*e=ex(E_VAR);e->name=n;return e;}
static Expr*expr(Parser*p); static Stmt*stmt(Parser*p);
static Expr*primary(Parser*p){if(p->t.k==NUM){long v=p->t.v;next(p);return num(v);} if(p->t.k==ID){char*n=xstr(p->t.s,p->t.n);next(p);return var(n);} if(take(p,LP)){Expr*e=expr(p);want(p,RP);return e;} diep(p,"expected expression");return 0;}
static Expr*unary(Parser*p){if(take(p,SUB)){Expr*e=ex(E_NEG);e->a=unary(p);return e;}return primary(p);}
static Expr*mul(Parser*p){Expr*e=unary(p);while(p->t.k==MUL||p->t.k==DIV){int op=p->t.k;next(p);Expr*r=unary(p);Expr*n=ex(E_BIN);n->op=op;n->a=e;n->b=r;e=n;}return e;}
static Expr*add(Parser*p){Expr*e=mul(p);while(p->t.k==ADD||p->t.k==SUB){int op=p->t.k;next(p);Expr*r=mul(p);Expr*n=ex(E_BIN);n->op=op;n->a=e;n->b=r;e=n;}return e;}
static Expr*cmp(Parser*p){Expr*e=add(p);while(p->t.k>=LT&&p->t.k<=GE){int op=p->t.k;next(p);Expr*r=add(p);Expr*n=ex(E_BIN);n->op=op;n->a=e;n->b=r;e=n;}return e;}
static Expr*eq(Parser*p){Expr*e=cmp(p);while(p->t.k==EQ||p->t.k==NE){int op=p->t.k;next(p);Expr*r=cmp(p);Expr*n=ex(E_BIN);n->op=op;n->a=e;n->b=r;e=n;}return e;}
static Expr*expr(Parser*p){Expr*e=eq(p);if(take(p,ASSIGN)){if(e->k!=E_VAR)diep(p,"assignment target must be a variable");Expr*n=ex(E_ASSIGN);n->name=xstr(e->name,strlen(e->name));n->a=expr(p);e=n;}return e;}
static Stmt*st(SK k){Stmt*s=xmalloc(sizeof*s);memset(s,0,sizeof*s);s->k=k;return s;}
static Stmt*block(Parser*p){Stmt*s=st(S_BLOCK);want(p,LB);while(p->t.k!=RB&&p->t.k!=EOF_T){s->items=realloc(s->items,(s->n+1)*sizeof(*s->items));s->items[s->n++]=stmt(p);}want(p,RB);return s;}
static Stmt*stmt(Parser*p){
  if(take(p,INT)){Stmt*s=st(S_DECL);if(p->t.k!=ID)diep(p,"expected variable name");s->name=xstr(p->t.s,p->t.n);next(p);if(take(p,ASSIGN))s->e=expr(p);want(p,SEMI);return s;}
  if(take(p,RETURN)){Stmt*s=st(S_RETURN);s->e=expr(p);want(p,SEMI);return s;}
  if(take(p,IF)){Stmt*s=st(S_IF);want(p,LP);s->cond=expr(p);want(p,RP);s->a=stmt(p);if(take(p,ELSE))s->b=stmt(p);return s;}
  if(take(p,WHILE)){Stmt*s=st(S_WHILE);want(p,LP);s->cond=expr(p);want(p,RP);s->a=stmt(p);return s;}
  if(p->t.k==LB)return block(p);
  Stmt*s=st(S_EXPR);s->e=expr(p);want(p,SEMI);return s;
}
static int find(const char*n){for(size_t i=0;i<nsym;i++)if(!strcmp(syms[i].name,n))return syms[i].off;return 0;}
static int declare(const char*n){if(find(n))die("duplicate local variable");if(nsym==capsym){capsym=capsym?capsym*2:16;syms=realloc(syms,capsym*sizeof*syms);}stack_bytes+=8;syms[nsym].name=xstr(n,strlen(n));syms[nsym].off=stack_bytes;nsym++;return stack_bytes;}
static void resolve_expr(Expr*e){if(!e)return;if(e->k==E_ASSIGN){if(!find(e->name))die("assignment to undeclared variable");resolve_expr(e->a);return;}if(e->k==E_VAR){if(!find(e->name))die("use of undeclared variable");return;}resolve_expr(e->a);resolve_expr(e->b);}
static void resolve(Stmt*s){if(!s)return;switch(s->k){case S_DECL:s->off=declare(s->name);resolve_expr(s->e);break;case S_RETURN:case S_EXPR:resolve_expr(s->e);break;case S_IF:resolve_expr(s->cond);resolve(s->a);resolve(s->b);break;case S_WHILE:resolve_expr(s->cond);resolve(s->a);break;case S_BLOCK:for(size_t i=0;i<s->n;i++)resolve(s->items[i]);break;}}
static void emit_expr(Expr*e){
  if(e->k==E_NUM){printf("  mov $%ld, %%rax\n",e->v);return;} if(e->k==E_VAR){printf("  mov -%d(%%rbp), %%rax\n",find(e->name));return;} if(e->k==E_ASSIGN){emit_expr(e->a);printf("  mov %%rax, -%d(%%rbp)\n",find(e->name));return;} if(e->k==E_NEG){emit_expr(e->a);puts("  neg %rax");return;}
  emit_expr(e->b);puts("  push %rax");emit_expr(e->a);puts("  pop %rcx");
  switch(e->op){case ADD:puts("  add %rcx, %rax");break;case SUB:puts("  sub %rcx, %rax");break;case MUL:puts("  imul %rcx, %rax");break;case DIV:puts("  cqo\n  idiv %rcx");break;
    default:puts("  cmp %rcx, %rax");switch(e->op){case LT:puts("  setl %al");break;case LE:puts("  setle %al");break;case GT:puts("  setg %al");break;case GE:puts("  setge %al");break;case EQ:puts("  sete %al");break;case NE:puts("  setne %al");break;default:break;}puts("  movzbq %al, %rax");}
}
static void emit_stmt(Stmt*s){if(!s)return;switch(s->k){case S_RETURN:emit_expr(s->e);puts("  jmp .Lreturn");break;case S_EXPR:emit_expr(s->e);break;case S_DECL:if(s->e)emit_expr(s->e);else puts("  mov $0, %rax");printf("  mov %%rax, -%d(%%rbp)\n",s->off);break;case S_BLOCK:for(size_t i=0;i<s->n;i++)emit_stmt(s->items[i]);break;case S_IF:{int f=label_id++,end=label_id++;emit_expr(s->cond);puts("  cmp $0, %rax");printf("  je .Lfalse%d\n",f);emit_stmt(s->a);printf("  jmp .Lend%d\n.Lfalse%d:\n",end,f);if(s->b)emit_stmt(s->b);printf(".Lend%d:\n",end);break;}case S_WHILE:{int a=label_id++,b=label_id++;printf(".Lwhile%d:\n",a);emit_expr(s->cond);puts("  cmp $0, %rax");printf("  je .Lwhile_end%d\n",b);emit_stmt(s->a);printf("  jmp .Lwhile%d\n.Lwhile_end%d:\n",a,b);break;}}}
static char*readall(FILE*f){size_t cap=4096,n=0;char*b=xmalloc(cap);int c;while((c=fgetc(f))!=EOF){if(n+1>=cap){cap*=2;b=realloc(b,cap);}b[n++]=(char)c;}b[n]=0;return b;}
static void free_expr(Expr*e){if(!e)return;free(e->name);free_expr(e->a);free_expr(e->b);free(e);} static void free_stmt(Stmt*s){if(!s)return;free(s->name);free_expr(s->e);free_expr(s->cond);free_stmt(s->a);free_stmt(s->b);for(size_t i=0;i<s->n;i++)free_stmt(s->items[i]);free(s->items);free(s);}
int main(void){char*src=readall(stdin);Parser p={src,0,{0}};next(&p);want(&p,INT);if(p.t.k!=ID||p.t.n!=4||strncmp(p.t.s,"main",4))diep(&p,"expected main");next(&p);want(&p,LP);want(&p,RP);Stmt*body=block(&p);if(p.t.k!=EOF_T)diep(&p,"unexpected tokens after main");resolve(body);int frame=(stack_bytes+15)&~15;puts(".text\n.globl main\nmain:");puts("  push %rbp\n  mov %rsp, %rbp");if(frame)printf("  sub $%d, %%rsp\n",frame);emit_stmt(body);puts("  mov $0, %rax\n.Lreturn:");if(frame)puts("  leave");else puts("  pop %rbp");puts("  ret");free_stmt(body);for(size_t i=0;i<nsym;i++)free(syms[i].name);free(syms);free(src);return 0;}