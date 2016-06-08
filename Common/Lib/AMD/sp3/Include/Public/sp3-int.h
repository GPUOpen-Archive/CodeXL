#ifndef SP3_INT_H
#define SP3_INT_H

#include "sp3.h"


#ifdef _MSC_VER
#ifndef strdup
#define strdup _strdup
#endif
#ifndef stricmp
#define stricmp _stricmp
#endif
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#pragma warning(disable:4090 4204 4245 4296 4389 4701 4702)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sp3_state;
#define Sp struct sp3_state *S

// clause types

#define CT_NONE         0
#define CT_SHADER       1

// parse tree

#define P_NUM   0       // integer
#define P_FLT   1       // float
#define P_STR   2       // string
#define P_REG   3       // register component(s)
#define P_RANGE 4       // closed range
#define P_RANGEL 5      // right-open range
#define P_SLICE 6       // array concatenation (used for slices)
#define P_RCAST 7       // integer -> register cast
#define P_LIST  8       // list (internal to the parser only)
#define P_VAR   9       // variable (with name)
#define P_VARE  10      // variable-element (result of lvalue slice)
#define P_CL    11      // clause
#define P_CLI   12      // clause instructions
#define P_WHILE 13      // while loop
#define P_REPEAT 14     // repeat-until loop
#define P_IF    15      // if or if-else
#define P_CFOR  16      // C-style for loop
#define P_FOR   17      // vector for loop
#define P_RET   18      // return from function
#define P_CSLICE 19     // componentwise slice
#define P_UREF  20      // unresolved reference
#define P_FREF  21      // resolved reference
#define P_CALL  22      // function call
#define P_PRINT 23      // print to stdout
#define P_PAR   24      // function parameters
#define P_NF    25      // native function
#define P_OMOD  27      // opcode modifier
#define P_OMODS 28      // opcode modifiers
#define P_OPARS 29      // opcode parameters
#define P_OP    30      // opcode
#define P_SWIZ0 31      // register swizzles with N components wrapped
#define P_SWIZ1 32      // -"-
#define P_SWIZ2 33      // -"-
#define P_SWIZ3 34      // -"-
#define P_SWIZ4 35      // -"-
#define P_VTXFMT 36     // vertex formats
#define P_LABEL 37      // unique identifier of a label
#define P_LINIT 38      // generate label identifiers
#define P_MARK  39      // mark a label
#define P_OPCALL 40     // opcode that does a clause instantiation on par0
#define P_ASIC  41      // ASIC model
#define P_ASICCAP 42    // ASIC capability
#define P_NCLOS 43      // create closure
#define P_CLOS  44      // closure
#define P_SH    45      // compiled shader

#define P_NOT   0x100
#define P_BNOT  0x101
#define P_NEG   0x102
#define P_MUL   0x103
#define P_DIV   0x104
#define P_MOD   0x105
#define P_ADD   0x106
#define P_SUB   0x107
#define P_SHL   0x108
#define P_SHR   0x109
#define P_SAR   0x10A
#define P_LT    0x10B
#define P_GT    0x10C
#define P_LEQ   0x10D
#define P_GEQ   0x10E
#define P_EQ    0x10F
#define P_NEQ   0x110
#define P_BAND  0x111
#define P_BOR   0x112
#define P_BXOR  0x113
#define P_AND   0x114
#define P_OR    0x115
#define P_XOR   0x116
#define P_SEL   0x117
#define P_XDEC  0x118
#define P_XINC  0x119
#define P_DECX  0x11A
#define P_INCX  0x11B
#define P_ASGN  0x11C
#define P_IND   0x11D
#define P_NOP   0x11E
#define P_VSUM  0x11F
#define P_VPROD 0x120
#define P_VBOR  0x121
#define P_VBAND 0x122
#define P_VBXOR 0x123
#define P_VOR   0x124
#define P_VAND  0x125
#define P_VXOR  0x126
#define P_VMIN  0x127
#define P_VMAX  0x128
#define P_CADD  0x129
#define P_CSUB  0x12A
#define P_CMUL  0x12B
#define P_CDIV  0x12C
#define P_CSHL  0x12D
#define P_CSHR  0x12E
#define P_CSAR  0x12F
#define P_CBAND 0x130
#define P_CBOR  0x131
#define P_CBXOR 0x132
#define P_CAND  0x133
#define P_COR   0x134
#define P_CXOR  0x135
#define P_CMIN  0x136
#define P_CMAX  0x137
#define P_MIN   0x138
#define P_MAX   0x139
#define P_PROBE 0x13A
#define P_BITS  0x13B

