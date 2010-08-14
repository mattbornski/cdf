/******************************************************************************
*
*  NSSDC/CDF                                GnuZIP compression/decompression.
*
*  Version 1.0b, 2-Sep-97.
*
*  Modification history:
*
*   V1.0  27-Aug-96, J Love     Original version, but...the `zip' and `unzip'
*                               routines (and their supporting routines) are
*                               modified versions of routines provided by the
*                               named institutions and/or individuals.
*   V1.0a 20-Dec-96, J Love	Renamed BIT macro to avoid collision on
*				Linux systems.  Eliminated sanity check if
*				using Salford C.
*   V1.0b  2-Sep-97, J Love	Type casting for ANSI C.
*
******************************************************************************/

#include "cdflib.h"

#if !SUPPORT_GZIP_c && defined(BORLANDC)
#pragma warn -par
#endif

/******************************************************************************
* Macros.
******************************************************************************/

#if 0                   /* MS-DOS? */
#define SMALL_MEM
#endif

#if 0                   /* MacOS? */
#define MEDIUM_MEM
#endif

#define N_BORDER        19
#define N_CPLENS        31
#define N_CPLEXT        31
#define N_CPDIST        30
#define N_CPDEXT        30
#define N_MASK_BITS     17
#define N_CRC_32_TAB    256

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#ifndef LIT_BUFSIZE
#ifdef SMALL_MEM
#define LIT_BUFSIZE  0x2000
#else
#ifdef MEDIUM_MEM
#define LIT_BUFSIZE  0x4000
#else
#define LIT_BUFSIZE  0x8000
#endif
#endif
#endif

/******************************************************************************
* Typedef's and structure definitions.
******************************************************************************/

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

typedef ush Pos;
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

typedef struct {
   ush good_length; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
} config;

/* Data structure describing a single value and its code string. */
typedef struct ct_data {
    union {
	ush  freq;       /* frequency count */
	ush  code;       /* bit string */
    } fc;
    union {
	ush  dad;        /* father node in Huffman tree */
	ush  len;        /* length of bit string */
    } dl;
} ct_data;

typedef struct tree_desc {
    ct_data *dyn_tree;      /* the dynamic tree */
    ct_data *static_tree;   /* corresponding static tree or NULL */
    int     *extra_bits;    /* extra bits for each code or NULL */
    int     extra_base;          /* base index for extra_bits */
    int     elems;               /* max number of elements in the tree */
    int     max_length;          /* max bit length for the codes */
    int     max_code;            /* largest code with non zero frequency */
} tree_desc;

/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct huft {
  uch e;                /* number of extra bits or operation */
  uch b;                /* number of bits in this code or subcode */
  union {
    ush n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};

struct gZipStruct {
  vFILE *iFp;
  vFILE *oFp;
  CDFstatus iError;     /* Returned if input file error. */
  CDFstatus oError;     /* Returned if output file error. */
  long ifile_size;      /* input file size */
  unsigned short bi_buf;/* Output buffer. bits are inserted starting
			   at the bottom (least significant bits). */
  int bi_valid;         /* Number of valid bits in bi_buf.  All bits above
			   the last valid bit are always zero. */
  long block_start;     /* window position at the beginning of the current
			   output block. Gets negative when the window is
			   moved backwards. */
  unsigned ins_h;       /* hash index of string to be inserted */
  unsigned strstart;    /* start of string to insert */
  unsigned match_start; /* start of matching string */
  int eofile;           /* flag set at end of input file */
  unsigned lookahead;   /* number of valid bytes ahead in window */
  unsigned int prev_length;
			/* Length of the best match at previous step. Matches
			   not greater than this are discarded. This is used
			   in the lazy match evaluation. */
  ush bl_count[MAX_BITS+1];
			/* number of codes at each bit length for an optimal
			   tree */
  int heap[2*L_CODES+1];/* heap used to build the Huffman trees */
  int heap_len;         /* number of elements in the heap */
  int heap_max;         /* element of largest frequency */
  /* The sons of heap[n] are heap[2*n] and heap[2*n+1].
   * heap[0] is not used.
   * The same heap array is used to build all trees.
   */
  uch depth[2*L_CODES+1];
			/* Depth of each subtree used as tie breaker for
			   trees of equal frequency */
  uch length_code[MAX_MATCH-MIN_MATCH+1];
			/* length code for each normalized match length
			   (0 == MIN_MATCH) */
  uch dist_code[512];   /* distance codes. The first 256 values correspond
			   to the distances 3 .. 258, the last 256 values
			   correspond to the top 8 bits of the 15 bit
			   distances. */
  int base_length[LENGTH_CODES];
			/* First normalized length for each code
			   (0 = MIN_MATCH) */
  int base_dist[D_CODES];
			/* First normalized distance for each code
			   (0 = distance of 1) */
  uch flag_buf[(LIT_BUFSIZE/8)];
			/* flag_buf is a bit array distinguishing literals
			   from lengths in l_buf (inbuf), thus indicating the
			   presence or absence of a distance. */
  unsigned last_lit;    /* running index in l_buf (inbuf) */
  unsigned last_dist;   /* running index in d_buf */
  unsigned last_flags;  /* running index in flag_buf */
  uch tree_flags;       /* current flags not yet saved in flag_buf */
  uch flag_bit;         /* current bit used in flags */
			/* bits are filled in flags starting at bit 0 (least
			   significant).  Note: these flags are overkill in
			   the current code since we don't take advantage of
			   DIST_BUFSIZE == LIT_BUFSIZE. */
  ulg opt_len;          /* bit length of current block with optimal trees */
  ulg static_len;       /* bit length of current block with static trees */
  int level;            /* compression level */
  unsigned outcnt;      /* bytes in output buffer */
  long bytes_in;        /* number of input bytes */
  long bytes_out;       /* number of output bytes */
  ulg crc;              /* crc on uncompressed file data */
  ulg crcReg;           /* shift register contents */
  config configuration_table[10];
  uch bl_order[BL_CODES];
			/* The lengths of the bit length codes are sent in
			   order of decreasing probability, to avoid
			   transmitting the lengths for unused bit length
			   codes. */
  int extra_lbits[LENGTH_CODES];
			/* extra bits for each length code */
  ct_data dyn_ltree[HEAP_SIZE];
			/* literal and length tree */
  ct_data static_ltree[L_CODES+2];
			/* The static literal tree. Since the bit lengths are
			   imposed, there is no need for the L_CODES extra
			   codes used during heap construction. However, the
			   codes 286 and 287 are needed to build a canonical
			   tree (see ct_init below). */
  tree_desc l_desc;
  int extra_dbits[D_CODES];
			/* extra bits for each distance code */
  ct_data dyn_dtree[2*D_CODES+1];
			/* distance tree */
  ct_data static_dtree[D_CODES];
			/* The static distance tree. (Actually a trivial tree
			   since all codes use 5 bits.) */
  tree_desc d_desc;
  int extra_blbits[BL_CODES];
			/* extra bits for each bit length code */
  ct_data bl_tree[2*BL_CODES+1];
			/* Huffman tree for the bit lengths */
  tree_desc bl_desc;
  ulg crc_32_tab[N_CRC_32_TAB];
			/* Table of CRC-32's of all single-byte values (made
			   by makecrc.c) */
  uch *inbuf;           /* input buffer */
  uch *outbuf;          /* output buffer */
  ush *d_buf;           /* buffer for distances, see trees.c */
  uch *window;          /* Sliding window and suffix table (unlzw) */
  ush *prev;
  ush *head;
};
typedef struct gZipStruct GZ;
typedef struct gZipStruct *GZp;

struct gUnZipStruct {
  vFILE *iFp;
  vFILE *oFp;
  CDFstatus iError;     /* Returned if input file error. */
  CDFstatus oError;     /* Returned if output file error. */
  ulg bb;               /* bit buffer */
  unsigned bk;          /* bits in bit buffer */
  unsigned hufts;       /* track memory usage */
  unsigned outcnt;      /* bytes in output buffer */
  long bytes_in;        /* number of input bytes */
  long bytes_out;       /* number of output bytes */
  ulg crcReg;           /* shift register contents */
  unsigned insize;      /* valid bytes in inbuf */
  unsigned inptr;       /* index of next byte to be processed in inbuf */
  unsigned border[N_BORDER];
			/* Order of the bit length code lengths */
  ush cplens[N_CPLENS]; /* Copy lengths for literal codes 257..285 */
  ush cplext[N_CPLEXT]; /* Extra bits for literal codes 257..285 */
  ush cpdist[N_CPDIST]; /* Copy offsets for distance codes 0..29 */
  ush cpdext[N_CPDEXT]; /* Extra bits for distance codes */
  ush mask_bits[N_MASK_BITS];
  ulg crc_32_tab[N_CRC_32_TAB];
			/* Table of CRC-32's of all single-byte values (made
			   by makecrc.c) */
  uch *inbuf;           /* input buffer */
  uch *window;          /* Sliding window and suffix table (unlzw) */
};
typedef struct gUnZipStruct GU;
typedef struct gUnZipStruct *GUp;

/******************************************************************************
* More macros.
******************************************************************************/

#define OS_CODE         0xFF    /* Does it really matter? - JTL */
				/* <Operating System,OS_CODE>: 
				   <MS-DOS,0x0> <OS2,0x6> <WindowsNT,0xB>
				   <VMS,0x2> <UNIX,0x3> <amiga,0x1>
				   <atari,0x5> <MacOS,0x7> <PRIMOS,0xF>
				   <TOPS20,0xA> */

#define OF(args) PROTOARGs(args)
#define memzero(s,n) memset((void*)(s),0,(n))

/* Compression methods (see algorithm.doc) */
#define DEFLATED    8

/* To save memory for 16 bit systems, some arrays are overlaid between
 * the various modules:
 * deflate:  prev+head   window      d_buf  l_buf  outbuf
 * unlzw:    tab_prefix  tab_suffix  stack  inbuf  outbuf
 * inflate:              window             inbuf
 * unpack:               window             inbuf  prefix_len
 * unlzh:    left+right  window      c_table inbuf c_len
 * For compression, input is done in window[]. For decompression, output
 * is done in window except for unlzw.
 */

#ifndef INBUFSIZ
#ifdef SMALL_MEM
#define INBUFSIZ  0x2000  /* input buffer size */
#else
#define INBUFSIZ  0x8000  /* input buffer size */
#endif
#endif
#define INBUF_EXTRA  64     /* required by unlzw() */

#ifndef OUTBUFSIZ
#ifdef SMALL_MEM
#define OUTBUFSIZ   8192  /* output buffer size */
#else
#define OUTBUFSIZ  16384  /* output buffer size */
#endif
#endif
#define OUTBUF_EXTRA 2048   /* required by unlzw() */

#ifndef DIST_BUFSIZE
#ifdef SMALL_MEM
#define DIST_BUFSIZE 0x2000 /* buffer for distances, see trees.c */
#else
#define DIST_BUFSIZE 0x8000 /* buffer for distances, see trees.c */
#endif
#endif

#define GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#ifndef WSIZE
#define WSIZE 0x8000     /* window size--must be a power of two, and */
#endif                     /*  at least 32K for zip's deflate method */
#define WINDOW_SIZE ((ulg)2*WSIZE)

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))

/* lzw.h -- define the lzw functions.
 * Copyright (C) 1992-1993 Jean-loup Gailly.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

#define LBITS 9         /* bits in base literal/length lookup table */
#define DBITS 6         /* bits in base distance lookup table */

#ifndef BITSx
#define BITSx 16
#endif
#define INIT_BITS 9              /* Initial number of bits per code */

#define BUF_SIZE (8 * 2*sizeof(char))
/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */

/* Compile with MEDIUM_MEM to reduce the memory requirements or
 * with SMALL_MEM to use as little memory as possible. Use BIG_MEM if the
 * entire input file can be held in memory (not possible on 16 bit systems).
 * Warning: defining these symbols affects HASH_BITS (see below) and thus
 * affects the compression ratio. The compressed output
 * is still correct, and might even be smaller in some cases.
 */

#ifdef SMALL_MEM
#define HASH_BITS  13  /* Number of bits used to hash strings */
#endif
#ifdef MEDIUM_MEM
#define HASH_BITS  14
#endif
#ifndef HASH_BITS
#define HASH_BITS  15
   /* For portability to 16 bit machines, do not use values above 15. */
#endif

/* To save space (see unlzw.c), we overlay prev+head with tab_prefix and
 * window with tab_suffix. Check that we can do this:
 */
#if !defined(SALFORDC)
#if (WSIZE<<1) > (1<<BITSx)
   error: cannot overlay window with tab_suffix and prev with tab_prefix0
#endif
#endif
#if HASH_BITS > BITSx-1
   error: cannot overlay head with tab_prefix1
#endif
#if EOF != (-1)
   error: EOF != (-1)
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */

#define NIL 0
/* Tail of hash chains */

#define FAST 4
#define SLOW 2
/* speed options for the general purpose bit flag */

#ifndef TOO_FAR
#define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */

/* ===========================================================================
 * Constants
 */

#define MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#define END_BLOCK 256
/* end of block literal code */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#ifndef DIST_BUFSIZE
#define DIST_BUFSIZE  LIT_BUFSIZE
#endif
/* Sizes of match buffers for literals/lengths and distances.  There are
 * 4 reasons for limiting LIT_BUFSIZE to 64K:
 *   - frequencies can be kept in 16 bit counters
 *   - if compression is not successful for the first block, all input data is
 *     still in the window so we can still emit a stored block even when input
 *     comes from standard input.  (This can also be done for all blocks if
 *     LIT_BUFSIZE is not greater than 32K.)
 *   - if compression is not successful for a file smaller than 64K, we can
 *     even emit a stored file instead of a stored block (saving 5 bytes).
 *   - creating new Huffman trees less frequently may not provide fast
 *     adaptation to changes in the input data statistics. (Take for
 *     example a binary file with poorly compressible code followed by
 *     a highly compressible string table.) Smaller buffer sizes give
 *     fast adaptation but have of course the overhead of transmitting trees
 *     more frequently.
 *   - I can't count above 4
 * The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
 * memory at the expense of compression). Some optimizations would be possible
 * if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
 */
#if LIT_BUFSIZE > INBUFSIZ
    error cannot overlay l_buf and inbuf
#endif

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

#define send_code(gz_, c_, tree_) \
send_bits(gz_, tree_[c_].fc.code, tree_[c_].dl.len)
   /* Send a code of the given tree. c and tree must not have side effects */

