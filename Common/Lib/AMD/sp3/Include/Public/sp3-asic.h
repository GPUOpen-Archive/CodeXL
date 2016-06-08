#ifndef SP3_ASIC_H
#define SP3_ASIC_H


#include "sp3-int.h"
#include "sp3-vm.h"


#ifdef __cplusplus
extern "C" {
#endif


// ASIC types


enum asic_backend {
    ASIC_BACKEND_SI,
    ASIC_BACKEND_CI,
    ASIC_BACKEND_GFX8,
    ASIC_BACKEND_GFX81,
    ASIC_MAX_BACKEND,           // Must be the last entry
};


enum asic_cap_id {
    ASIC_THREAD_SIZE = 1,
    ASIC_FED_INSTRUCTIONS = 2,
    ASIC_LEGACY_LOG = 3,
    ASIC_LARGE_DS_READ = 4,
    ASIC_32BANK_LDS = 5,
};


struct asic_info {
    const char *name;
    enum asic_backend backend;  // which backend to use
    int asic_thread_size;       // number of threads in a wave
    int asic_fed_instructions;  // FED instructions are available
    int asic_legacy_log;        // Legacy EXP and LOG opcodes are available
    int asic_large_ds_read;     // Large DS read opcodes (96b and 128b) are available
    int asic_32bank_lds;        // Full 32 bank lds P1LL_F16 INTERP instruction available
};


struct sp3_asic_state {
    struct sp3_asic_aluop {
        int pos;	// original position in code
        int op, na, nc;	// na = number of args, nc = number of consts in args
        int lds, offset; // lds = is an LDS_IDX_OP subop, offset = LDS offset
        unsigned dst;
        unsigned arg[3];
        unsigned lit[3]; // float literals are no longer float at this point
        unsigned flags;
        int scalar;
    } bundle [5];
    unsigned lds_lit[2], lds_mask[2];
    int nbundle;
    int reorder;
    int last_reorder, last_po[5];
    int nscalar;		// number of nominally-scalar opcodes in bundle
    int barrier_after;	// require barrier after this clause
    
    // sp3-r6xx
    int asic;
    struct da_reloc {
        unsigned addr, ref;
        struct da_reloc *next;
    } *da_relocs;
    struct cf_reloc **instrels;
    struct cf_reloc *labels;
    int sinstrels;
    int slabels;
    char unk_name[16];
};
#define A S->ap


extern struct asic_info asics[];
#define ASICNAME asics[A->asic].name
#define ASIC asics[A->asic]
void set_asic(Sp, int asic);
int find_asic(const char *name);


// opcode tables

void sp3_unbuild_tables(void);
void sp3_si_unbuild_tables(void);
void sp3_ci_unbuild_tables(void);
void sp3_gfx8_unbuild_tables(void);

void sp3_build_tables(void);
void sp3_si_build_tables(void);
void sp3_ci_build_tables(void);
void sp3_gfx8_build_tables(void);




// helper functions


#define FMT_FMT		0x00000000
#define FMT_COMP	0x00010000
#define FMT_ENDIAN	0x00020000
#define FMT_NUM		0x00030000
#define FMT_SRF		0x00040000
#define FMT_MASK	0xFFFF0000
#define FMT_IMASK	0x0000FFFF

void mark_sgpr(Sp, unsigned);
void mark_vgpr(Sp, unsigned);
void mark_global(Sp, unsigned);
void mark_ctemp(Sp, unsigned);
int is_mod_bool(Sp, pnode *, const char *);
int get_mod_bool(Sp, pnode *, const char *);
int get_mod_int(Sp, pnode *, int, int);
int get_mod_int32(Sp, pnode *);
int par_cmask(Sp, pnode *);
unsigned reg_csel(Sp, unsigned , int);
unsigned reg_msel(Sp, unsigned *, int);

const char *spec_sel_to_name(Sp, int sel);
const char *sp3_fmt_to_name(Sp, int cls, int val);
const char *sp3_si_fmt_to_name(Sp, int cls, int val);
const char *sp3_ci_fmt_to_name(Sp, int cls, int val);
const char *sp3_gfx8_fmt_to_name(Sp, int cls, int val);

void add_reloc_label(Sp, int li, int blame);
void add_reloc_inst(Sp, int ii, int blame);
void add_reloc_cf(Sp, int offs);

int grouping_for_group_size(Sp, int group_size);

//JENNICA - this block of name_tree will go away, replace 
//with backend specific.

enum nametree_enum {
    NAMETREE_OPCODES,
    NAMETREE_OPCODES_0ARG,
    NAMETREE_OPCODES_CALL,   
    NAMETREE_VTX_FMTS,
    NAMETREE_SPEC_SELS,
    NAMETREE_SPEC_VEC_SELS,
    NAMETREE_SGPR_NAME_SELS,
    NAMETREE_CONSTS,
    NAMETREE_DEPRECATED,
};

struct name_tree **get_name_tree(struct sp3_state *S, enum nametree_enum whichtree);

extern struct name_tree *opcodes_0arg;
extern struct name_tree *opcodes_call;
extern struct name_tree *vtx_fmts;
extern struct name_tree *spec_sels;
extern struct name_tree *spec_vec_sels;
extern struct name_tree *sgpr_name_sels;
extern struct name_tree *consts;
extern struct name_tree *deprecated;

extern struct name_tree *asic_names;
struct asic_caps{const char *name; int id;};
extern struct asic_caps asiccaps[];
extern struct name_tree *asic_caps; //JENNICA - this may need to go away.

void update_sgpr_names(Sp);

#ifdef __cplusplus
}
#endif

#endif