// register types
#define R_VGPR   0x00000
#define R_OFF    0x04000
#define R_SNAME  0x06000
#define R_INTERP 0x08000
#define R_SPEC   0x0A000
#define R_SGPR   0x0C000
#define R_EXPBUF 0x0E000
#define R_TMASK  0x1E000

// magic values for R_SPEC
#define R_P_CL  3       // used internally only (inline literal)
#define R_P_CI_L 0xDB   // used internally only
#define R_P_LDX_L 0xDB  // any LDS inline
#define R_P_LDS_L 0xDF  // direct LDS inline
#define R_P_LDS_H 0xE0
#define R_P_LDX_H 0xE0
#define R_P_CI_S 0xF3   // end of new R8xx constants
#define R_P_CI_H 0xFC
#define R_P_NOTLAST 0xFF// notlast operand for export

// magic values for R_SNAME
#define R_S_SCRATCH 1
#define R_S_PSVS_STATE 2
#define R_S_SO_WRITE_INDEX 3
#define R_S_SO_BASE_OFFSET0 4
#define R_S_SO_BASE_OFFSET1 5
#define R_S_SO_BASE_OFFSET2 6
#define R_S_SO_BASE_OFFSET3 7
#define R_S_OFFCHIP_LDS 8
#define R_S_IS_OFFCHIP 9
#define R_S_RING_OFFSET 10
#define R_S_GS_WAVE_ID 11
#define R_S_TG_SIZE 12
#define R_S_TF_BASE 13
#define R_S_TGID_X 14
#define R_S_TGID_Y 15
#define R_S_TGID_Z 16
#define R_S_WAVE_CNT 17
#define R_S_GLOBAL_WAVE_ID 18

// register components
#define R_CMASK 0x1C00
#define R_CSHIFT 10
#define R_CX    0x0000
#define R_CY    0x0400
#define R_CZ    0x0800
#define R_CW    0x0C00
#define R_CS    0x1000  // used to identify scalar elements
#define R_CN    0x1800

#define R_IMASK 0x03FF

// source transforms
#define R_NEG   0x80000
#define R_ABS   0x100000
#define R_SEXT  0x200000

// subencodings for export targets

#define R_E_TMASK       0x0380
#define R_E_MRT         0x0000
#define R_E_Z           0x0080
#define R_E_POS         0x0100
#define R_E_PARAM       0x0180
#define R_E_ATTR        0x0280
#define R_E_NULL        0x0300

#define R_E_IMASK 0x007F

// subencodings for interp

#define R_I_TMASK       0x0380
#define R_I_P10         0x0000
#define R_I_P20         0x0080
#define R_I_P0          0x0100

// function parameters
#define F_CANY  0x00000000
#define F_CNUM  0x01000000
#define F_CREG  0x02000000
#define F_CTMP  0x03000000
#define F_CFPTR 0x04000000
#define F_CINT  0x05000000
#define F_CMASK 0x07000000
#define F_OPT   0x40000000
#define F_VEC   0x80000000

typedef struct pnode {
    struct pnode *gc_next;
    int gc_mark;
    int type;
    int et;                             // error reporting tag
    int ni;                             // number of items
    union pnode_item {
        int num;                        // integer
        float flt;                      // float
        char *str;                      // string
        struct pnode *ptr;              // tree item
        struct {
            struct pnode *v;
            int e;
        } ve;                           // variable-element pair
        struct {
            int p;
            char *n;
        } var;                          // variable (stack offset, name)
        struct sp3_shader *sh;
        unsigned int reg;               // register components
        struct pnode *(* nf)(Sp, struct pnode **);  // native function
    } i[1];
} pnode;

