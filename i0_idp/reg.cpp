#include "i0.h"
#include "ins.hpp"
#include <i0_ida_common/i0_ida_spec.h>
#include <map>
#include <string>

static /*const*/ char* i0_shnames[] = { I0_IDA_SHORT_NAME, NULL };
static /*const*/ char* i0_lnames[] = { I0_IDA_LONG_NAME, NULL };

static std::map<ea_t, std::string> i0_sym_map;
static std::map<std::string, ea_t> i0_rev_sym_map;
static bool i0_has_sym_map_file = false;

//TODO: i0 assembler
/*static asm_t i0_asm = {};*/

static void i0_init_map()
{
	i0_sym_map.clear();
	i0_rev_sym_map.clear();
	i0_has_sym_map_file = true;
}

static void i0_insert_map(const ea_t* pEA, const char* sym)
{
	i0_sym_map[*pEA] = sym;
	i0_rev_sym_map[sym] = *pEA;
}

static asm_t i0_asm = {
	AS_COLON,
	0,
	"VPC i0 assembler",
	0,
	NULL,
	NULL,
	".org",
	".end",

	";",          // comment string
	'\'',         // string delimiter
	'\0',         // char delimiter (no char consts)
	"\\\"'",      // special symbols in char and string constants

	".ascii",     // ascii string directive
	".byte",      // byte directive
	".word",      // word directive
	NULL,         // dword  (4 bytes)
	NULL,         // qword  (8 bytes)
	NULL,         // oword  (16 bytes)
	NULL,         // float  (4 bytes)
	NULL,         // double (8 bytes)
	NULL,         // tbyte  (10/12 bytes)
	NULL,         // packed decimal real
	NULL,         // arrays (#h,#d,#v,#s(...)
	".block %s",  // uninited arrays
	".equ",       // Equ
	NULL,         // seg prefix
	//  preline, NULL, operdim,
	NULL, NULL, NULL,
	NULL,
	"$",
	NULL,         // func_header
	NULL,         // func_footer
	NULL,         // public
	NULL,         // weak
	NULL,         // extrn
	NULL,         // comm
	NULL,         // get_type_name
	NULL,         // align
	'(', ')',     // lbrace, rbrace
	NULL,    // mod
	NULL,    // and
	NULL,    // or
	NULL,    // xor
	NULL,    // not
	NULL,    // shl
	NULL,    // shr
	NULL,    // sizeof
};

static asm_t* i0_assemblers[] = { &i0_asm, NULL };

int idaapi i0_notify(processor_t::idp_notify msgid, ...)
{
	va_list va;
	va_start(va, msgid);
	int ret = invoke_callbacks(HT_IDP, msgid, va);
	if (ret) { return ret; }
	ret = 1;
	switch (msgid)
	{
	case processor_t::init:
		inf.mf = 0;
		break;
	case processor_t::loader:
	{
		I0_LDR_IDP_notify loader_notify_type = va_arg(va, I0_LDR_IDP_notify);
		switch (loader_notify_type)
		{
		case i0_loader_req_init_symtable:
			i0_init_map();
			break;
		case i0_loader_req_insert_sym:
		{
			const ea_t* pEA = va_arg(va, const ea_t*);
			const char* sym = va_arg(va, const char*);
			i0_insert_map(pEA, sym);
		}
			break;
		case i0_loader_req_finish_symtable:
			break;
		default:
			assert(0);
		}
	}
		break;
	}
	va_end(va);
	return ret;
}

processor_t LPH =
{
	IDP_INTERFACE_VERSION,        // version
	I0_IDA_LPH_ID,                      // id
	PRN_HEX | PR_USE64 | PR_DEFSEG64,
	8,                            // 8 bits in a byte for code segments
	8,                            // 8 bits in a byte for other segments

#if IDA_SDK_VERSION >= 600
	(const char**)i0_shnames,    // short processor names (null term)
#else
	i0_shnames,
#endif

#if IDA_SDK_VERSION >= 600
	(const char**)i0_lnames,     // long processor names (null term)
#else
	i0_lnames,
#endif

	i0_assemblers,       // array of enabled assemblers

	i0_notify,     // Various messages:

	i0_header,     // produce start of text file
	i0_footer,     // produce end of text file

	i0_segstart,   // produce start of segment
	i0_segend,     // produce end of segment

	NULL,

	i0_ana,
	i0_emu,

	i0_out_ui,
	i0_outop_ui,
	i0_data,    //intel_data,
	i0_cmp_opnd,       // compare operands
	NULL,       // can have type

	i0_number_of_register,    // Number of registers

#if IDA_SDK_VERSION >= 600
	i0_RegNames,             // Register names
#else
	i0_RegNames,
#endif
	NULL,                 // get abstract register

	0,                    // Number of register files
	NULL,                 // Register file names
	NULL,                 // Register descriptions
	NULL,                 // Pointer to CPU registers

	i0_reg_rVcs, i0_reg_rVds,
	0,                    // size of a segment register
	i0_reg_rVcs, i0_reg_rVds,

	NULL,                 // No known code start sequences
	NULL,

	0, I0_ins_last_ins,
	i0_Instructions,
	NULL,                 // int  (*is_far_jump)(int icode);
	NULL,                 // Translation function for offsets
	0,                    // int tbyte_size;  -- doesn't exist
	NULL,                 // int (*realcvt)(void *m, ushort *e, ushort swt);
	{ 0, 0, 0, 0 },       // char real_width[4];
	// number of symbols after decimal point
	// 2byte float (0-does not exist)
	// normal float
	// normal double
	// long double
	NULL,                 // int (*is_switch)(switch_info_t *si);
	NULL,                 // int32 (*gen_map_file)(FILE *fp);
	NULL,                 // ea_t (*extract_address)(ea_t ea,const char *string,int x);
	NULL,                 // int (*is_sp_based)(op_t &x); -- always, so leave it NULL
	NULL,                 // int (*create_func_frame)(func_t *pfn);
	NULL,                 // int (*get_frame_retsize(func_t *pfn)
	NULL,                 // void (*gen_stkvar_def)(char *buf,const member_t *mptr,int32 v);
	gen_spcdef,           // Generate text representation of an item in a special segment
};