#define d_code(dist_,dist_code_) \
   ((dist_) < 256 ? dist_code_[dist_] : dist_code_[256+((dist_)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. dist_code[256] and dist_code[257] are never
 * used.
 */

/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */

/* Macros for inflate() bit peeking and grabbing.
   The usage is:
   
	NEEDBITS(gu,b,k,j)
	x = b & mask_bits[j];
	DUMPBITS(b,k,j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the beginning of a
   routine that uses these macros from a global bit buffer and count.

   If we assume that EOB will be the longest code, then we will never
   ask for bits with NEEDBITS that are beyond the end of the stream.
   So, NEEDBITS should not read any more bytes than are needed to
   meet the request.  Then no bytes need to be "returned" to the buffer
   at the end of the last block.

   However, this assumption is not true for fixed blocks--the EOB code
   is 7 bits, but the other literal/length codes can be 8 or 9 bits.
   (The EOB code is shorter than other codes because fixed blocks are
   generally short.  So, while a block always has an EOB, many other
   literal/length codes have a significantly lower probability of
   showing up at all.)  However, by making the first table have a
   lookup of seven bits, the EOB code will be found in that first
   lookup, and so will not require that too many bits be pulled from
   the stream.
 */

#define NEEDBITS(gu_,b_,k_,n_) { \
  while (k_ < (n_)) { \
    uch byte_; \
    if (!GetByte(gu_,&byte_)) return 4; /* Read error. */ \
    b_ |= (((ulg)byte_) << k_); \
    k_ += 8; \
  } \
}

#define DUMPBITS(b_,k_,n_) { \
b_ >>= (n_); \
k_ -= (n_); \
}

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */

/******************************************************************************
* Function prototypes.
******************************************************************************/

#if SUPPORT_GZIP
static void initCRC OF((ulg crc_32_tab[N_CRC_32_TAB]));
static Logical PutByte OF((GZp gz, unsigned c));
static Logical PutShort OF((GZp gz, unsigned w));
static Logical PutLong OF((GZp gz, ulg n));
static Logical GetByte OF((GUp gu, uch *byte));
static CDFstatus zip OF((
  vFILE *iFp, Int32 iSize, CDFstatus iError, vFILE *oFp, Int32 *oSize,
  CDFstatus oError, Int32 level
));
static GZp initZip OF((
  vFILE *iFp, vFILE *oFp, Int32 iSize, Int32 level, CDFstatus iError,
  CDFstatus oError
));
static void freeZip OF((GZp gz));
static int file_read OF((GZp gz, char *buf,  unsigned size));
static CDFstatus unzip OF((
  vFILE *iFp, vFILE *oFp, CDFstatus iError, CDFstatus oError
));
static GUp initUnZip OF((
  vFILE *iFp, vFILE *oFp, CDFstatus iError, CDFstatus oError
));
static void freeUnZip OF((GUp gu));
static Logical lm_init OF((GZp gz, ush *flags));
static CDFstatus deflate OF((GZp gz));
static void ct_init OF((GZp gz));
static int ct_tally OF((GZp gz, int dist, int lc));
static Logical flush_block OF((GZp gz, char *buf, ulg stored_len, int eof));
static void bi_init OF((GZp gz));
static Logical send_bits OF((GZp gz, int value, int length));
static unsigned bi_reverse OF((unsigned value, int length));
static Logical bi_windup  OF((GZp gz));
static Logical copy_block OF((GZp gz, char *buf, unsigned len, int header));
static ulg updcrc OF((
  uch *s, unsigned n, ulg *crcReg, ulg crc_32_tab[N_CRC_32_TAB]
));
static Logical fill_inbuf OF((GUp gu, uch *byte));
static Logical flush_outbuf OF((GZp gz));
static Logical flush_window OF((GUp gu));
static Logical write_buf OF((vFILE *vFp, void *buf, unsigned cnt));
static int inflate OF((GUp gu));
static Logical fill_window OF((GZp gz));
static CDFstatus deflate_fast OF((GZp gz));
static int longest_match OF((GZp gz, IPos cur_match));
static void init_block OF((GZp gz));
static void pqdownheap OF((GZp gz, ct_data *tree, int k));
static void gen_bitlen OF((GZp gz, tree_desc *desc));
static void gen_codes OF((GZp gz, ct_data *tree, int max_code));
static void build_tree_gz OF((GZp gz, tree_desc *desc));
static void scan_tree OF((GZp gz, ct_data *tree, int max_code));
static Logical send_tree OF((GZp gz, ct_data *tree, int max_code));
static int  build_bl_tree OF((GZp gz));
static Logical send_all_trees OF((
  GZp gz, int lcodes, int dcodes, int blcodes
));
static Logical compress_block OF((GZp gz, ct_data *ltree, ct_data *dtree));
static int huft_build OF((
  GUp gu, unsigned *b, unsigned n, unsigned s, ush *d, ush *e,
  struct huft **t, int *m
));
static int huft_free OF((struct huft *t));
static int inflate_codes OF((
  GUp gu, struct huft *tl, struct huft *td, int bl, int bd
));
static int inflate_stored OF((GUp gu));
static int inflate_fixed OF((GUp gu));
static int inflate_dynamic OF((GUp gu));
static int inflate_block OF((GUp gu, int *e));
static int inflate OF((GUp gu));
#endif

/******************************************************************************
* CompressGZIP.
******************************************************************************/

STATICforIDL CDFstatus CompressGZIP (srcFp, srcOffset, srcSize, srcError,
				     destFp, destOffset, destSize, destError,
				     level)
vFILE *srcFp;
Int32 srcOffset;
Int32 srcSize;
CDFstatus srcError;
vFILE *destFp;
Int32 destOffset;
Int32 *destSize;
CDFstatus destError;
Int32 level;
{
#if SUPPORT_GZIP
  CDFstatus pStatus = CDF_OK;
  if (!SEEKv(srcFp,(long)srcOffset,vSEEK_SET)) return srcError;
  if (!SEEKv(destFp,(long)destOffset,vSEEK_SET)) return destError;
  if (!sX(zip(srcFp,srcSize,srcError,
	      destFp,destSize,destError,level),&pStatus)) return pStatus;
  return pStatus;
#else
  return UNKNOWN_COMPRESSION;
#endif
}

/******************************************************************************
* DecompressGZIP.
******************************************************************************/

STATICforIDL CDFstatus DecompressGZIP (srcFp, srcOffset, srcError, destFp,
				       destOffset, destError)
vFILE *srcFp;
Int32 srcOffset;
CDFstatus srcError;
vFILE *destFp;
Int32 destOffset;
CDFstatus destError;
{
#if SUPPORT_GZIP
  CDFstatus pStatus = CDF_OK;
  if (!SEEKv(srcFp,(long)srcOffset,vSEEK_SET)) return srcError;
  if (!SEEKv(destFp,(long)destOffset,vSEEK_SET)) return destError;
  if (!sX(unzip(srcFp,destFp,srcError,destError),&pStatus)) return pStatus;
  return pStatus;
#else
  return UNKNOWN_COMPRESSION;
#endif
}

#if SUPPORT_GZIP
/******************************************************************************
* zip.
*   Returns as soon as an error occurs.  Any memory allocated will be freed
* by the calling routine.  If successful, this routine frees the memory.
******************************************************************************/

/* zip.c -- compress files to the gzip or pkzip format
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/* ===========================================================================
 * Deflate in to out.
 * IN assertions: the input and output buffers are cleared.
 *   The variables time_stamp and save_orig_name are initialized.
 */

static CDFstatus zip (iFp, iSize, iError, oFp, oSize, oError, level)
vFILE *iFp;
Int32 iSize;
CDFstatus iError;
vFILE *oFp;
Int32 *oSize;
CDFstatus oError;
Int32 level;
{
  CDFstatus pStatus = CDF_OK;
  uch  flags2 = 0;         /* general purpose bit flags */
  ush  deflate_flags = 0;  /* pkzip -es, -en or -ex equivalent */
  long time_stamp = 0;  /* JTL */
  GZp gz;
  gz = initZip (iFp, oFp, iSize, level, iError, oError);
  if (gz == NULL) return BAD_MALLOC;
  /* Write the header to the gzip file. See algorithm.doc for the format */
  if (!PutByte(gz,(unsigned)GZIP_MAGIC[0])) return gz->oError;
						/* magic header[0]. */
  if (!PutByte(gz,(unsigned)GZIP_MAGIC[1])) return gz->oError;
						/* magic header[1]. */
  if (!PutByte(gz,(unsigned)DEFLATED)) return gz->oError;
						/* compression method. */
  if (!PutByte(gz,(unsigned)flags2)) return gz->oError;
						/* general flags. */
  if (!PutLong(gz,time_stamp)) return gz->oError;
						/* time stamp. */
  /* Write deflated file to zip file */
  gz->crc = updcrc (0, 0, &(gz->crcReg), gz->crc_32_tab);
  bi_init(gz);
  ct_init(gz);
  if (!lm_init(gz,&deflate_flags)) return gz->iError;   /* Read error. */
  if (!PutByte(gz,(unsigned)deflate_flags)) return gz->oError;
						/* extra flags. */
  if (!PutByte(gz,(unsigned)OS_CODE)) return gz->oError;
						/* OS identifier. */
  if (!sX(deflate(gz),&pStatus)) return pStatus;
  /* Write the crc and uncompressed size */
  if (!PutLong(gz,gz->crc)) return gz->oError;
  if (!PutLong(gz,gz->bytes_in)) return gz->oError;
  if (!flush_outbuf(gz)) return gz->oError;     /* Write error. */
  *oSize = (Int32) gz->bytes_out;
  freeZip (gz);
  return pStatus;
}

/******************************************************************************
* initZip.
*   Returns as soon as an error occurs.  Any memory allocated will be freed
* by the calling routine.
******************************************************************************/

static GZp initZip (iFp, oFp, iSize, level, iError, oError)
vFILE *iFp;
vFILE *oFp;
Int32 iSize;
Int32 level;
CDFstatus iError;
CDFstatus oError;
{
    GZp gz = (GZp) CallocateMemory ((size_t) 1, sizeof(GZ), NULL);
    if (gz == NULL) return NULL;
    gz->inbuf = (uch *) CallocateMemory ((size_t) (INBUFSIZ+INBUF_EXTRA),
					 sizeof(uch), NULL);
    if (gz->inbuf == NULL) return NULL;
    gz->outbuf = (uch *) CallocateMemory ((size_t) (OUTBUFSIZ+OUTBUF_EXTRA),
					  sizeof(uch), NULL);
    if (gz->outbuf == NULL) return NULL;
    gz->d_buf = (ush *) CallocateMemory ((size_t) (DIST_BUFSIZE),
					 sizeof(ush), NULL);
    if (gz->d_buf == NULL) return NULL;
    gz->window = (uch *) CallocateMemory ((size_t) (2L*WSIZE),
					  sizeof(uch), NULL);
    if (gz->window == NULL) return NULL;
    gz->prev = (ush *) CallocateMemory ((size_t) (1L<<(BITSx-1)),
					sizeof(ush), NULL);
    if (gz->prev == NULL) return NULL;
    gz->head = (ush *) CallocateMemory ((size_t) (1L<<(BITSx-1)),
					sizeof(ush), NULL);
    if (gz->head == NULL) return NULL;

    gz->iFp = iFp;
    gz->oFp = oFp;
    gz->ifile_size = (long) iSize;
    gz->level = (int) level;
    gz->outcnt = 0;
    gz->bytes_in = 0;
    gz->bytes_out = 0;
    gz->crcReg = (ulg) 0xffffffffL;
    gz->iError = iError;
    gz->oError = oError;

    gz->configuration_table[0].good_length = 0;
    gz->configuration_table[0].max_lazy = 0;
    gz->configuration_table[0].nice_length = 0;
    gz->configuration_table[0].max_chain = 0;
    gz->configuration_table[1].good_length = 4;
    gz->configuration_table[1].max_lazy = 4;
    gz->configuration_table[1].nice_length = 8;
    gz->configuration_table[1].max_chain = 4;
    gz->configuration_table[2].good_length = 4;
    gz->configuration_table[2].max_lazy = 5;
    gz->configuration_table[2].nice_length = 16;
    gz->configuration_table[2].max_chain = 8;
    gz->configuration_table[3].good_length = 4;
    gz->configuration_table[3].max_lazy = 6;
    gz->configuration_table[3].nice_length = 32;
    gz->configuration_table[3].max_chain = 32;
    gz->configuration_table[4].good_length = 4;
    gz->configuration_table[4].max_lazy = 4;
    gz->configuration_table[4].nice_length = 16;
    gz->configuration_table[4].max_chain = 16;
    gz->configuration_table[5].good_length = 8;
    gz->configuration_table[5].max_lazy = 16;
    gz->configuration_table[5].nice_length = 32;
    gz->configuration_table[5].max_chain = 32;
    gz->configuration_table[6].good_length = 8;
    gz->configuration_table[6].max_lazy = 16;
    gz->configuration_table[6].nice_length = 128;
    gz->configuration_table[6].max_chain = 128;
    gz->configuration_table[7].good_length = 8;
    gz->configuration_table[7].max_lazy = 32;
    gz->configuration_table[7].nice_length = 128;
    gz->configuration_table[7].max_chain = 256;
    gz->configuration_table[8].good_length = 32;
    gz->configuration_table[8].max_lazy = 128;
    gz->configuration_table[8].nice_length = 258;
    gz->configuration_table[8].max_chain = 1024;
    gz->configuration_table[9].good_length = 32;
    gz->configuration_table[9].max_lazy = 258;
    gz->configuration_table[9].nice_length = 258;
    gz->configuration_table[9].max_chain = 4096;

    gz->bl_order[0] = 16;
    gz->bl_order[1] = 17;
    gz->bl_order[2] = 18;
    gz->bl_order[3] = 0;
    gz->bl_order[4] = 8;
    gz->bl_order[5] = 7;
    gz->bl_order[6] = 9;
    gz->bl_order[7] = 6;
    gz->bl_order[8] = 10;
    gz->bl_order[9] = 5;
    gz->bl_order[10] = 11;
    gz->bl_order[11] = 4;
    gz->bl_order[12] = 12;
    gz->bl_order[13] = 3;
    gz->bl_order[14] = 13;
    gz->bl_order[15] = 2;
    gz->bl_order[16] = 14;
    gz->bl_order[17] = 1;
    gz->bl_order[18] = 15;

    gz->extra_lbits[0] = 0;
    gz->extra_lbits[1] = 0;
    gz->extra_lbits[2] = 0;
    gz->extra_lbits[3] = 0;
    gz->extra_lbits[4] = 0;
    gz->extra_lbits[5] = 0;
    gz->extra_lbits[6] = 0;
    gz->extra_lbits[7] = 0;
    gz->extra_lbits[8] = 1;
    gz->extra_lbits[9] = 1;
    gz->extra_lbits[10] = 1;
    gz->extra_lbits[11] = 1;
    gz->extra_lbits[12] = 2;
    gz->extra_lbits[13] = 2;
    gz->extra_lbits[14] = 2;
    gz->extra_lbits[15] = 2;
    gz->extra_lbits[16] = 3;
    gz->extra_lbits[17] = 3;
    gz->extra_lbits[18] = 3;
    gz->extra_lbits[19] = 3;
    gz->extra_lbits[20] = 4;
    gz->extra_lbits[21] = 4;
    gz->extra_lbits[22] = 4;
    gz->extra_lbits[23] = 4;
    gz->extra_lbits[24] = 5;
    gz->extra_lbits[25] = 5;
    gz->extra_lbits[26] = 5;
    gz->extra_lbits[27] = 5;
    gz->extra_lbits[28] = 0;

    gz->extra_dbits[0] = 0;
    gz->extra_dbits[1] = 0;
    gz->extra_dbits[2] = 0;
    gz->extra_dbits[3] = 0;
    gz->extra_dbits[4] = 1;
    gz->extra_dbits[5] = 1;
    gz->extra_dbits[6] = 2;
    gz->extra_dbits[7] = 2;
    gz->extra_dbits[8] = 3;
    gz->extra_dbits[9] = 3;
    gz->extra_dbits[10] = 4;
    gz->extra_dbits[11] = 4;
    gz->extra_dbits[12] = 5;
    gz->extra_dbits[13] = 5;
    gz->extra_dbits[14] = 6;
    gz->extra_dbits[15] = 6;
    gz->extra_dbits[16] = 7;
    gz->extra_dbits[17] = 7;
    gz->extra_dbits[18] = 8;
    gz->extra_dbits[19] = 8;
    gz->extra_dbits[20] = 9;
    gz->extra_dbits[21] = 9;
    gz->extra_dbits[22] = 10;
    gz->extra_dbits[23] = 10;
    gz->extra_dbits[24] = 11;
    gz->extra_dbits[25] = 11;
    gz->extra_dbits[26] = 12;
    gz->extra_dbits[27] = 12;
    gz->extra_dbits[28] = 13;
    gz->extra_dbits[29] = 13;

    gz->extra_blbits[0] = 0;
    gz->extra_blbits[1] = 0;
    gz->extra_blbits[2] = 0;
    gz->extra_blbits[3] = 0;
    gz->extra_blbits[4] = 0;
    gz->extra_blbits[5] = 0;
    gz->extra_blbits[6] = 0;
    gz->extra_blbits[7] = 0;
    gz->extra_blbits[8] = 0;
    gz->extra_blbits[9] = 0;
    gz->extra_blbits[10] = 0;
    gz->extra_blbits[11] = 0;
    gz->extra_blbits[12] = 0;
    gz->extra_blbits[13] = 0;
    gz->extra_blbits[14] = 0;
    gz->extra_blbits[15] = 0;
    gz->extra_blbits[16] = 2;
    gz->extra_blbits[17] = 3;
    gz->extra_blbits[18] = 7;

    gz->l_desc.dyn_tree = gz->dyn_ltree;
    gz->l_desc.static_tree = gz->static_ltree;
    gz->l_desc.extra_bits = gz->extra_lbits;
    gz->l_desc.extra_base = LITERALS+1;
    gz->l_desc.elems = L_CODES;
    gz->l_desc.max_length = MAX_BITS;
    gz->l_desc.max_code = 0;

    gz->d_desc.dyn_tree = gz->dyn_dtree;
    gz->d_desc.static_tree = gz->static_dtree;
    gz->d_desc.extra_bits = gz->extra_dbits;
    gz->d_desc.extra_base = 0;
    gz->d_desc.elems = D_CODES;
    gz->d_desc.max_length = MAX_BITS;
    gz->d_desc.max_code = 0;

    gz->bl_desc.dyn_tree = gz->bl_tree;
    gz->bl_desc.static_tree = NULL;
    gz->bl_desc.extra_bits = gz->extra_blbits;
    gz->bl_desc.extra_base = 0;
    gz->bl_desc.elems = BL_CODES;
    gz->bl_desc.max_length = MAX_BL_BITS;
    gz->bl_desc.max_code = 0;

    initCRC (gz->crc_32_tab);

    return gz;
}

/******************************************************************************
* freeZip.
******************************************************************************/

static void freeZip (gz)
GZp gz;
{
  cdf_FreeMemory (gz->head, NULL);
  cdf_FreeMemory (gz->prev, NULL);
  cdf_FreeMemory (gz->window, NULL);
  cdf_FreeMemory (gz->d_buf, NULL);
  cdf_FreeMemory (gz->outbuf, NULL);
  cdf_FreeMemory (gz->inbuf, NULL);
  cdf_FreeMemory (gz, NULL);
  return;
}

/******************************************************************************
* unzip.
*   Returns as soon as an error occurs.  Any memory allocated will be freed
* by the calling routine.  If successful, this routine frees the memory.
******************************************************************************/

/* unzip.c -- decompress files in gzip or pkzip format.
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 *
 * The code in this file is derived from the file funzip.c written
 * and put in the public domain by Mark Adler.
 */

/*
   This version can extract files in gzip or pkzip format.
   For the latter, only the first entry is extracted, and it has to be
   either deflated or stored.
 */

/* ===========================================================================
 * Unzip in to out.  This routine works on both gzip and pkzip files.
 *
 * IN assertions: the buffer inbuf contains already the beginning of
 *   the compressed data, from offsets inptr to insize-1 included.
 *   The magic header has already been checked. The output buffer is cleared.
 */

static CDFstatus unzip (iFp, oFp, iError, oError)
vFILE *iFp;
vFILE *oFp;
CDFstatus iError;
CDFstatus oError;
{
  ulg orig_crc = 0;       /* original crc */
  ulg orig_len = 0;       /* original uncompressed length */
  int n; uch buf[8], byte;
  /* uch flags1; */    /* compression flags */
  /* char magic[2]; */ /* magic header */
  /* ulg stamp; */     /* time stamp */
  int method; GUp gu;
  gu = initUnZip (iFp, oFp, iError, oError);
  if (gu == NULL) return BAD_MALLOC;
  if (!GetByte(gu,&byte)) return gu->iError;		/* magic[0]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* magic[1]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* method. */
  method = (int) byte;
  if (!GetByte(gu,&byte)) return gu->iError;		/* flags1. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* stamp[0]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* stamp[1]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* stamp[2]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* stamp[3]. */
  if (!GetByte(gu,&byte)) return gu->iError;		/* "extra flags". */
  if (!GetByte(gu,&byte)) return gu->iError;		/* "OS type". */
  updcrc (NULL, 0, &(gu->crcReg), gu->crc_32_tab);      /* initialize crc */
  /* Decompress */
  if (method == DEFLATED)  {
    switch (inflate(gu)) {
      case 0:
	break;  /* Success. */
      case 1:
      case 2:
	return DECOMPRESSION_ERROR;
      case 3:
	return BAD_MALLOC;
      case 4:
	return gu->iError;      /* Read error. */
      case 5:
	return gu->oError;      /* Write error. */
      default:
	return CDF_INTERNAL_ERROR;      /* ??? */
    }
  }
  else {
    /* error("internal error, invalid method"); */
    return DECOMPRESSION_ERROR;
  }
  /* Get the crc and original length */
  /* crc32  (see algorithm.doc)
   * uncompressed input size modulo 2^32
   */
  for (n = 0; n < 8; n++) {
     if (!GetByte(gu,&byte)) return gu->iError;
     buf[n] = (uch) byte;
  }
  orig_crc = LG(buf);
  orig_len = LG(buf+4);
  /* Validate decompression */
  if (orig_crc != updcrc(gu->window,0,&(gu->crcReg),gu->crc_32_tab)) {
						/* JTL: outbuf --> window */
    /* error("invalid compressed data--crc error"); */
    return DECOMPRESSION_ERROR;
  }
  if (orig_len != (ulg)gu->bytes_out) {
    /* error("invalid compressed data--length error"); */
    return DECOMPRESSION_ERROR;
  }
  freeUnZip (gu);
  return CDF_OK;
}

/******************************************************************************
* initUnZip.
*   Returns as soon as an error occurs.  Any memory allocated will be freed
* by the calling routine.
******************************************************************************/

static GUp initUnZip (iFp, oFp, iError, oError)
vFILE *iFp;
vFILE *oFp;
CDFstatus iError;
CDFstatus oError;
{
    GUp gu = (GUp) CallocateMemory ((size_t) 1, sizeof(GU), NULL);
    if (gu == NULL) return NULL;
    gu->inbuf = (uch *) CallocateMemory ((size_t) (INBUFSIZ+INBUF_EXTRA),
					 sizeof(uch), NULL);
    if (gu->inbuf == NULL) return NULL;
    gu->window = (uch *) CallocateMemory ((size_t) (2L*WSIZE), sizeof(uch),
					  NULL);
    if (gu->window == NULL) return NULL;

    gu->iFp = iFp;
    gu->oFp = oFp;
    gu->iError = iError;
    gu->oError = oError;
    gu->outcnt = 0;
    gu->bytes_in = 0;
    gu->bytes_out = 0;
    gu->insize = 0;
    gu->inptr = 0;
    gu->crcReg = (ulg) 0xffffffffL;

    gu->border[0] = 16;
    gu->border[1] = 17;
    gu->border[2] = 18;
    gu->border[3] = 0;
    gu->border[4] = 8;
    gu->border[5] = 7;
    gu->border[6] = 9;
    gu->border[7] = 6;
    gu->border[8] = 10;
    gu->border[9] = 5;
    gu->border[10] = 11;
    gu->border[11] = 4;
    gu->border[12] = 12;
    gu->border[13] = 3;
    gu->border[14] = 13;
    gu->border[15] = 2;
    gu->border[16] = 14;
    gu->border[17] = 1;
    gu->border[18] = 15;

    gu->cplens[0] = 3;
    gu->cplens[1] = 4;
    gu->cplens[2] = 5;
    gu->cplens[3] = 6;
    gu->cplens[4] = 7;
    gu->cplens[5] = 8;
    gu->cplens[6] = 9;
    gu->cplens[7] = 10;
    gu->cplens[8] = 11;
    gu->cplens[9] = 13;
    gu->cplens[10] = 15;
    gu->cplens[11] = 17;
    gu->cplens[12] = 19;
    gu->cplens[13] = 23;
    gu->cplens[14] = 27;
    gu->cplens[15] = 31;
    gu->cplens[16] = 35;
    gu->cplens[17] = 43;
    gu->cplens[18] = 51;
    gu->cplens[19] = 59;
    gu->cplens[20] = 67;
    gu->cplens[21] = 83;
    gu->cplens[22] = 99;
    gu->cplens[23] = 115;
    gu->cplens[24] = 131;
    gu->cplens[25] = 163;
    gu->cplens[26] = 195;
    gu->cplens[27] = 227;
    gu->cplens[28] = 258;
    gu->cplens[29] = 0;
    gu->cplens[30] = 0;

    gu->cplext[0] = 0;
    gu->cplext[1] = 0;
    gu->cplext[2] = 0;
    gu->cplext[3] = 0;
    gu->cplext[4] = 0;
    gu->cplext[5] = 0;
    gu->cplext[6] = 0;
    gu->cplext[7] = 0;
    gu->cplext[8] = 1;
    gu->cplext[9] = 1;
    gu->cplext[10] = 1;
    gu->cplext[11] = 1;
    gu->cplext[12] = 2;
    gu->cplext[13] = 2;
    gu->cplext[14] = 2;
    gu->cplext[15] = 2;
    gu->cplext[16] = 3;
    gu->cplext[17] = 3;
    gu->cplext[18] = 3;
    gu->cplext[19] = 3;
    gu->cplext[20] = 4;
    gu->cplext[21] = 4;
    gu->cplext[22] = 4;
    gu->cplext[23] = 4;
    gu->cplext[24] = 5;
    gu->cplext[25] = 5;
    gu->cplext[26] = 5;
    gu->cplext[27] = 5;
    gu->cplext[28] = 0;
    gu->cplext[29] = 99;
    gu->cplext[30] = 99;

    gu->cpdist[0] = 1;
    gu->cpdist[1] = 2;
    gu->cpdist[2] = 3;
    gu->cpdist[3] = 4;
    gu->cpdist[4] = 5;
    gu->cpdist[5] = 7;
    gu->cpdist[6] = 9;
    gu->cpdist[7] = 13;
    gu->cpdist[8] = 17;
    gu->cpdist[9] = 25;
    gu->cpdist[10] = 33;
    gu->cpdist[11] = 49;
    gu->cpdist[12] = 65;
    gu->cpdist[13] = 97;
    gu->cpdist[14] = 129;
    gu->cpdist[15] = 193;
    gu->cpdist[16] = 257;
    gu->cpdist[17] = 385;
    gu->cpdist[18] = 513;
    gu->cpdist[19] = 769;
    gu->cpdist[20] = 1025;
    gu->cpdist[21] = 1537;
    gu->cpdist[22] = 2049;
    gu->cpdist[23] = 3073;
    gu->cpdist[24] = 4097;
    gu->cpdist[25] = 6145;
    gu->cpdist[26] = 8193;
    gu->cpdist[27] = 12289;
    gu->cpdist[28] = 16385;
    gu->cpdist[29] = 24577;

    gu->cpdext[0] = 0;
    gu->cpdext[1] = 0;
    gu->cpdext[2] = 0;
    gu->cpdext[3] = 0;
    gu->cpdext[4] = 1;
    gu->cpdext[5] = 1;
    gu->cpdext[6] = 2;
    gu->cpdext[7] = 2;
    gu->cpdext[8] = 3;
    gu->cpdext[9] = 3;
    gu->cpdext[10] = 4;
    gu->cpdext[11] = 4;
    gu->cpdext[12] = 5;
    gu->cpdext[13] = 5;
    gu->cpdext[14] = 6;
    gu->cpdext[15] = 6;
    gu->cpdext[16] = 7;
    gu->cpdext[17] = 7;
    gu->cpdext[18] = 8;
    gu->cpdext[19] = 8;
    gu->cpdext[20] = 9;
    gu->cpdext[21] = 9;
    gu->cpdext[22] = 10;
    gu->cpdext[23] = 10;
    gu->cpdext[24] = 11;
    gu->cpdext[25] = 11;
    gu->cpdext[26] = 12;
    gu->cpdext[27] = 12;
    gu->cpdext[28] = 13;
    gu->cpdext[29] = 13;

    gu->mask_bits[0] = 0x0000;
    gu->mask_bits[1] = 0x0001;
    gu->mask_bits[2] = 0x0003;
    gu->mask_bits[3] = 0x0007;
    gu->mask_bits[4] = 0x000f;
    gu->mask_bits[5] = 0x001f;
    gu->mask_bits[6] = 0x003f;
    gu->mask_bits[7] = 0x007f;
    gu->mask_bits[8] = 0x00ff;
    gu->mask_bits[9] = 0x01ff;
    gu->mask_bits[10] = 0x03ff;
    gu->mask_bits[11] = 0x07ff;
    gu->mask_bits[12] = 0x0fff;
    gu->mask_bits[13] = 0x1fff;
    gu->mask_bits[14] = 0x3fff;
    gu->mask_bits[15] = 0x7fff;
    gu->mask_bits[16] = 0xffff;

    initCRC (gu->crc_32_tab);

    return gu;
}

/******************************************************************************
* freeUnZip.
******************************************************************************/

static void freeUnZip (gu)
GUp gu;
{
  cdf_FreeMemory (gu->window, NULL);
  cdf_FreeMemory (gu->inbuf, NULL);
  cdf_FreeMemory (gu, NULL);
  return;
}

/* ===========================================================================
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
/* Returns:     >0      number of bytes read,   - JTL...
		0       fake EOF,
		-1      error or real EOF.
*/

static int file_read(gz, buf, size)
    GZp gz;
    char *buf;
    unsigned size;
{
    unsigned len;
    long remaining = gz->ifile_size - gz->bytes_in; /* JTL... */
    if (remaining == 0) return 0;                       /* Fake `EOF'. */
    size = (unsigned) MINIMUM((long)size,remaining);        /* ...JTL */
    len = (unsigned) V_read (buf, (size_t) 1, (size_t) size, gz->iFp);
    if (len == 0) return (int)(-1);     /* V_read returns 0 if an error. */
    gz->crc = updcrc ((uch*) buf, len, &(gz->crcReg), gz->crc_32_tab);
    gz->bytes_in += (ulg)len;
    return (int)len;
}



/* bits.c -- output variable-length bit strings
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */


/*
 *  PURPOSE
 *
 *      Output variable-length bit strings. Compression can be done
 *      to a file or to memory. (The latter is not supported in this version.)
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflate" file format interprets compressed file data
 *      as a sequence of bits.  Multi-bit strings in the file may cross
 *      byte boundaries without restriction.
 *
 *      The first bit of each byte is the low-order bit.
 *
 *      The routines in this file allow a variable-length bit value to
 *      be output right-to-left (useful for literal values). For
 *      left-to-right output (useful for code strings from the tree routines),
 *      the bits must have been reversed first with bi_reverse().
 *
 *      For in-memory compression, the compressed bit stream goes directly
 *      into the requested output buffer. The input data is read in blocks
 *      by the mem_read() function. The buffer is limited to 64K on 16 bit
 *      machines.
 *
 *  INTERFACE
 *
 *      void bi_init (GZp gz)
 *          Initialize the bit string routines.
 *
 *      Logical send_bits (GZp gz, int value, int length)
 *          Write out a bit string, taking the source bits right to
 *          left.
 *
 *      int bi_reverse (int value, int length)
 *          Reverse the bits of a bit string, taking the source bits left to
 *          right and emitting them right to left.
 *
 *      Logical bi_windup (GZp gz)
 *          Write out any remaining bits in an incomplete byte.
 *
 *      Logical copy_block(GZp gz, char *buf, unsigned len, int header)
 *          Copy a stored block to the zip file, storing first the length and
 *          its one's complement if requested.
 *
 */

/* ===========================================================================
 * Initialize the bit string routines.
 */
static void bi_init (gz)
GZp gz;
{
    gz->bi_buf = 0;
    gz->bi_valid = 0;
}

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
static Logical send_bits(gz, value, length)
    GZp gz;
    int value;  /* value to send */
    int length; /* number of bits */
{
    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
     * unused bits in value.
     */
    if (gz->bi_valid > (int)BUF_SIZE - length) {
	gz->bi_buf |= (value << gz->bi_valid);
	if (!PutShort(gz,(unsigned)gz->bi_buf)) return FALSE;
	gz->bi_buf = (ush)value >> (BUF_SIZE - gz->bi_valid);
	gz->bi_valid += length - BUF_SIZE;
    } else {
	gz->bi_buf |= value << gz->bi_valid;
	gz->bi_valid += length;
    }
    return TRUE;
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
static unsigned bi_reverse(code, len)
    unsigned code; /* the value to invert */
    int len;       /* its bit length */
{
    register unsigned res = 0;
    do {
	res |= code & 1;
	code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* ===========================================================================
 * Write out any remaining bits in an incomplete byte.
 */
static Logical bi_windup(gz)
GZp gz;
{
    if (gz->bi_valid > 8) {
	if (!PutShort(gz,(unsigned)gz->bi_buf)) return FALSE;
    } else if (gz->bi_valid > 0) {
	if (!PutByte(gz,(unsigned)gz->bi_buf)) return FALSE;
    }
    gz->bi_buf = 0;
    gz->bi_valid = 0;
    return TRUE;
}

/* ===========================================================================
 * Copy a stored block to the zip file, storing first the length and its
 * one's complement if requested.
 */
static Logical copy_block(gz, buf, len, header)
    GZp gz;
    char     *buf;    /* the input data */
    unsigned len;     /* its length */
    int      header;  /* true if block header must be written */
{
    if (!bi_windup(gz)) return FALSE;              /* align on byte boundary */
    if (header) {
	if (!PutShort(gz,(unsigned)len)) return FALSE;
	if (!PutShort(gz,(unsigned)~len)) return FALSE;
    }
    while (len--) {
	if (!PutByte(gz,(unsigned)*buf++)) return FALSE;
    }
    return TRUE;
}


/* deflate.c -- compress data using the deflation algorithm
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/*
 *  PURPOSE
 *
 *      Identify new text as repetitions of old text within a fixed-
 *      length sliding window trailing behind the new text.
 *
 *  DISCUSSION
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many info-zippers for bug reports and testing.
 *
 *  REFERENCES
 *
 *      APPNOTE.TXT documentation file in PKZIP 1.93a distribution.
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 *  INTERFACE
 *
 *      Logical lm_init (GZp gz, ush *flags)
 *          Initialize the "longest match" routines for a new file
 *
 *      ulg deflate (GZp gz)
 *          Processes a new input file and return its compressed length. Sets
 *          the compressed length, crc, deflate flags and internal file
 *          attributes.
 */

/* ===========================================================================
 * Initialize the "longest match" routines for a new file
 */
static Logical lm_init (gz, flags)
    GZp gz;
    ush *flags;     /* general purpose bit flag */
{
    register unsigned j;

    /* Initialize the hash table. */
#if HASH_BITS == 15
    for (j = 0;  j < HASH_SIZE; j++) gz->head[j] = NIL;
#else
    memzero((char *)gz->head, HASH_SIZE * sizeof(*gz->head));
#endif
    /* prev will be initialized on the fly */

    /* Set the default configuration parameters:
     */
    if (gz->level == 1) {
       *flags |= FAST;
    } else if (gz->level == 9) {
       *flags |= SLOW;
    }
    /* ??? reduce max_chain_length for binary files */

    gz->strstart = 0;
    gz->block_start = 0L;

    gz->lookahead = file_read(gz, (char *) gz->window,
			  sizeof(int) <= 2 ? (unsigned)WSIZE : 2*WSIZE);
    if (gz->lookahead == (unsigned)(-1)) {
      return FALSE;             /* Error/real EOF shouldn't ever occur - JTL */
    }
    if (gz->lookahead == 0) {
       gz->eofile = 1, gz->lookahead = 0;
       return TRUE;
    }
    gz->eofile = 0;
    /* Make sure that we always have enough lookahead. This is important
     * if input comes from a device such as a tty.
     */
    while (gz->lookahead < MIN_LOOKAHEAD && !gz->eofile) {
      if (!fill_window(gz)) return FALSE;
    }

    gz->ins_h = 0;
    for (j=0; j<MIN_MATCH-1; j++) {
       gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[j]) & HASH_MASK;
    }
    /* If lookahead < MIN_MATCH, gz->ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
    return TRUE;
}

/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */

static int longest_match(gz, cur_match)
    GZp gz;
    IPos cur_match;                             /* current match */
{
    unsigned chain_length = gz->configuration_table[gz->level].max_chain;
						/* max hash chain length */
    register uch *scan = gz->window + gz->strstart;     /* current string */
    register uch *match;                        /* matched string */
    register int len;                           /* length of current match */
    int best_len = gz->prev_length;                 /* best match length so far */
    IPos limit = gz->strstart > (IPos)MAX_DIST ? gz->strstart - (IPos)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */

/* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
 * It is easy to get rid of this optimization if necessary.
 */
#if HASH_BITS < 8 || MAX_MATCH != 258
   error: Code too clever
#endif

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register uch *strend = gz->window + gz->strstart + MAX_MATCH - 1;
    register ush scan_start = *(ush*)scan;
    register ush scan_end   = *(ush*)(scan+best_len-1);
#else
    register uch *strend = gz->window + gz->strstart + MAX_MATCH;
    register uch scan_end1  = scan[best_len-1];
    register uch scan_end   = scan[best_len];
#endif

    /* Do not waste too much time if we already have a good match: */
    if (gz->prev_length >= gz->configuration_table[gz->level].good_length) {
	chain_length >>= 2;
    }

    do {
	match = gz->window + cur_match;

	/* Skip to next match if the match length cannot increase
	 * or if the match length is less than 2:
	 */
#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)
	/* This code assumes sizeof(unsigned short) == 2. Do not use
	 * UNALIGNED_OK if your compiler uses a different size.
	 */
	if (*(ush*)(match+best_len-1) != scan_end ||
	    *(ush*)match != scan_start) continue;

	/* It is not necessary to compare scan[2] and match[2] since they are
	 * always equal when the other bytes match, given that the hash keys
	 * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
	 * strstart+3, +5, ... up to strstart+257. We check for insufficient
	 * lookahead only every 4th comparison; the 128th check will be made
	 * at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
	 * necessary to put more guard bytes at the end of the window, or
	 * to check more often for insufficient lookahead.
	 */
	scan++, match++;
	do {
	} while (*(ush*)(scan+=2) == *(ush*)(match+=2) &&
		 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
		 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
		 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
		 scan < strend);
	/* The funny "do {}" generates better code on most compilers */

	/* Here, scan <= window+strstart+257 */
	if (*scan == *match) scan++;

	len = (MAX_MATCH - 1) - (int)(strend-scan);
	scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

	if (match[best_len]   != scan_end  ||
	    match[best_len-1] != scan_end1 ||
	    *match            != *scan     ||
	    *++match          != scan[1])      continue;

	/* The check at best_len-1 can be removed because it will be made
	 * again later. (This heuristic is not always a win.)
	 * It is not necessary to compare scan[2] and match[2] since they
	 * are always equal when the other bytes match, given that
	 * the hash keys are equal and that HASH_BITS >= 8.
	 */
	scan += 2, match++;

	/* We check for insufficient lookahead only every 8th comparison;
	 * the 256th check will be made at strstart+258.
	 */
	do {
	} while (*++scan == *++match && *++scan == *++match &&
		 *++scan == *++match && *++scan == *++match &&
		 *++scan == *++match && *++scan == *++match &&
		 *++scan == *++match && *++scan == *++match &&
		 scan < strend);

	len = MAX_MATCH - (int)(strend - scan);
	scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

	if (len > best_len) {
	    gz->match_start = cur_match;
	    best_len = len;
	    if (len >=
		(int) gz->configuration_table[gz->level].nice_length) break;
#ifdef UNALIGNED_OK
	    scan_end = *(ush*)(scan+best_len-1);
#else
	    scan_end1  = scan[best_len-1];
	    scan_end   = scan[best_len];
#endif
	}
    } while ((cur_match = gz->prev[cur_match & WMASK]) > limit
	     && --chain_length != 0);

    return best_len;
}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of input file.
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: at least one byte has been read, or eofile is set;
 *    file reads are performed for at least two bytes (required for the
 *    translate_eol option).
 */
static Logical fill_window(gz)
GZp gz;
{
    register unsigned n, m;
    unsigned more = (unsigned)(WINDOW_SIZE - (ulg)gz->lookahead - (ulg)gz->strstart);
    /* Amount of free space at the end of the window. */

    /* If the window is almost full and there is insufficient lookahead,
     * move the upper half to the lower one to make room in the upper half.
     */
    if (more == (unsigned)EOF) {
	/* Very unlikely, but possible on 16 bit machine if strstart == 0
	 * and lookahead == 1 (input done one byte at time)
	 */
	more--;
    } else if (gz->strstart >= WSIZE+MAX_DIST) {
	/* By the IN assertion, the window is not empty so we can't confuse
	 * more == 0 with more == 64K on a 16 bit machine.
	 */
	memcpy((char*)gz->window, (char*)gz->window+WSIZE, (unsigned)WSIZE);
	gz->match_start -= WSIZE;
	gz->strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */
	gz->block_start -= (long) WSIZE;
	for (n = 0; n < HASH_SIZE; n++) {
	    m = gz->head[n];
	    gz->head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
	}
	for (n = 0; n < WSIZE; n++) {
	    m = gz->prev[n];
	    gz->prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
	    /* If n is not on any hash chain, prev[n] is garbage but
	     * its value will never be used.
	     */
	}
	more += WSIZE;
    }
    /* At this point, more >= 2 */
    if (!gz->eofile) {
	n = file_read(gz, (char*)gz->window+gz->strstart+gz->lookahead, more);
	if (n == (unsigned)(-1)) {
	  return FALSE;         /* Error/real EOF should never occur - JTL */
	}
	if (n == 0) {
	    gz->eofile = 1;
	}
	else {
	    gz->lookahead += n;
	}
    }
    return TRUE;
}

/* ===========================================================================
 * Processes a new input file and return its compressed length. This
 * function does not perform lazy evaluationof matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
static CDFstatus deflate_fast(gz)
GZp gz;
{
    IPos hash_head; /* head of the hash chain */
    int flush;      /* set if current block must be flushed */
    unsigned match_length = 0;  /* length of best match */

    gz->prev_length = MIN_MATCH-1;
    while (gz->lookahead != 0) {
	/* Insert the string window[strstart .. strstart+2] in the
	 * dictionary, and set hash_head to the head of the hash chain:
	 */
	gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[(gz->strstart) + MIN_MATCH-1]) & HASH_MASK;
	gz->prev[(gz->strstart) & WMASK] = hash_head = gz->head[gz->ins_h];
	gz->head[gz->ins_h] = (gz->strstart);

	/* Find the longest match, discarding those <= prev_length.
	 * At this point we have always match_length < MIN_MATCH
	 */
	if (hash_head != NIL && gz->strstart - hash_head <= MAX_DIST) {
	    /* To simplify the code, we prevent matches with the string
	     * of window index 0 (in particular we have to avoid a match
	     * of the string with itself at the start of the input file).
	     */
	    match_length = longest_match (gz, hash_head);
	    /* longest_match() sets match_start */
	    if (match_length > gz->lookahead) match_length = gz->lookahead;
	}
	if (match_length >= MIN_MATCH) {
	    flush = ct_tally (gz, gz->strstart - gz->match_start,
			      match_length - MIN_MATCH);
	    gz->lookahead -= match_length;
	    /* Insert new strings in the hash table only if the match length
	     * is not too large. This saves time but degrades compression.
	     */
	    if (match_length <= gz->configuration_table[gz->level].max_lazy) {
		match_length--; /* string at strstart already in hash table */
		do {
		    gz->strstart++;
		    gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[(gz->strstart) +
			     MIN_MATCH-1]) & HASH_MASK;
		    gz->prev[(gz->strstart) & WMASK] = hash_head = gz->head[gz->ins_h];
		    gz->head[gz->ins_h] = (gz->strstart);

		    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
		     * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
		     * these bytes are garbage, but it does not matter since
		     * the next lookahead bytes will be emitted as literals.
		     */
		} while (--match_length != 0);
		gz->strstart++; 
	    } else {
		gz->strstart += match_length;
		match_length = 0;
		gz->ins_h = gz->window[gz->strstart];
		gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[gz->strstart+1]) & HASH_MASK;
	    }
	} else {
	    /* No match, output a literal byte */
	    flush = ct_tally (gz, 0, gz->window[gz->strstart]);
	    gz->lookahead--;
	    gz->strstart++; 
	}
	if (flush) {
	  if (!flush_block(gz, gz->block_start >= 0L ?
			   (char*) &(gz->window[(unsigned)gz->block_start]) :
			   (char*) NULL,
			   (long) gz->strstart - gz->block_start,
			   0)) return gz->oError;       /* Write error. */
	  gz->block_start = gz->strstart;
	}

	/* Make sure that we always have enough lookahead, except
	 * at the end of the input file. We need MAX_MATCH bytes
	 * for the next match, plus MIN_MATCH bytes to insert the
	 * string following the next match.
	 */
	while (gz->lookahead < MIN_LOOKAHEAD && !gz->eofile) {
	  if (!fill_window(gz)) return gz->iError;      /* Read error. */
	}
    }
    if (!flush_block(gz, gz->block_start >= 0L ?
		     (char *) &(gz->window[(unsigned)gz->block_start]) :
		     (char *) NULL,
		     (long) gz->strstart - gz->block_start,
		     1)) return gz->oError; /* Write error. */  /* eof */
    return CDF_OK;
}