pnode *p_str(Sp, char *s);              // wrap a string
pnode *p_float(Sp, float f);            // wrap a float
pnode *p_num(Sp, int i);                // wrap an integer
pnode *p_vec(Sp, int type, int len);    // create a vector
pnode *p_list(Sp, pnode *list, pnode *item);    // append item to P_LIST
pnode *p_list_rev(Sp, pnode *list);    // reverse the order of the list
pnode *p_tree(Sp, int type, int nitems, ...);   // create a tree node
pnode *p_l2t(Sp, int type, pnode *list);        // list to tree
pnode *p_l2v(Sp, int type, pnode *list);        // list to vector
pnode *p_x2x(Sp, int type, pnode *p);   // cast to type
pnode *p_clause(Sp, int vstk, int lstk, pnode *parlist, pnode *instlist, int type);
pnode *p_reg(Sp, int type, int idx);    // wrap a register
pnode *p_swizzle(Sp, char *str);        // parse a swizzle string
pnode *p_lv2rv(Sp, pnode *lval);        // lvalue to rvalue
pnode *p_newlabel(Sp, pnode *t, int tag);       // define new label
pnode *p_label(Sp, int cnt);            // fill with label IDs
pnode *p_clone(Sp, pnode *src);

void print_node(pnode *);               // print to stdout

void mark_gc_storage(Sp);               // mark all internal storage of sp3 for gc

// functions provided by machine driver
int is_opcode(struct sp3_state *S, const char *name);        // is an opcode (any)
int is_opcode_0arg(struct sp3_state *S, const char *name);   // is an opcode (0-argument)
int is_opcode_call(struct sp3_state *S, const char *name);   // is a call op (1st argument is a closure)
void sp3_gen_opcode(Sp, const char *op, pnode *par, pnode *mod);
void sp3_si_gen_opcode(Sp, const char *op, pnode *par, pnode *mod);
void sp3_ci_gen_opcode(Sp, const char *op, pnode *par, pnode *mod);
void sp3_gfx8_gen_opcode(Sp, const char *op, pnode *par, pnode *mod);
pnode *machine_const(Sp, char *name);   // if a machine const, parse it (else NULL)
void mark_label(Sp, int li);            // "label:"
pnode *asic_getcap(Sp, int id);         // get ASIC capability #id
void mach_cleanup(Sp);                  // initialize generator state

// name trees

#define NT_SEARCH 0
#define NT_ADD 1
#define NT_ADD_ONLY 2
#define NT_ADD_STRDUP 4
struct name_tree {
    const char *name;
    int tag;
    int add;
    struct name_tree *l, *r;
};

struct name_tree *name_tree_operation(struct name_tree **t, const char *name, int tag, int add);
void name_tree_delete(struct name_tree **t);

// symbol table

void f_decl(Sp, char *, pnode *);
pnode *f_ref(Sp, char *);
void f_check(Sp);
pnode *f_call(Sp, const char *);

void f_decl_native(Sp, int, char *, pnode *(*)(Sp, pnode **), int, ...);

// parse-time variable stack

void vs_decl(Sp, const char *, int tag);
int vs_lookup(Sp, const char *, pnode **, int);
char *vs_getname(pnode *);

void vs_enter_func(Sp);
int vs_leave_func(Sp, int *);   // returns number of stack allocations &
                                // (through param) number of lstack allocs
void vs_enter_block(Sp);
void vs_leave_block(Sp);

int vs_get_topmax(Sp);          // returns number of stack allocation for top level

// runtime variable stack

void rv_set(Sp, pnode *, pnode *);
pnode *rv_get(Sp, pnode *);
void rv_alloc(Sp, int);
void rv_setpar(Sp, int, pnode *);
int rv_enter(Sp, int);
void rv_leave(Sp, int);

int rl_enter(Sp, int);
void rl_leave(Sp, int);

void rv_leave_native(Sp);
pnode **rv_getpar_native(Sp);

// all-in-one variable setter

void rv_set_by_name(Sp, const char *, pnode *);

// growable binary buffer

typedef struct grow_buf {
    int n, size;
    unsigned i[1];
} grow_buf;

grow_buf *gb_alloc(int);
grow_buf *gb_append(grow_buf *, int, unsigned *);
grow_buf *gb_add(grow_buf *, unsigned);
grow_buf *gb_reg(grow_buf *, unsigned, unsigned);

// clause contents

struct clause_info {
    unsigned base;
    grow_buf *data;
    int type;
};

void start_clause(Sp, int);
void cb_emit(Sp, unsigned *, int);
int cb_ptr(Sp);
void cb_patch(Sp, int, int, unsigned);

int remap_clauses(Sp);

struct sp3_shader *gen_output(Sp);
void convert_relocs(Sp);
void perform_relocs(Sp);

pnode *shader_clos(Sp, pnode *);        // call this to get a binary shader from closure
pnode *shader_name(Sp, const char *);   // call this to get a binary shader from name

void set_const(Sp, int idx, unsigned val);
int find_const(Sp, unsigned val);

