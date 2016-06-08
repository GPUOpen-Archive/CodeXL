#ifndef SP3_TYPE_H
#define SP3_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/// @file sp3-type.h
/// @brief sp3 types

enum sp3_shtype {
    SP3_SHTYPE_NONE = -1,
    SP3_SHTYPE_PS   = 0,
    SP3_SHTYPE_VS   = 1,
    SP3_SHTYPE_GS   = 2,
    SP3_SHTYPE_ES   = 3,
    SP3_SHTYPE_HS   = 4,
    SP3_SHTYPE_LS   = 5,
    SP3_SHTYPE_CS   = 6,
};

enum sp3_count {
    SP3_NUM_MRT     = 8,
    SP3_NUM_STRM    = 4,
};

enum sp3_flag {
    SP3DIS_NO_STATE     = 0x01,
    SP3DIS_NO_BINARY    = 0x02,
    SP3DIS_COMMENTS     = 0x04,
    SP3DIS_NO_GPR_COUNT = 0x08,
    SP3DIS_FORCEVALID   = 0x10,
    SP3DIS_NO_ASIC      = 0x20,
};

/// @brief Shader context.  Contains no user-visible fields.
struct sp3_context;

/// @brief Storage entry for register streams.
struct sp3_reg {
    unsigned index; ///< One of the mm* values from chip_enum.h.
    unsigned value;
};

/// @brief Wrapped shader metadata.
///
/// After generation, shaders are encapsulated in sp3_shader structures.
///
/// Those structures contain the shader binary, its register stream,
/// constants and constant buffers and metadata needed for SC compatibility.
struct sp3_shader {
    int type;                   ///< One of the SHTYPE_* constants.
    int asic_int;               ///< Internal ASIC index. Do not use.
    const char *asic;           ///< ASIC name as a string ("RV870" etc).
    unsigned size;              ///< Size of the compiled shader, in 32-bit words.
    unsigned nsgprs;            ///< Number of scalar GPRs used.
    unsigned nvgprs;            ///< Number of vector GPRs used.
    unsigned trap_present;
    unsigned user_sgpr_count;
    unsigned scratch_en;
    unsigned dispatch_draw_en;
    unsigned so_en;
    unsigned so_base0_en;
    unsigned so_base1_en;
    unsigned so_base2_en;
    unsigned so_base3_en;
    unsigned oc_lds_en;
    unsigned tg_size_en;
    unsigned tidig_comp_cnt;    ///< Number of components(-1) enabled for thread id in group
    unsigned tgid_x_en;
    unsigned tgid_y_en;
    unsigned tgid_z_en;
    unsigned wave_cnt_en;
    unsigned sgpr_scratch;
    unsigned sgpr_psvs_state;
    unsigned sgpr_so_write_index;
    unsigned sgpr_so_base_offset0;
    unsigned sgpr_so_base_offset1;
    unsigned sgpr_so_base_offset2;
    unsigned sgpr_so_base_offset3;
    unsigned sgpr_offchip_lds;
    unsigned sgpr_is_offchip;
    unsigned sgpr_ring_offset;
    unsigned sgpr_gs_wave_id;
    unsigned sgpr_global_wave_id;
    unsigned sgpr_tg_size;
    unsigned sgpr_tgid_x;
    unsigned sgpr_tgid_y;
    unsigned sgpr_tgid_z;
    unsigned sgpr_tf_base;
    unsigned sgpr_wave_cnt;
    unsigned pc_exports;        ///< Range of parameters exported (if VS).
    unsigned pos_export;        ///< Shader executes a position export (if VS).
    unsigned cb_exports;        ///< Range of MRTs exported (if PS).
    unsigned mrtz_export_format; ///< Export format of the mrtz export.
    unsigned z_export;          ///< Shader executes a Z export (if PS).
    unsigned pops_en;           ///< Shader is POPS (PS)
    unsigned load_collision_waveid; ///< Shader sets load collision waveid (if PS).
    unsigned stencil_test_export; ///< Shader exports stencil (if PS).
    unsigned stencil_op_export; ///< Shader exports stencil (if PS).
    unsigned kill_used;         ///< Shader executes ALU KILL operations.
    unsigned cb_masks[SP3_NUM_MRT]; ///< Component masks for each MRT exported (if PS).
    unsigned emit_used;         ///< EMIT opcodes used (if GS).
    unsigned covmask_export;    ///< Shader exports coverage mask (if PS).
    unsigned mask_export;       ///< Shader exports mask (if PS).
    unsigned strm_used[SP3_NUM_STRM];   ///< Streamout operations used (map).
    unsigned scratch_used;      ///< Scratch SMX exports used.
    unsigned scratch_itemsize;  ///< Scratch ring item size.
    unsigned reduction_used;    ///< Reduction SMX exports used.
    unsigned ring_used;         ///< ESGS/GSVS ring SMX exports used.
    unsigned ring_itemsize;     ///< ESGS/GSVS ring item size (for ES/GS respectively).
    unsigned vertex_size[4];    ///< GSVS ring vertex size (for GS).
    unsigned mem_used;          ///< Raw memory SMX exports used.
    unsigned rats_used;         ///< Mask of RATs (UAVs) used
    unsigned group_size[3];     ///< Wavefront group size (for ELF files).
    unsigned alloc_lds;         ///< Number of LDS bytes allocated for wave group. (translates to lds_size in CS and LS)
    unsigned *data;             ///< Shader binary data.
    unsigned nregs;             ///< Number of register writes in the stream.
    struct sp3_reg *regs;       ///< Register writes (index-value pairs).
};

/// @brief Comment callback.
typedef const char *(*sp3_comment_cb)(void *, int);

#ifdef __cplusplus
}
#endif

#endif