/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
static CDFstatus deflate(gz)
GZp gz;
{
    IPos hash_head;          /* head of hash chain */
    IPos prev_match;         /* previous match */
    int flush;               /* set if current block must be flushed */
    int match_available = 0; /* set if previous match exists */
    register unsigned match_length = MIN_MATCH-1; /* length of best match */

    if (gz->level <= 3) return deflate_fast(gz); /* optimized for speed */

    /* Process the input block. */
    while (gz->lookahead != 0) {
	/* Insert the string window[strstart .. strstart+2] in the
	 * dictionary, and set hash_head to the head of the hash chain:
	 */
	gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[(gz->strstart) + MIN_MATCH-1]) &
		HASH_MASK;
	gz->prev[(gz->strstart) & WMASK] = hash_head = gz->head[gz->ins_h];
	gz->head[gz->ins_h] = (gz->strstart);

	/* Find the longest match, discarding those <= prev_length.
	 */
	gz->prev_length = match_length, prev_match = gz->match_start;
	match_length = MIN_MATCH-1;

	if (hash_head != NIL &&
	    gz->prev_length < gz->configuration_table[gz->level].max_lazy &&
	    gz->strstart - hash_head <= MAX_DIST) {
	    /* To simplify the code, we prevent matches with the string
	     * of window index 0 (in particular we have to avoid a match
	     * of the string with itself at the start of the input file).
	     */
	    match_length = longest_match (gz, hash_head);
	    /* longest_match() sets match_start */
	    if (match_length > gz->lookahead) match_length = gz->lookahead;

	    /* Ignore a length 3 match if it is too distant: */
	    if (match_length == MIN_MATCH && gz->strstart-gz->match_start > TOO_FAR){
		/* If prev_match is also MIN_MATCH, match_start is garbage
		 * but we will ignore the current match anyway.
		 */
		match_length--;
	    }
	}
	/* If there was a match at the previous step and the current
	 * match is not better, output the previous match:
	 */
	if (gz->prev_length >= MIN_MATCH && match_length <= gz->prev_length) {
	    flush = ct_tally (gz, gz->strstart - 1 - prev_match,
			      gz->prev_length - MIN_MATCH);
	    /* Insert in hash table all strings up to the end of the match.
	     * strstart-1 and strstart are already inserted.
	     */
	    gz->lookahead -= gz->prev_length-1;
	    gz->prev_length -= 2;
	    do {
		gz->strstart++;
		gz->ins_h = ((gz->ins_h << H_SHIFT) ^ gz->window[(gz->strstart) + MIN_MATCH-1]) & HASH_MASK;
		gz->prev[(gz->strstart) & WMASK] = hash_head = gz->head[gz->ins_h];
		gz->head[gz->ins_h] = (gz->strstart);

		/* strstart never exceeds WSIZE-MAX_MATCH, so there are
		 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
		 * these bytes are garbage, but it does not matter since the
		 * next lookahead bytes will always be emitted as literals.
		 */
	    } while (--gz->prev_length != 0);
	    match_available = 0;
	    match_length = MIN_MATCH-1;
	    gz->strstart++;
	    if (flush) {
	      if (!flush_block(gz, gz->block_start >= 0L ?
			       (char *) &(gz->window[(unsigned)gz->block_start]) :
			       (char *) NULL, (long) gz->strstart - gz->block_start,
			       0)) return gz->oError;   /* Write error. */
	      gz->block_start = gz->strstart;
	    }
	} else if (match_available) {
	    /* If there was no match at the previous position, output a
	     * single literal. If there was a match but the current match
	     * is longer, truncate the previous match to a single literal.
	     */
	    if (ct_tally (gz, 0, gz->window[gz->strstart-1])) {
	      if (!flush_block(gz, gz->block_start >= 0L ?
			       (char *) &(gz->window[(unsigned)gz->block_start]) :
			       (char *) NULL, (long) gz->strstart - gz->block_start,
			       0)) return gz->oError;   /* Write error. */
	      gz->block_start = gz->strstart;
	    }
	    gz->strstart++;
	    gz->lookahead--;
	} else {
	    /* There is no previous match to compare with, wait for
	     * the next step to decide.
	     */
	    match_available = 1;
	    gz->strstart++;
	    gz->lookahead--;
	}

	/* Make sure that we always have enough lookahead, except
	 * at the end of the input file. We need MAX_MATCH bytes
	 * for the next match, plus MIN_MATCH bytes to insert the
	 * string following the next match.
	 */
	while (gz->lookahead < MIN_LOOKAHEAD && !gz->eofile) {
	  if (!fill_window(gz)) return gz->iError;      /* Read error. */
	}
    }
    if (match_available) ct_tally (gz, 0, gz->window[gz->strstart-1]);

    if (!flush_block(gz, gz->block_start >= 0L ?
		     (char *) &(gz->window[(unsigned)gz->block_start]) :
		     (char *) NULL,
		     (long) gz->strstart - gz->block_start,
		     1)) return gz->oError; /* Write error. */  /* eof */
    return CDF_OK;
}