void set_kbuf(Sp, int kbuf, int idx, unsigned val);

const char *asic_name(Sp);
int asic_id(Sp);
int asic_capbyname(int, const char *);
int asic_capbyid(int, int);

// register stream packer
int sp3_guess_shader_type(struct sp3_state *S, struct sp3_shader *sh);
int sp3_si_guess_shader_type(struct sp3_shader *sh);
int sp3_ci_guess_shader_type(struct sp3_shader *sh);
int sp3_gfx8_guess_shader_type(struct sp3_shader *sh);
void sp3_pack_reg_stream(Sp, int type, struct sp3_shader *sh);
void sp3_si_pack_reg_stream(Sp, int type, struct sp3_shader *sh);
void sp3_ci_pack_reg_stream(Sp, int type, struct sp3_shader *sh);
void sp3_gfx8_pack_reg_stream(Sp, int type, struct sp3_shader *sh);
void unpack_reg_stream(Sp, struct sp3_shader *sh);

// instances

int new_instance(Sp, pnode *, int);
void eval_instances(Sp);
int get_instance_clause(Sp, int);
int get_instance_type(Sp, int);

// error reporting

void et_parse_mode(Sp, int);
int et_get_id(Sp);
#ifdef _MSC_VER
__declspec(noreturn)
#endif
void et_error(Sp, char *, char *, ...)
#ifdef __GNUC__
__attribute__ ((__noreturn__))
__attribute__ ((format(printf, 3, 4)))
#endif
;
void et_warning(Sp, char *, char *, ...)
#ifdef __GNUC__
__attribute__ ((format(printf, 3, 4)))
#endif
;
void et_blame(Sp, pnode *);
void et_blame_et(Sp, int);
void et_print(Sp, pnode *);
int et_get_blame(Sp);

// text buffer for disasm
void bprintf(Sp, char *, ...)
#ifdef __GNUC__
__attribute__ ((format(printf, 2, 3)))
#endif
;
void bcmt(Sp, const char *cmt, const char *start, const char *line, const char *end);
void btab(Sp, int);
char *bget(Sp);

// state structure
struct sp3_state {
    // flex
    void *scanner;
    void *yystate;

    char *yyfile;
    int yyline;

    // sp3-gc
    struct sp3_gc_state *gc;

    // asic private
    struct sp3_asic_state *ap;

    // sp3-eval
    int retflag;
    pnode *retval;

    // sp3-int
    struct sp3_shader config;

    int clause_id;              // counts up during evaluation
    int clause_type;
    struct clause_info *clauses;
    int nclauses, sclauses;

    int memsize, ctsizes[4];
    int in_shader;

    char *disasm_text;
    int disasm_column;
    int disasm_len, disasm_maxlen;

    sp3_vma *comment_map;
    void *comment_ctx;
    sp3_comment_cb comment_top, comment_right;

    unsigned const_buf[1024];
    int const_vld[1024], const_vld_range;

    unsigned *kval[16];
    int knum[16];

    struct et_record {
        const char *file;
        int line;
    } *et_names;
    int et_node;
    int et_parsing;
    int net_names, set_names;

    char *fname_last;
    struct name_tree *fnames;
    struct fsym {
        char *name;
        pnode *func;
        struct fref *refs;
        struct fsym *l, *r;
    } *fsymbols;
    int func_id;        // counts up during parsing

    struct instance {
        int type;
        int clause_id;
        pnode *call;
    } *instances;
    int ninstances, sinstances;

    struct vstack {
        char *name;
        int tag;
        int vs_sp, vs_level;
        struct vstack *next;
    } *var_stack, *lbl_stack;
    int vs_max, vs_sp, vs_top, vs_topmax;
    int ls_max, ls_sp;

    pnode **rl_stack;
    int rl_sp, rl_ss, rl_base, rl_id, rl_size;

    pnode **rv_stack;
    int rv_sp, rv_ss, rv_base, rv_size;

    int werror, wcount;
    const char *err_hdr;

    unsigned entry_point_table_size;
    unsigned entry_point_table_alloc_size;
    sp3_vmaddr *entry_point_table;
};
struct sp3_state *sp3_new_state(void);
void sp3_asic_attach_state(Sp);
void sp3_new_parser(Sp);
void sp3_free_parser(Sp);
void sp3_free_state(Sp);

void reg_natives(Sp);

#ifdef __cplusplus
}
#endif

#endif