/* trees.c -- output deflated data using Huffman coding
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/*
 *  PURPOSE
 *
 *      Encode various sets of source values using variable-length
 *      binary code trees.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in the ZIP file in a compressed form
 *      which is itself a Huffman encoding of the lengths of
 *      all the code strings (in ascending order by source values).
 *      The actual code strings are reconstructed from the lengths in
 *      the UNZIP process, as described in the "application note"
 *      (APPNOTE.TXT) distributed as part of PKWARE's PKZIP program.
 *
 *  REFERENCES
 *
 *      Lynch, Thomas J.
 *          Data Compression:  Techniques and Applications, pp. 53-55.
 *          Lifetime Learning Publications, 1985.  ISBN 0-534-03418-7.
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 *
 *  INTERFACE
 *
 *      void ct_init (GZp gz)
 *          Allocate the match buffer, initialize the various tables.
 *
 *      void ct_tally (GZp gz, int dist, int lc);
 *          Save the match info and tally the frequency counts.
 *
 *      Logical flush_block (GZp gz, char *buf, ulg stored_len, int eof)
 *          Determine the best encoding for the current block: dynamic trees,
 *          static trees or store, and output the encoded block to the zip
 *          file.
 *
 */

/* ===========================================================================
 * Allocate the match buffer and initialize the various tables.
 */
static void ct_init(gz)
GZp gz;
{
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */
 
    if (gz->static_dtree[0].dl.len != 0) return; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
	gz->base_length[code] = length;
	for (n = 0; n < (1<<gz->extra_lbits[code]); n++) {
	    gz->length_code[length++] = (uch)code;
	}
    }
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    gz->length_code[length-1] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
	gz->base_dist[code] = dist;
	for (n = 0; n < (1<<gz->extra_dbits[code]); n++) {
	    gz->dist_code[dist++] = (uch)code;
	}
    }
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++) {
	gz->base_dist[code] = dist << 7;
	for (n = 0; n < (1<<(gz->extra_dbits[code]-7)); n++) {
	    gz->dist_code[256 + dist++] = (uch)code;
	}
    }

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits++) gz->bl_count[bits] = 0;
    n = 0;
    while (n <= 143) gz->static_ltree[n++].dl.len = 8, gz->bl_count[8]++;
    while (n <= 255) gz->static_ltree[n++].dl.len = 9, gz->bl_count[9]++;
    while (n <= 279) gz->static_ltree[n++].dl.len = 7, gz->bl_count[7]++;
    while (n <= 287) gz->static_ltree[n++].dl.len = 8, gz->bl_count[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes(gz, (ct_data *)gz->static_ltree, L_CODES+1);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n++) {
	gz->static_dtree[n].dl.len = 5;
	gz->static_dtree[n].fc.code = bi_reverse(n, 5);
    }

    /* Initialize the first block of the first file: */
    init_block(gz);
}

/* ===========================================================================
 * Initialize a new block.
 */
static void init_block(gz)
GZp gz;
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES;  n++) gz->dyn_ltree[n].fc.freq = 0;
    for (n = 0; n < D_CODES;  n++) gz->dyn_dtree[n].fc.freq = 0;
    for (n = 0; n < BL_CODES; n++) gz->bl_tree[n].fc.freq = 0;

    gz->dyn_ltree[END_BLOCK].fc.freq = 1;
    gz->opt_len = gz->static_len = 0L;
    gz->last_lit = gz->last_dist = gz->last_flags = 0;
    gz->tree_flags = 0; gz->flag_bit = 1;
    return;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(tree_, depth_, n_, m_) \
   (tree_[n_].fc.freq < tree_[m_].fc.freq || \
   (tree_[n_].fc.freq == tree_[m_].fc.freq && depth_[n_] <= depth_[m_]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
static void pqdownheap (gz, tree, k)
    GZp gz;
    ct_data *tree;  /* the tree to restore */
    int k;               /* node to move down */
{
    int v = gz->heap[k];
    int j = k << 1;  /* left son of k */
    while (j <= gz->heap_len) {
	/* Set j to the smallest of the two sons: */
	if (j < gz->heap_len && smaller(tree, gz->depth, gz->heap[j+1], gz->heap[j])) j++;

	/* Exit if v is smaller than both sons */
	if (smaller(tree, gz->depth, v, gz->heap[j])) break;

	/* Exchange v with the smallest son */
	gz->heap[k] = gz->heap[j];  k = j;

	/* And continue down the tree, setting j to the left son of k */
	j <<= 1;
    }
    gz->heap[k] = v;
    return;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
static void gen_bitlen(gz, desc)
    GZp gz;
    tree_desc *desc; /* the tree descriptor */
{
    ct_data *tree  = desc->dyn_tree;
    int *extra     = desc->extra_bits;
    int base            = desc->extra_base;
    int max_code        = desc->max_code;
    int max_length      = desc->max_length;
    ct_data *stree = desc->static_tree;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    ush f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++) gz->bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[gz->heap[gz->heap_max]].dl.len = 0; /* root of the heap */

    for (h = gz->heap_max+1; h < HEAP_SIZE; h++) {
	n = gz->heap[h];
	bits = tree[tree[n].dl.dad].dl.len + 1;
	if (bits > max_length) bits = max_length, overflow++;
	tree[n].dl.len = (ush)bits;
	/* We overwrite tree[n].dl.dad which is no longer needed */

	if (n > max_code) continue; /* not a leaf node */

	gz->bl_count[bits]++;
	xbits = 0;
	if (n >= base) xbits = extra[n-base];
	f = tree[n].fc.freq;
	gz->opt_len += (ulg)f * (bits + xbits);
	if (stree) gz->static_len += (ulg)f * (stree[n].dl.len + xbits);
    }
    if (overflow == 0) return;

    /* Find the first bit length which could increase: */
    do {
	bits = max_length-1;
	while (gz->bl_count[bits] == 0) bits--;
	gz->bl_count[bits]--;      /* move one leaf down the tree */
	gz->bl_count[bits+1] += 2; /* move one overflow item as its brother */
	gz->bl_count[max_length]--;
	/* The brother of the overflow item also moves one step up,
	 * but this does not affect bl_count[max_length]
	 */
	overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
	n = gz->bl_count[bits];
	while (n != 0) {
	    m = gz->heap[--h];
	    if (m > max_code) continue;
	    if (tree[m].dl.len != (unsigned) bits) {
		gz->opt_len += ((long)bits-(long)tree[m].dl.len) *
			   (long)tree[m].fc.freq;
		tree[m].dl.len = (ush)bits;
	    }
	    n--;
	}
    }
    return;
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
static void gen_codes (gz, tree, max_code)
    GZp gz;
    ct_data *tree;        /* the tree to decorate */
    int max_code;              /* largest code with non zero frequency */
{
    ush next_code[MAX_BITS+1]; /* next code value for each bit length */
    ush code = 0;              /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
	next_code[bits] = code = (code + gz->bl_count[bits-1]) << 1;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */

    for (n = 0;  n <= max_code; n++) {
	int len = tree[n].dl.len;
	if (len == 0) continue;
	/* Now reverse the bits */
	tree[n].fc.code = bi_reverse(next_code[len]++, len);
    }
    return;
}

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
static void build_tree_gz(gz, desc)
    GZp gz;
    tree_desc *desc; /* the tree descriptor */
{
    ct_data *tree   = desc->dyn_tree;
    ct_data *stree  = desc->static_tree;
    int elems            = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    gz->heap_len = 0, gz->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
	if (tree[n].fc.freq != 0) {
	    gz->heap[++gz->heap_len] = max_code = n;
	    gz->depth[n] = 0;
	} else {
	    tree[n].dl.len = 0;
	}
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (gz->heap_len < 2) {
	int new = gz->heap[++gz->heap_len] = (max_code < 2 ? ++max_code : 0);
	tree[new].fc.freq = 1;
	gz->depth[new] = 0;
	gz->opt_len--; if (stree) gz->static_len -= stree[new].dl.len;
	/* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = gz->heap_len/2; n >= 1; n--) pqdownheap (gz, tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
	n = gz->heap[SMALLEST];
	gz->heap[SMALLEST] = gz->heap[gz->heap_len--];
	pqdownheap (gz, tree, SMALLEST);
			     /* n = node of least frequency */
	m = gz->heap[SMALLEST];  /* m = node of next least frequency */

	gz->heap[--gz->heap_max] = n; /* keep the nodes sorted by frequency */
	gz->heap[--gz->heap_max] = m;

	/* Create a new node father of n and m */
	tree[node].fc.freq = tree[n].fc.freq + tree[m].fc.freq;
	gz->depth[node] = (uch) (MAXIMUM(gz->depth[n],gz->depth[m]) + 1);
	tree[n].dl.dad = tree[m].dl.dad = (ush)node;
	/* and insert the new node in the heap */
	gz->heap[SMALLEST] = node++;
	pqdownheap (gz, tree, SMALLEST);

    } while (gz->heap_len >= 2);

    gz->heap[--gz->heap_max] = gz->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(gz, (tree_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes (gz, (ct_data *)tree, max_code);

    return;
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
static void scan_tree (gz, tree, max_code)
    GZp gz;
    ct_data *tree; /* the tree to be scanned */
    int max_code;       /* and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].dl.len = (ush)0xffff; /* guard */

    for (n = 0; n <= max_code; n++) {
	curlen = nextlen; nextlen = tree[n+1].dl.len;
	if (++count < max_count && curlen == nextlen) {
	    continue;
	} else if (count < min_count) {
	    gz->bl_tree[curlen].fc.freq += count;
	} else if (curlen != 0) {
	    if (curlen != prevlen) gz->bl_tree[curlen].fc.freq++;
	    gz->bl_tree[REP_3_6].fc.freq++;
	} else if (count <= 10) {
	    gz->bl_tree[REPZ_3_10].fc.freq++;
	} else {
	    gz->bl_tree[REPZ_11_138].fc.freq++;
	}
	count = 0; prevlen = curlen;
	if (nextlen == 0) {
	    max_count = 138, min_count = 3;
	} else if (curlen == nextlen) {
	    max_count = 6, min_count = 3;
	} else {
	    max_count = 7, min_count = 4;
	}
    }
    return;
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
static Logical send_tree (gz, tree, max_code)
    GZp gz;
    ct_data *tree; /* the tree to be scanned */
    int max_code;       /* and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].dl.len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].dl.len = -1; */  /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
	curlen = nextlen; nextlen = tree[n+1].dl.len;
	if (++count < max_count && curlen == nextlen) {
	    continue;
	} else if (count < min_count) {
	    do {
	      if (!send_code(gz,curlen,gz->bl_tree)) return FALSE;
	    } while (--count != 0);
	} else if (curlen != 0) {
	    if (curlen != prevlen) {
		if (!send_code(gz,curlen,gz->bl_tree)) return FALSE;
		count--;
	    }
	    if (!send_code(gz,REP_3_6,gz->bl_tree)) return FALSE;
	    if (!send_bits(gz,count-3,2)) return FALSE;

	} else if (count <= 10) {
	    if (!send_code(gz,REPZ_3_10,gz->bl_tree)) return FALSE;
	    if (!send_bits(gz,count-3,3)) return FALSE;

	} else {
	    if (!send_code(gz,REPZ_11_138,gz->bl_tree)) return FALSE;
	    if (!send_bits(gz,count-11,7)) return FALSE;
	}
	count = 0; prevlen = curlen;
	if (nextlen == 0) {
	    max_count = 138, min_count = 3;
	} else if (curlen == nextlen) {
	    max_count = 6, min_count = 3;
	} else {
	    max_count = 7, min_count = 4;
	}
    }
    return TRUE;
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
static int build_bl_tree(gz)
GZp gz;
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree(gz, (ct_data *)gz->dyn_ltree, gz->l_desc.max_code);
    scan_tree(gz, (ct_data *)gz->dyn_dtree, gz->d_desc.max_code);

    /* Build the bit length tree: */
    build_tree_gz(gz, (tree_desc *)(&(gz->bl_desc)));
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
	if (gz->bl_tree[gz->bl_order[max_blindex]].dl.len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    gz->opt_len += 3*(max_blindex+1) + 5+5+4;

    return max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
static Logical send_all_trees (gz, lcodes, dcodes, blcodes)
    GZp gz;
    int lcodes, dcodes, blcodes; /* number of codes for each tree */
{
    int rank;                    /* index in bl_order */
    if (!send_bits(gz,lcodes-257,5)) return FALSE;
				 /* not +255 as stated in appnote.txt */
    if (!send_bits(gz,dcodes-1,5)) return FALSE;
    if (!send_bits(gz,blcodes-4,4)) return FALSE;
				 /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
	if (!send_bits(gz,gz->bl_tree[gz->bl_order[rank]].dl.len,3)) return FALSE;
    }
    if (!send_tree(gz,(ct_data *)gz->dyn_ltree,lcodes-1)) return FALSE;
						 /* send the literal tree */
    if (!send_tree(gz,(ct_data *)gz->dyn_dtree,dcodes-1)) return FALSE;
						 /* send the distance tree*/
    return TRUE;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file.
 */
static Logical flush_block(gz, buf, stored_len, eof)
    GZp gz;
    char *buf;        /* input block, or NULL if too old */
    ulg stored_len;   /* length of input block */
    int eof;          /* true if this is the last block for a file */
{
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

    gz->flag_buf[gz->last_flags] = gz->tree_flags;
		      /*Save the flags for the last 8 items */

    /* Construct the literal and distance trees */
    build_tree_gz(gz, (tree_desc *)(&(gz->l_desc)));

    build_tree_gz(gz, (tree_desc *)(&(gz->d_desc)));
    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations.
     */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send.
     */
    max_blindex = build_bl_tree(gz);

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (gz->opt_len+3+7)>>3;
    static_lenb = (gz->static_len+3+7)>>3;

    if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

    if (stored_len+4 <= opt_lenb && buf != (char*)0) {
	/* 4: two words for the lengths */
	/* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
	 * Otherwise we can't have processed more than WSIZE input bytes since
	 * the last block flush, because compression would have been
	 * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
	 * transform a block into a stored block.
	 */
	if (!send_bits(gz,(STORED_BLOCK<<1)+eof, 3)) return FALSE;
						  /* send block type */
	if (!copy_block(gz,buf,(unsigned)stored_len,1)) return FALSE;
							 /* with header */
    }
    else
      if (static_lenb == opt_lenb) {
	if (!send_bits(gz,(STATIC_TREES<<1)+eof,3)) return FALSE;
	if (!compress_block(gz,(ct_data *)gz->static_ltree,
			    (ct_data *)gz->static_dtree)) return FALSE;
      }
      else {
	if (!send_bits(gz,(DYN_TREES<<1)+eof,3)) return FALSE;
	if (!send_all_trees(gz,gz->l_desc.max_code+1,
			    gz->d_desc.max_code+1,
			    max_blindex+1)) return FALSE;
	if (!compress_block(gz,(ct_data *)gz->dyn_ltree,
			    (ct_data *)gz->dyn_dtree)) return FALSE;
      }

    init_block(gz);

    if (eof) {
	if (!bi_windup(gz)) return FALSE;
    }

    return TRUE;
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
static int ct_tally (gz, dist, lc)
    GZp gz;
    int dist;  /* distance of matched string */
    int lc;    /* match length-MIN_MATCH or unmatched char (if dist==0) */
{
    gz->inbuf[gz->last_lit++] = (uch)lc;
    if (dist == 0) {
	/* lc is the unmatched char */
	gz->dyn_ltree[lc].fc.freq++;
    } else {
	/* Here, lc is the match length - MIN_MATCH */
	dist--;             /* dist = match distance - 1 */

	gz->dyn_ltree[gz->length_code[lc]+LITERALS+1].fc.freq++;
	gz->dyn_dtree[d_code(dist,gz->dist_code)].fc.freq++;

	gz->d_buf[gz->last_dist++] = (ush)dist;
	gz->tree_flags |= gz->flag_bit;
    }
    gz->flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((gz->last_lit & 7) == 0) {
	gz->flag_buf[gz->last_flags++] = gz->tree_flags;
	gz->tree_flags = 0, gz->flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if (gz->level > 2 && (gz->last_lit & 0xfff) == 0) {
	/* Compute an upper bound for the compressed length */
	ulg out_length = (ulg)gz->last_lit*8L;
	ulg in_length = (ulg)gz->strstart - gz->block_start;
	int dcode;
	for (dcode = 0; dcode < D_CODES; dcode++) {
	    out_length += (ulg)gz->dyn_dtree[dcode].fc.freq *
			  (5L+gz->extra_dbits[dcode]);
	}
	out_length >>= 3;
	if (gz->last_dist < gz->last_lit/2 && out_length < in_length/2) return 1;
    }
    return (gz->last_lit == LIT_BUFSIZE-1 || gz->last_dist == DIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
static Logical compress_block (gz, ltree, dtree)
    GZp gz;
    ct_data *ltree; /* literal tree */
    ct_data *dtree; /* distance tree */
{
    unsigned dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    unsigned lx = 0;    /* running index in l_buf (inbuf) */
    unsigned dx = 0;    /* running index in d_buf */
    unsigned fx = 0;    /* running index in flag_buf */
    uch flag = 0;       /* current flags */
    unsigned code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (gz->last_lit != 0) do {
	if ((lx & 7) == 0) flag = gz->flag_buf[fx++];
	lc = gz->inbuf[lx++];
	if ((flag & 1) == 0) {
	    if (!send_code(gz,lc,ltree)) return FALSE;
					 /* send a literal byte */
	} else {
	    /* Here, lc is the match length - MIN_MATCH */
	    code = gz->length_code[lc];
	    if (!send_code(gz,code+LITERALS+1,ltree)) return FALSE;
						 /* send the length code */
	    extra = gz->extra_lbits[code];
	    if (extra != 0) {
		lc -= gz->base_length[code];
		if (!send_bits(gz,lc,extra)) return FALSE;
					       /* send the extra length bits */
	    }
	    dist = gz->d_buf[dx++];
	    /* Here, dist is the match distance - 1 */
	    code = d_code(dist,gz->dist_code);

	    if (!send_code(gz,code,dtree)) return FALSE;
					       /* send the distance code */
	    extra = gz->extra_dbits[code];
	    if (extra != 0) {
		dist -= gz->base_dist[code];
		if (!send_bits(gz,dist,extra)) return FALSE;
					   /* send the extra distance bits */
	    }
	} /* literal or match pair ? */
	flag >>= 1;
    } while (lx < gz->last_lit);
    if (!send_code(gz,END_BLOCK,ltree)) return FALSE;
    return TRUE;
}



/* util.c -- utility functions for gzip support
 * Copyright (C) 1992-1993 Jean-loup Gailly
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */

/* ===========================================================================
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 */
static ulg updcrc (s, n, crcReg, crc_32_tab)
    uch *s;                 /* pointer to bytes to pump through */
    unsigned n;             /* number of bytes in s[] */
    ulg *crcReg;            /* shift register contents */
    ulg crc_32_tab[N_CRC_32_TAB];
			    /* Table of CRC-32's of all single-byte values. */
{
    register ulg c;         /* temporary variable */

    if (s == NULL) {
	c = 0xffffffffL;
    } else {
	c = *crcReg;
	if (n) do {
	    c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
	} while (--n);
    }
    *crcReg = c;
    return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty.
 */
static Logical fill_inbuf (gu, byte)   /* JTL */
GUp gu;
uch *byte;
{
    int len;
    /* Read as much as possible */
    gu->insize = 0;
    do {
	len = (int) V_read ((char *) gu->inbuf + gu->insize, (size_t) 1,
			    (size_t) (INBUFSIZ - gu->insize), gu->iFp);
	if (len == 0) break;    /* V_read returns 0 if an error. */
	gu->insize += len;
    } while (gu->insize < INBUFSIZ);
    if (gu->insize == 0) return FALSE;
    gu->bytes_in += (ulg)gu->insize;
    gu->inptr = 1;
    *byte = gu->inbuf[0];
    return TRUE;
}

/* ===========================================================================
 * Write the output buffer outbuf[0..outcnt-1] and update bytes_out.
 * (used for the compressed data only)
 */
static Logical flush_outbuf(gz)
GZp gz;
{
    if (gz->outcnt == 0) return TRUE;
    if (!write_buf(gz->oFp,(char *)gz->outbuf,gz->outcnt)) return FALSE;
    gz->bytes_out += (ulg)gz->outcnt;
    gz->outcnt = 0;
    return TRUE;
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static Logical flush_window(gu)
GUp gu;
{
    if (gu->outcnt == 0) return TRUE;
    updcrc (gu->window, gu->outcnt, &(gu->crcReg), gu->crc_32_tab);
    if (!write_buf(gu->oFp,(char *)gu->window,gu->outcnt)) return FALSE;
    gu->bytes_out += (ulg)gu->outcnt;
    gu->outcnt = 0;
    return TRUE;
}

/* ===========================================================================
 * Does the same as write(), but also handles partial pipe writes and checks
 * for error return.
 */
static Logical write_buf(vFp, buf, cnt)
    vFILE     *vFp;
    void      *buf;
    unsigned  cnt;
{
    unsigned  n;
    while (cnt > 0) {
	n = (unsigned) V_write (buf, (size_t) 1, (size_t) cnt, vFp);
	if (n == 0) return FALSE;       /* V_write returns 0 if error. */
	cnt -= n;
	buf = (void *)((char*)buf+n);
    }
    return TRUE;
}

/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
   [The history has been moved to the file ChangeLog.]
 */

/*
   Inflate deflated (PKZIP's method 8 compressed) data.  The compression
   method searches for as much of the current string of bytes (up to a
   length of 258) in the previous 32K bytes.  If it doesn't find any
   matches (of at least length 3), it codes the next byte.  Otherwise, it
   codes the length of the matched string and its distance backwards from
   the current position.  There is a single Huffman code that codes both
   single bytes (called "literals") and match lengths.  A second Huffman
   code codes the distance information, which follows a length code.  Each
   length or distance code actually represents a base value and a number
   of "extra" (sometimes zero) bits to get to add to the base value.  At
   the end of each deflated block is a special end-of-block (EOB) literal/
   length code.  The decoding process is basically: get a literal/length
   code; if EOB then done; if a literal, emit the decoded byte; if a
   length then get the distance and emit the referred-to bytes from the
   sliding window of previously emitted data.

   There are (currently) three kinds of inflate blocks: stored, fixed, and
   dynamic.  The compressor deals with some chunk of data at a time, and
   decides which method to use on a chunk-by-chunk basis.  A chunk might
   typically be 32K or 64K.  If the chunk is uncompressible, then the
   "stored" method is used.  In this case, the bytes are simply stored as
   is, eight bits per byte, with none of the above coding.  The bytes are
   preceded by a count, since there is no longer an EOB code.

   If the data is compressible, then either the fixed or dynamic methods
   are used.  In the dynamic method, the compressed data is preceded by
   an encoding of the literal/length and distance Huffman codes that are
   to be used to decode this block.  The representation is itself Huffman
   coded, and so is preceded by a description of that code.  These code
   descriptions take up a little space, and so for small blocks, there is
   a predefined set of codes, called the fixed codes.  The fixed method is
   used if the block codes up smaller that way (usually for quite small
   chunks), otherwise the dynamic method is used.  In the latter case, the
   codes are customized to the probabilities in the current block, and so
   can code it much better than the pre-determined fixed codes.
 
   The Huffman codes themselves are decoded using a mutli-level table
   lookup, in order to maximize the speed of decoding plus the speed of
   building the decoding tables.  See the comments below that precede the
   LBITS and DBITS tuning parameters.
 */


/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarly, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: #of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */


static int huft_build(gu, b, n, s, d, e, t, m)
GUp gu;
unsigned *b;            /* code lengths in bits (all assumed <= BMAX) */
unsigned n;             /* number of codes (assumed <= N_MAX) */
unsigned s;             /* number of simple-valued codes (0..s-1) */
ush *d;                 /* list of base values for non-simple codes */
ush *e;                 /* list of extra bits for non-simple codes */
struct huft **t;        /* result: starting table */
int *m;                 /* maximum lookup bits, returns actual */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
  unsigned a;                   /* counter for codes of length k */
  unsigned c[BMAX+1];           /* bit length count table */
  unsigned f;                   /* i repeats in table every f entries */
  int g;                        /* maximum code length */
  int h;                        /* table level */
  register unsigned i;          /* counter, current code */
  register unsigned j;          /* counter */
  register int k;               /* number of bits in current code */
  int l;                        /* bits per table (returned in m) */
  register unsigned *p;         /* pointer into c[], b[], or v[] */
  register struct huft *q;      /* points to current table */
  struct huft r;                /* table entry for structure assignment */
  struct huft *u[BMAX];         /* table stack */
  unsigned v[N_MAX];            /* values in order of bit length */
  register int w;               /* bits before this table == (l * h) */
  unsigned x[BMAX+1];           /* bit offsets, then code stack */
  unsigned *xp;                 /* pointer into x */
  int y;                        /* number of dummy codes added */
  unsigned z;                   /* number of entries in current table */

  /* Generate counts for each bit length */
  memzero(c, sizeof(c));
  p = b;  i = n;
  do {
    c[*p]++;                    /* assume all entries <= BMAX */
    p++;                      /* Can't combine with above line (Solaris bug) */
  } while (--i);
  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (struct huft *)NULL;
    *m = 0;
    return 0;
  }


  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((unsigned)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((unsigned)l > i)
    l = i;
  *m = l;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;                 /* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }


  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);


  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = -l;                       /* bits decoded == (l * h) */
  u[0] = (struct huft *)NULL;   /* just to keep compilers happy */
  q = (struct huft *)NULL;      /* ditto */
  z = 0;                        /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l)
      {
	h++;
	w += l;                 /* previous table always l bits */

	/* compute minimum size table less than or equal to l bits */
	z = (z = g - w) > (unsigned)l ? l : z;  /* upper limit on table size */
	if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
	{                       /* too few codes for k-w bit table */
	  f -= a + 1;           /* deduct codes from patterns left */
	  xp = c + k;
	  while (++j < z)       /* try smaller tables up to z bits */
	  {
	    if ((f <<= 1) <= *++xp)
	      break;            /* enough codes to use up j bits */
	    f -= *xp;           /* else deduct codes from patterns */
	  }
	}
	z = 1 << j;             /* table entries for j-bit table */

	/* allocate and link in new table */
	if ((q = (struct huft *)
	    CallocateMemory((size_t) 1,
			    (z + 1) * sizeof(struct huft),NULL)) == NULL) {
	  if (h) huft_free(u[0]);
	  return 3;             /* not enough memory */
	}
	gu->hufts += z + 1;         /* track memory usage */
	*t = q + 1;             /* link to list for huft_free() */
	*(t = &(q->v.t)) = (struct huft *)NULL;
	u[h] = ++q;             /* table starts after link */

	/* connect to last table, if there is one */
	if (h)
	{
	  x[h] = i;             /* save pattern for backing up */
	  r.b = (uch)l;         /* bits to dump before this table */
	  r.e = (uch)(16 + j);  /* bits in this table */
	  r.v.t = q;            /* pointer to this table */
	  j = i >> (w - l);     /* (get around Turbo C bug) */
	  u[h-1][j] = r;        /* connect to last table */
	}
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);
      if (p >= v + n)
	r.e = 99;               /* out of values--invalid code */
      else if (*p < s)
      {
	r.e = (uch)(*p < 256 ? 16 : 15);    /* 256 is end-of-block code */
	r.v.n = (ush)(*p);             /* simple code is just the value */
	p++;                           /* one compiler does not like *p++ */
      }
      else
      {
	r.e = (uch)e[*p - s];   /* non-simple--look up in lists */
	r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
	q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
	i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
      {
	h--;                    /* don't need to update q */
	w -= l;
      }
    }
  }

  /* Return 1 if we were given an incomplete table */
  return (y != 0 && g != 1 ? 1 : 0);
}



static int huft_free(t)
struct huft *t;         /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *)NULL)
  {
    q = (--p)->v.t;
    cdf_FreeMemory ((char *) p, NULL);
    p = q;
  } 
  return 0;
}


static int inflate_codes(gu, tl, td, bl, bd)
GUp gu;
struct huft *tl, *td;   /* literal/length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */
{
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */

  /* make local copies of globals */
  b = gu->bb;                       /* initialize bit buffer */
  k = gu->bk;
  w = gu->outcnt;                       /* initialize window position */

  /* inflate the coded data */
  ml = gu->mask_bits[bl];           /* precompute masks for speed */
  md = gu->mask_bits[bd];
  for (;;)                      /* do until end of block */
  {
    NEEDBITS(gu,b,k,(unsigned)bl)
    if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
      do {
	if (e == 99) return 1;
	DUMPBITS(b,k,t->b)
	e -= 16;
	NEEDBITS(gu,b,k,e)
      } while ((e = (t = t->v.t + ((unsigned)b & gu->mask_bits[e]))->e) > 16);
    DUMPBITS(b,k,t->b)
    if (e == 16)                /* then it's a literal */
    {
      gu->window[w++] = (uch)t->v.n;
      if (w == WSIZE) {
	gu->outcnt = w;
	if (!flush_window(gu)) return 5;        /* Write error - JTL */
	w = 0;
      }
    }
    else                        /* it's an EOB or a length */
    {
      /* exit if end of block */
      if (e == 15)
	break;

      /* get length of block to copy */
      NEEDBITS(gu,b,k,e)
      n = t->v.n + ((unsigned)b & gu->mask_bits[e]);
      DUMPBITS(b,k,e);

      /* decode distance of block to copy */
      NEEDBITS(gu,b,k,(unsigned)bd)
      if ((e = (t = td + ((unsigned)b & md))->e) > 16)
	do {
	  if (e == 99) return 1;
	  DUMPBITS(b,k,t->b)
	  e -= 16;
	  NEEDBITS(gu,b,k,e)
	} while ((e = (t = t->v.t + ((unsigned)b & gu->mask_bits[e]))->e) > 16);
      DUMPBITS(b,k,t->b)
      NEEDBITS(gu,b,k,e)
      d = w - t->v.n - ((unsigned)b & gu->mask_bits[e]);
      DUMPBITS(b,k,e)

      /* do the copy */
      do {
	n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
	if (w - d >= e)         /* (this test assumes unsigned comparison) */
	{
	  memcpy(gu->window + w, gu->window + d, e);
	  w += e;
	  d += e;
	}
	else                      /* do it slow to avoid memcpy() overlap */
	  do {
	    gu->window[w++] = gu->window[d++];
	  } while (--e);
	if (w == WSIZE)
	{
	  gu->outcnt = w;
	  if (!flush_window(gu)) return 5;      /* Write error. */
	  w = 0;
	}
      } while (n);
    }
  }

  /* restore the globals from the locals */
  gu->outcnt = w;                       /* restore global window pointer */
  gu->bb = b;                       /* restore global bit buffer */
  gu->bk = k;

  /* done */
  return 0;
}

static int inflate_stored(gu)
GUp gu;
/* "decompress" an inflated type 0 (stored) block. */
{
  unsigned n;           /* number of bytes in block */
  unsigned w;           /* current window position */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */

  /* make local copies of globals */
  b = gu->bb;                       /* initialize bit buffer */
  k = gu->bk;
  w = gu->outcnt;                       /* initialize window position */

  /* go to byte boundary */
  n = k & 7;
  DUMPBITS(b,k,n);

  /* get the length and its complement */
  NEEDBITS(gu,b,k,16)
  n = ((unsigned)b & 0xffff);
  DUMPBITS(b,k,16)
  NEEDBITS(gu,b,k,16)
  if (n != (unsigned)((~b) & 0xffff))
    return 1;                   /* error in compressed data */
  DUMPBITS(b,k,16)

  /* read and output the compressed data */
  while (n--)
  {
    NEEDBITS(gu,b,k,8)
    gu->window[w++] = (uch)b;
    if (w == WSIZE)
    {
      gu->outcnt = w;
      if (!flush_window(gu)) return 5;  /* Write error. */
      w = 0;
    }
    DUMPBITS(b,k,8)
  }

  /* restore the globals from the locals */
  gu->outcnt = w;                       /* restore global window pointer */
  gu->bb = b;                       /* restore global bit buffer */
  gu->bk = k;
  return 0;
}

static int inflate_fixed(gu)
GUp gu;
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
  int i;                /* temporary variable */
  struct huft *tl;      /* literal/length code table */
  struct huft *td;      /* distance code table */
  int bl;               /* lookup bits for tl */
  int bd;               /* lookup bits for td */
  unsigned l[288];      /* length list for huft_build */

  /* set up literal table */
  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)          /* make a complete, but wrong code set */
    l[i] = 8;
  bl = 7;
  if ((i = huft_build(gu, l, 288, 257, gu->cplens, gu->cplext, &tl, &bl)) != 0)
    return i;

  /* set up distance table */
  for (i = 0; i < 30; i++)      /* make an incomplete code set */
    l[i] = 5;
  bd = 5;
  if ((i = huft_build(gu, l, 30, 0, gu->cpdist, gu->cpdext, &td, &bd)) > 1)
  {
    huft_free(tl);
    return i;
  }

  /* decompress until an end-of-block code */
  if ((i = inflate_codes(gu,tl,td,bl,bd)) != 0) return i;       /* JTL */

  /* free the decoding tables, return */
  huft_free (tl);
  huft_free (td);
  return 0;
}

static int inflate_dynamic(gu)
GUp gu;
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
  int i;                /* temporary variables */
  unsigned j;
  unsigned l;           /* last length */
  unsigned m;           /* mask for bit lengths table */
  unsigned n;           /* number of lengths to get */
  struct huft *tl;      /* literal/length code table */
  struct huft *td;      /* distance code table */
  int bl;               /* lookup bits for tl */
  int bd;               /* lookup bits for td */
  unsigned nb;          /* number of bit length codes */
  unsigned nl;          /* number of literal/length codes */
  unsigned nd;          /* number of distance codes */
  unsigned ll[286+30];  /* literal/length and distance code lengths */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */

  /* make local bit buffer */
  b = gu->bb;
  k = gu->bk;

  /* read in table lengths */
  NEEDBITS(gu,b,k,5)
  nl = 257 + ((unsigned)b & 0x1f);      /* number of literal/length codes */
  DUMPBITS(b,k,5)
  NEEDBITS(gu,b,k,5)
  nd = 1 + ((unsigned)b & 0x1f);        /* number of distance codes */
  DUMPBITS(b,k,5)
  NEEDBITS(gu,b,k,4)
  nb = 4 + ((unsigned)b & 0xf);         /* number of bit length codes */
  DUMPBITS(b,k,4)
  if (nl > 286 || nd > 30) return 1;                   /* bad lengths */

  /* read in bit-length-code lengths */
  for (j = 0; j < nb; j++)
  {
    NEEDBITS(gu,b,k,3)
    ll[gu->border[j]] = (unsigned)b & 7;
    DUMPBITS(b,k,3)
  }
  for (; j < 19; j++) ll[gu->border[j]] = 0;

  /* build decoding table for trees--single level, 7 bit lookup */
  bl = 7;
  if ((i = huft_build(gu, ll, 19, 19, NULL, NULL, &tl, &bl)) != 0)
  {
    if (i == 1) huft_free (tl);
    return i;                   /* incomplete code set */
  }

  /* read in literal and distance code lengths */
  n = nl + nd;
  m = gu->mask_bits[bl];
  i = l = 0;
  while ((unsigned)i < n)
  {
    NEEDBITS(gu,b,k,(unsigned)bl)
    j = (td = tl + ((unsigned)b & m))->b;
    DUMPBITS(b,k,j)
    j = td->v.n;
    if (j < 16)                 /* length of code in bits (0..15) */
      ll[i++] = l = j;          /* save last length in l */
    else if (j == 16)           /* repeat last length 3 to 6 times */
    {
      NEEDBITS(gu,b,k,2)
      j = 3 + ((unsigned)b & 3);
      DUMPBITS(b,k,2)
      if ((unsigned)i + j > n)
	return 1;
      while (j--)
	ll[i++] = l;
    }
    else if (j == 17)           /* 3 to 10 zero length codes */
    {
      NEEDBITS(gu,b,k,3)
      j = 3 + ((unsigned)b & 7);
      DUMPBITS(b,k,3)
      if ((unsigned)i + j > n)
	return 1;
      while (j--)
	ll[i++] = 0;
      l = 0;
    }
    else                        /* j == 18: 11 to 138 zero length codes */
    {
      NEEDBITS(gu,b,k,7)
      j = 11 + ((unsigned)b & 0x7f);
      DUMPBITS(b,k,7)
      if ((unsigned)i + j > n)
	return 1;
      while (j--)
	ll[i++] = 0;
      l = 0;
    }
  }

  /* free decoding table for trees */
  huft_free (tl);

  /* restore the global bit buffer */
  gu->bb = b;
  gu->bk = k;

  /* build the decoding tables for literal/length and distance codes */
  bl = LBITS;
  if ((i = huft_build(gu, ll, nl, 257, gu->cplens, gu->cplext, &tl, &bl)) != 0)
  {
    if (i == 1) {
      /* fprintf(stderr, " incomplete literal tree\n"); */
      huft_free (tl);
    }
    return i;                   /* incomplete code set */
  }
  bd = DBITS;
  if ((i = huft_build(gu, ll + nl, nd, 0, gu->cpdist, gu->cpdext, &td, &bd)) != 0) {
    if (i == 1) {
      /* fprintf(stderr, " incomplete distance tree\n"); */
      huft_free (td);
    }
    huft_free (tl);
    return i;                   /* incomplete code set */
  }


  /* decompress until an end-of-block code */
  if ((i = inflate_codes(gu,tl,td,bl,bd)) != 0) return i;       /* JTL */

  /* free the decoding tables, return */
  huft_free (tl);
  huft_free (td);
  return 0;
}

static int inflate_block(gu, e)
GUp gu;
int *e;                 /* last block flag */
/* decompress an inflated block */
{
  unsigned t;           /* block type */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */

  /* make local bit buffer */
  b = gu->bb;
  k = gu->bk;

  /* read in last block bit */
  NEEDBITS(gu,b,k,1)
  *e = (int)b & 1;
  DUMPBITS(b,k,1)

  /* read in block type */
  NEEDBITS(gu,b,k,2)
  t = (unsigned)b & 3;
  DUMPBITS(b,k,2)

  /* restore the global bit buffer */
  gu->bb = b;
  gu->bk = k;

  /* inflate that block type */
  if (t == 2) return inflate_dynamic(gu);
  if (t == 0) return inflate_stored(gu);
  if (t == 1) return inflate_fixed(gu);

  /* bad block type */
  return 2;
}



static int inflate(gu)
GUp gu;
/* decompress an inflated entry */
{
  int e;                /* last block flag */
  int r;                /* result code */
  unsigned h;           /* maximum struct huft's malloc'ed */

  /* initialize window, bit buffer */
  gu->outcnt = 0;
  gu->bk = 0;
  gu->bb = 0;

  /* decompress until the last block */
  h = 0;
  do {
    gu->hufts = 0;
    if ((r = inflate_block(gu,&e)) != 0) return r;
    if (gu->hufts > h) h = gu->hufts;
  } while (!e);

  /* Undo too much lookahead. The next read will be byte aligned so we
   * can discard unused bits in the last meaningful byte.
   */
  while (gu->bk >= 8) {
    gu->bk -= 8;
    gu->inptr--;
  }

  /* flush out slide */
  if (!flush_window(gu)) return 5;      /* Write error. */

  /* return success */
  return 0;
}

/******************************************************************************
* PutByte.
******************************************************************************/

static Logical PutByte (gz, c)
GZp gz;
unsigned c;
{
  gz->outbuf[gz->outcnt++] = (uch) c;
  if (gz->outcnt == OUTBUFSIZ) {
    if (!flush_outbuf(gz)) return FALSE;
  }
  return TRUE;
}

/******************************************************************************
* PutShort.
******************************************************************************/

static Logical PutShort (gz, w)
GZp gz;
unsigned w;
{
  if (gz->outcnt < OUTBUFSIZ-2) {
    gz->outbuf[gz->outcnt++] = (uch) ((ush)(w) & 0xff);
    gz->outbuf[gz->outcnt++] = (uch) ((ush)(w) >> 8);
  }
  else {
    if (!PutByte(gz,(unsigned)((ush)(w) & 0xff))) return FALSE;
    if (!PutByte(gz,(unsigned)((ush)(w) >> 8))) return FALSE;
  }
  return TRUE;
}

/******************************************************************************
* PutLong.
******************************************************************************/

static Logical PutLong (gz, n)
GZp gz;
ulg n;
{
  if (!PutShort(gz,(unsigned)((n) & 0xffff))) return FALSE;
  if (!PutShort(gz,(unsigned)((n) >> 16))) return FALSE;
  return TRUE;
}

/******************************************************************************
* GetByte.
******************************************************************************/

static Logical GetByte (gu, byte)
GUp gu;
uch *byte;
{
  if (gu->inptr < gu->insize)
    *byte = gu->inbuf[gu->inptr++];
  else
    if (!fill_inbuf(gu,byte)) return FALSE;
  return TRUE;
}

/******************************************************************************
* initCRC.
******************************************************************************/

static void initCRC (crc_32_tab)
ulg crc_32_tab[N_CRC_32_TAB];
{
  crc_32_tab[0]= 0x00000000L;
  crc_32_tab[1]= 0x77073096L;
  crc_32_tab[2]= 0xee0e612cL;
  crc_32_tab[3]= 0x990951baL;
  crc_32_tab[4]= 0x076dc419L;
  crc_32_tab[5]= 0x706af48fL;
  crc_32_tab[6]= 0xe963a535L;
  crc_32_tab[7]= 0x9e6495a3L;
  crc_32_tab[8]= 0x0edb8832L;
  crc_32_tab[9]= 0x79dcb8a4L;
  crc_32_tab[10]= 0xe0d5e91eL;
  crc_32_tab[11]= 0x97d2d988L;
  crc_32_tab[12]= 0x09b64c2bL;
  crc_32_tab[13]= 0x7eb17cbdL;
  crc_32_tab[14]= 0xe7b82d07L;
  crc_32_tab[15]= 0x90bf1d91L;
  crc_32_tab[16]= 0x1db71064L;
  crc_32_tab[17]= 0x6ab020f2L;
  crc_32_tab[18]= 0xf3b97148L;
  crc_32_tab[19]= 0x84be41deL;
  crc_32_tab[20]= 0x1adad47dL;
  crc_32_tab[21]= 0x6ddde4ebL;
  crc_32_tab[22]= 0xf4d4b551L;
  crc_32_tab[23]= 0x83d385c7L;
  crc_32_tab[24]= 0x136c9856L;
  crc_32_tab[25]= 0x646ba8c0L;
  crc_32_tab[26]= 0xfd62f97aL;
  crc_32_tab[27]= 0x8a65c9ecL;
  crc_32_tab[28]= 0x14015c4fL;
  crc_32_tab[29]= 0x63066cd9L;
  crc_32_tab[30]= 0xfa0f3d63L;
  crc_32_tab[31]= 0x8d080df5L;
  crc_32_tab[32]= 0x3b6e20c8L;
  crc_32_tab[33]= 0x4c69105eL;
  crc_32_tab[34]= 0xd56041e4L;
  crc_32_tab[35]= 0xa2677172L;
  crc_32_tab[36]= 0x3c03e4d1L;
  crc_32_tab[37]= 0x4b04d447L;
  crc_32_tab[38]= 0xd20d85fdL;
  crc_32_tab[39]= 0xa50ab56bL;
  crc_32_tab[40]= 0x35b5a8faL;
  crc_32_tab[41]= 0x42b2986cL;
  crc_32_tab[42]= 0xdbbbc9d6L;
  crc_32_tab[43]= 0xacbcf940L;
  crc_32_tab[44]= 0x32d86ce3L;
  crc_32_tab[45]= 0x45df5c75L;
  crc_32_tab[46]= 0xdcd60dcfL;
  crc_32_tab[47]= 0xabd13d59L;
  crc_32_tab[48]= 0x26d930acL;
  crc_32_tab[49]= 0x51de003aL;
  crc_32_tab[50]= 0xc8d75180L;
  crc_32_tab[51]= 0xbfd06116L;
  crc_32_tab[52]= 0x21b4f4b5L;
  crc_32_tab[53]= 0x56b3c423L;
  crc_32_tab[54]= 0xcfba9599L;
  crc_32_tab[55]= 0xb8bda50fL;
  crc_32_tab[56]= 0x2802b89eL;
  crc_32_tab[57]= 0x5f058808L;
  crc_32_tab[58]= 0xc60cd9b2L;
  crc_32_tab[59]= 0xb10be924L;
  crc_32_tab[60]= 0x2f6f7c87L;
  crc_32_tab[61]= 0x58684c11L;
  crc_32_tab[62]= 0xc1611dabL;
  crc_32_tab[63]= 0xb6662d3dL;
  crc_32_tab[64]= 0x76dc4190L;
  crc_32_tab[65]= 0x01db7106L;
  crc_32_tab[66]= 0x98d220bcL;
  crc_32_tab[67]= 0xefd5102aL;
  crc_32_tab[68]= 0x71b18589L;
  crc_32_tab[69]= 0x06b6b51fL;
  crc_32_tab[70]= 0x9fbfe4a5L;
  crc_32_tab[71]= 0xe8b8d433L;
  crc_32_tab[72]= 0x7807c9a2L;
  crc_32_tab[73]= 0x0f00f934L;
  crc_32_tab[74]= 0x9609a88eL;
  crc_32_tab[75]= 0xe10e9818L;
  crc_32_tab[76]= 0x7f6a0dbbL;
  crc_32_tab[77]= 0x086d3d2dL;
  crc_32_tab[78]= 0x91646c97L;
  crc_32_tab[79]= 0xe6635c01L;
  crc_32_tab[80]= 0x6b6b51f4L;
  crc_32_tab[81]= 0x1c6c6162L;
  crc_32_tab[82]= 0x856530d8L;
  crc_32_tab[83]= 0xf262004eL;
  crc_32_tab[84]= 0x6c0695edL;
  crc_32_tab[85]= 0x1b01a57bL;
  crc_32_tab[86]= 0x8208f4c1L;
  crc_32_tab[87]= 0xf50fc457L;
  crc_32_tab[88]= 0x65b0d9c6L;
  crc_32_tab[89]= 0x12b7e950L;
  crc_32_tab[90]= 0x8bbeb8eaL;
  crc_32_tab[91]= 0xfcb9887cL;
  crc_32_tab[92]= 0x62dd1ddfL;
  crc_32_tab[93]= 0x15da2d49L;
  crc_32_tab[94]= 0x8cd37cf3L;
  crc_32_tab[95]= 0xfbd44c65L;
  crc_32_tab[96]= 0x4db26158L;
  crc_32_tab[97]= 0x3ab551ceL;
  crc_32_tab[98]= 0xa3bc0074L;
  crc_32_tab[99]= 0xd4bb30e2L;
  crc_32_tab[100]= 0x4adfa541L;
  crc_32_tab[101]= 0x3dd895d7L;
  crc_32_tab[102]= 0xa4d1c46dL;
  crc_32_tab[103]= 0xd3d6f4fbL;
  crc_32_tab[104]= 0x4369e96aL;
  crc_32_tab[105]= 0x346ed9fcL;
  crc_32_tab[106]= 0xad678846L;
  crc_32_tab[107]= 0xda60b8d0L;
  crc_32_tab[108]= 0x44042d73L;
  crc_32_tab[109]= 0x33031de5L;
  crc_32_tab[110]= 0xaa0a4c5fL;
  crc_32_tab[111]= 0xdd0d7cc9L;
  crc_32_tab[112]= 0x5005713cL;
  crc_32_tab[113]= 0x270241aaL;
  crc_32_tab[114]= 0xbe0b1010L;
  crc_32_tab[115]= 0xc90c2086L;
  crc_32_tab[116]= 0x5768b525L;
  crc_32_tab[117]= 0x206f85b3L;
  crc_32_tab[118]= 0xb966d409L;
  crc_32_tab[119]= 0xce61e49fL;
  crc_32_tab[120]= 0x5edef90eL;
  crc_32_tab[121]= 0x29d9c998L;
  crc_32_tab[122]= 0xb0d09822L;
  crc_32_tab[123]= 0xc7d7a8b4L;
  crc_32_tab[124]= 0x59b33d17L;
  crc_32_tab[125]= 0x2eb40d81L;
  crc_32_tab[126]= 0xb7bd5c3bL;
  crc_32_tab[127]= 0xc0ba6cadL;
  crc_32_tab[128]= 0xedb88320L;
  crc_32_tab[129]= 0x9abfb3b6L;
  crc_32_tab[130]= 0x03b6e20cL;
  crc_32_tab[131]= 0x74b1d29aL;
  crc_32_tab[132]= 0xead54739L;
  crc_32_tab[133]= 0x9dd277afL;
  crc_32_tab[134]= 0x04db2615L;
  crc_32_tab[135]= 0x73dc1683L;
  crc_32_tab[136]= 0xe3630b12L;
  crc_32_tab[137]= 0x94643b84L;
  crc_32_tab[138]= 0x0d6d6a3eL;
  crc_32_tab[139]= 0x7a6a5aa8L;
  crc_32_tab[140]= 0xe40ecf0bL;
  crc_32_tab[141]= 0x9309ff9dL;
  crc_32_tab[142]= 0x0a00ae27L;
  crc_32_tab[143]= 0x7d079eb1L;
  crc_32_tab[144]= 0xf00f9344L;
  crc_32_tab[145]= 0x8708a3d2L;
  crc_32_tab[146]= 0x1e01f268L;
  crc_32_tab[147]= 0x6906c2feL;
  crc_32_tab[148]= 0xf762575dL;
  crc_32_tab[149]= 0x806567cbL;
  crc_32_tab[150]= 0x196c3671L;
  crc_32_tab[151]= 0x6e6b06e7L;
  crc_32_tab[152]= 0xfed41b76L;
  crc_32_tab[153]= 0x89d32be0L;
  crc_32_tab[154]= 0x10da7a5aL;
  crc_32_tab[155]= 0x67dd4accL;
  crc_32_tab[156]= 0xf9b9df6fL;
  crc_32_tab[157]= 0x8ebeeff9L;
  crc_32_tab[158]= 0x17b7be43L;
  crc_32_tab[159]= 0x60b08ed5L;
  crc_32_tab[160]= 0xd6d6a3e8L;
  crc_32_tab[161]= 0xa1d1937eL;
  crc_32_tab[162]= 0x38d8c2c4L;
  crc_32_tab[163]= 0x4fdff252L;
  crc_32_tab[164]= 0xd1bb67f1L;
  crc_32_tab[165]= 0xa6bc5767L;
  crc_32_tab[166]= 0x3fb506ddL;
  crc_32_tab[167]= 0x48b2364bL;
  crc_32_tab[168]= 0xd80d2bdaL;
  crc_32_tab[169]= 0xaf0a1b4cL;
  crc_32_tab[170]= 0x36034af6L;
  crc_32_tab[171]= 0x41047a60L;
  crc_32_tab[172]= 0xdf60efc3L;
  crc_32_tab[173]= 0xa867df55L;
  crc_32_tab[174]= 0x316e8eefL;
  crc_32_tab[175]= 0x4669be79L;
  crc_32_tab[176]= 0xcb61b38cL;
  crc_32_tab[177]= 0xbc66831aL;
  crc_32_tab[178]= 0x256fd2a0L;
  crc_32_tab[179]= 0x5268e236L;
  crc_32_tab[180]= 0xcc0c7795L;
  crc_32_tab[181]= 0xbb0b4703L;
  crc_32_tab[182]= 0x220216b9L;
  crc_32_tab[183]= 0x5505262fL;
  crc_32_tab[184]= 0xc5ba3bbeL;
  crc_32_tab[185]= 0xb2bd0b28L;
  crc_32_tab[186]= 0x2bb45a92L;
  crc_32_tab[187]= 0x5cb36a04L;
  crc_32_tab[188]= 0xc2d7ffa7L;
  crc_32_tab[189]= 0xb5d0cf31L;
  crc_32_tab[190]= 0x2cd99e8bL;
  crc_32_tab[191]= 0x5bdeae1dL;
  crc_32_tab[192]= 0x9b64c2b0L;
  crc_32_tab[193]= 0xec63f226L;
  crc_32_tab[194]= 0x756aa39cL;
  crc_32_tab[195]= 0x026d930aL;
  crc_32_tab[196]= 0x9c0906a9L;
  crc_32_tab[197]= 0xeb0e363fL;
  crc_32_tab[198]= 0x72076785L;
  crc_32_tab[199]= 0x05005713L;
  crc_32_tab[200]= 0x95bf4a82L;
  crc_32_tab[201]= 0xe2b87a14L;
  crc_32_tab[202]= 0x7bb12baeL;
  crc_32_tab[203]= 0x0cb61b38L;
  crc_32_tab[204]= 0x92d28e9bL;
  crc_32_tab[205]= 0xe5d5be0dL;
  crc_32_tab[206]= 0x7cdcefb7L;
  crc_32_tab[207]= 0x0bdbdf21L;
  crc_32_tab[208]= 0x86d3d2d4L;
  crc_32_tab[209]= 0xf1d4e242L;
  crc_32_tab[210]= 0x68ddb3f8L;
  crc_32_tab[211]= 0x1fda836eL;
  crc_32_tab[212]= 0x81be16cdL;
  crc_32_tab[213]= 0xf6b9265bL;
  crc_32_tab[214]= 0x6fb077e1L;
  crc_32_tab[215]= 0x18b74777L;
  crc_32_tab[216]= 0x88085ae6L;
  crc_32_tab[217]= 0xff0f6a70L;
  crc_32_tab[218]= 0x66063bcaL;
  crc_32_tab[219]= 0x11010b5cL;
  crc_32_tab[220]= 0x8f659effL;
  crc_32_tab[221]= 0xf862ae69L;
  crc_32_tab[222]= 0x616bffd3L;
  crc_32_tab[223]= 0x166ccf45L;
  crc_32_tab[224]= 0xa00ae278L;
  crc_32_tab[225]= 0xd70dd2eeL;
  crc_32_tab[226]= 0x4e048354L;
  crc_32_tab[227]= 0x3903b3c2L;
  crc_32_tab[228]= 0xa7672661L;
  crc_32_tab[229]= 0xd06016f7L;
  crc_32_tab[230]= 0x4969474dL;
  crc_32_tab[231]= 0x3e6e77dbL;
  crc_32_tab[232]= 0xaed16a4aL;
  crc_32_tab[233]= 0xd9d65adcL;
  crc_32_tab[234]= 0x40df0b66L;
  crc_32_tab[235]= 0x37d83bf0L;
  crc_32_tab[236]= 0xa9bcae53L;
  crc_32_tab[237]= 0xdebb9ec5L;
  crc_32_tab[238]= 0x47b2cf7fL;
  crc_32_tab[239]= 0x30b5ffe9L;
  crc_32_tab[240]= 0xbdbdf21cL;
  crc_32_tab[241]= 0xcabac28aL;
  crc_32_tab[242]= 0x53b39330L;
  crc_32_tab[243]= 0x24b4a3a6L;
  crc_32_tab[244]= 0xbad03605L;
  crc_32_tab[245]= 0xcdd70693L;
  crc_32_tab[246]= 0x54de5729L;
  crc_32_tab[247]= 0x23d967bfL;
  crc_32_tab[248]= 0xb3667a2eL;
  crc_32_tab[249]= 0xc4614ab8L;
  crc_32_tab[250]= 0x5d681b02L;
  crc_32_tab[251]= 0x2a6f2b94L;
  crc_32_tab[252]= 0xb40bbe37L;
  crc_32_tab[253]= 0xc30c8ea1L;
  crc_32_tab[254]= 0x5a05df1bL;
  crc_32_tab[255]= 0x2d02ef8dL;
  return;
}
#endif  /* SUPPORT_GZIP */

/******************************************************************************
* Miscellaneous...most of this could be reimplemented as global constants if
* that were a "safe" thing to do considering that the routines in this file
* are part of a shareable library.
******************************************************************************/

#if 0
config configuration_table[10] = {
/* good lazy nice chain */
  {0,    0,  0,    0},  /* 0 */   /* store only */
  {4,    4,  8,    4},  /* 1 */   /* maximum speed, no lazy matches */
  {4,    5, 16,    8},  /* 2 */ 
  {4,    6, 32,   32},  /* 3 */ 
  {4,    4, 16,   16},  /* 4 */   /* lazy matches */
  {8,   16, 32,   32},  /* 5 */ 
  {8,   16, 128, 128},  /* 6 */ 
  {8,   32, 128, 256},  /* 7 */ 
  {32, 128, 258, 1024}, /* 8 */ 
  {32, 258, 258, 4096}  /* 9 */   /* maximum compression */
};
uch bl_order[BL_CODES] = {
  16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
};
/* Tables for deflate from PKZIP's appnote.txt. */
unsigned border[] = {    /* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* note: see note #13 above about the 258 in this list. */
ush cplext[] = {         /* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};
ush cpdext[] = {         /* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13};
ush mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
int extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
ct_data dyn_ltree[HEAP_SIZE];
			/* literal and length tree */
ct_data static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see ct_init
 * below).
 */
tree_desc l_desc = {
  dyn_ltree, static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS, 0
};
int extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
ct_data dyn_dtree[2*D_CODES+1];
			/* distance tree */
ct_data static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */
tree_desc d_desc = {
  dyn_dtree, static_dtree, extra_dbits, 0, D_CODES, MAX_BITS, 0
};
int extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};
ct_data bl_tree[2*BL_CODES+1];
/* Huffman tree for the bit lengths */
tree_desc bl_desc = {
  bl_tree, (ct_data *) NULL, extra_blbits, 0, BL_CODES, MAX_BL_BITS, 0
};
/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by makecrc.c)
 */
ulg crc_32_tab[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};
#endif
