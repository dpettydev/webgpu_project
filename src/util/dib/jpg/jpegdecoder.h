//------------------------------------------------------------------------------
// jpegdecoder.h
// Small JPEG Decoder Library v0.93b
// Last updated: Dec. 28, 2001 
// Copyright (C) 1994-2000 Rich Geldreich
// richgel@voicenet.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//------------------------------------------------------------------------------
#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H
//------------------------------------------------------------------------------
#include <setjmp.h>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define JPGD_INBUFSIZE       4096
//------------------------------------------------------------------------------
// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPGD_MAXBLOCKSPERMCU 10
//------------------------------------------------------------------------------
#define JPGD_MAXHUFFTABLES   8
#define JPGD_MAXQUANTTABLES  4
#define JPGD_MAXCOMPONENTS   4
#define JPGD_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPGD_MAXBLOCKSPERROW 6144
//------------------------------------------------------------------------------
// Max. allocated blocks
#define JPGD_MAXBLOCKS    100
//------------------------------------------------------------------------------
#define JPGD_MAX_HEIGHT 8192
#define JPGD_MAX_WIDTH  8192
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPGD_BAD_DHT_COUNTS              -200
#define JPGD_BAD_DHT_INDEX               -201
#define JPGD_BAD_DHT_MARKER              -202
#define JPGD_BAD_DQT_MARKER              -203
#define JPGD_BAD_DQT_TABLE               -204
#define JPGD_BAD_PRECISION               -205
#define JPGD_BAD_HEIGHT                  -206
#define JPGD_BAD_WIDTH                   -207
#define JPGD_TOO_MANY_COMPONENTS         -208
#define JPGD_BAD_SOF_LENGTH              -209
#define JPGD_BAD_VARIABLE_MARKER         -210
#define JPGD_BAD_DRI_LENGTH              -211
#define JPGD_BAD_SOS_LENGTH              -212
#define JPGD_BAD_SOS_COMP_ID             -213
#define JPGD_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPGD_NO_ARITHMITIC_SUPPORT       -215
#define JPGD_UNEXPECTED_MARKER           -216
#define JPGD_NOT_JPEG                    -217
#define JPGD_UNSUPPORTED_MARKER          -218
#define JPGD_BAD_DQT_LENGTH              -219
#define JPGD_TOO_MANY_BLOCKS             -221
#define JPGD_UNDEFINED_QUANT_TABLE       -222
#define JPGD_UNDEFINED_HUFF_TABLE        -223
#define JPGD_NOT_SINGLE_SCAN             -224
#define JPGD_UNSUPPORTED_COLORSPACE      -225
#define JPGD_UNSUPPORTED_SAMP_FACTORS    -226
#define JPGD_DECODE_ERROR                -227
#define JPGD_BAD_RESTART_MARKER          -228
#define JPGD_ASSERTION_ERROR             -229
#define JPGD_BAD_SOS_SPECTRAL            -230
#define JPGD_BAD_SOS_SUCCESSIVE          -231
#define JPGD_STREAM_READ                 -232
#define JPGD_NOTENOUGHMEM                -233
//------------------------------------------------------------------------------
#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1
#define JPGD_YH2V1     2
#define JPGD_YH1V2     3
#define JPGD_YH2V2     4

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

//------------------------------------------------------------------------------
const int JPGD_FAILED = -1;
const int JPGD_DONE = 1;
const int JPGD_OKAY = 0;
//------------------------------------------------------------------------------
typedef enum
{
  M_SOF0  = 0xC0,
  M_SOF1  = 0xC1,
  M_SOF2  = 0xC2,
  M_SOF3  = 0xC3,

  M_SOF5  = 0xC5,
  M_SOF6  = 0xC6,
  M_SOF7  = 0xC7,

  M_JPG   = 0xC8,
  M_SOF9  = 0xC9,
  M_SOF10 = 0xCA,
  M_SOF11 = 0xCB,

  M_SOF13 = 0xCD,
  M_SOF14 = 0xCE,
  M_SOF15 = 0xCF,

  M_DHT   = 0xC4,

  M_DAC   = 0xCC,

  M_RST0  = 0xD0,
  M_RST1  = 0xD1,
  M_RST2  = 0xD2,
  M_RST3  = 0xD3,
  M_RST4  = 0xD4,
  M_RST5  = 0xD5,
  M_RST6  = 0xD6,
  M_RST7  = 0xD7,

  M_SOI   = 0xD8,
  M_EOI   = 0xD9,
  M_SOS   = 0xDA,
  M_DQT   = 0xDB,
  M_DNL   = 0xDC,
  M_DRI   = 0xDD,
  M_DHP   = 0xDE,
  M_EXP   = 0xDF,

  M_APP0  = 0xE0,
  M_APP15 = 0xEF,

  M_JPG0  = 0xF0,
  M_JPG13 = 0xFD,
  M_COM   = 0xFE,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;
//------------------------------------------------------------------------------
#define RST0 0xD0
//------------------------------------------------------------------------------
typedef struct huff_tables_tag
{
  unsigned int  look_up[256];
  unsigned char code_size[256];
  // FIXME: Is 512 tree entries really enough to handle _all_ possible
  // code sets? I think so but not 100% positive.
  unsigned int  tree[512];
} huff_tables_t, *Phuff_tables_t;
//------------------------------------------------------------------------------
typedef struct coeff_buf_tag
{
  unsigned char *Pdata;

  int block_num_x, block_num_y;
  int block_len_x, block_len_y;

  int block_size;

} coeff_buf_t, *Pcoeff_buf_t;
//------------------------------------------------------------------------------
class jpeg_decoder;
typedef void (*Pdecode_block_func)(jpeg_decoder *, int, int, int);
//------------------------------------------------------------------------------
class progressive_block_decoder
{
public:
  static void decode_block_dc_first(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_dc_refine(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_ac_first(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
  static void decode_block_ac_refine(
    jpeg_decoder *Pd,
    int component_id, int block_x, int block_y);
};

//------------------------------------------------------------------------------
class jpeg_decoder_mem_stream
{
  char* pData;
  int	iReadCounter;
  int	iTotalSize;
  bool	eof_flag;

public:

  jpeg_decoder_mem_stream()
  {
    close();
  }

  void close()
  {
    pData = NULL;
	iReadCounter = 0;
	iTotalSize = 0;
    eof_flag = false;
  }

  ~jpeg_decoder_mem_stream()
  {
    close();
  }

  bool open(void* pbuf, int buflen)
  {
    close();

	pData = (char*)pbuf;
	iTotalSize = buflen;
	return ((pData!=0) && (iTotalSize > 0));
  }

  int readjpg(unsigned char *Pbuf, int max_bytes_to_read, bool *Peof_flag)
  {
    if (!pData)
      return (-1);

	int bytesremaining = iTotalSize - iReadCounter;

	if (eof_flag || (bytesremaining <= 0))
    {
      *Peof_flag = true;
      return (0);
    }

	int bytestocopy = (max_bytes_to_read < bytesremaining) ? max_bytes_to_read : bytesremaining;

	memcpy(Pbuf, &pData[iReadCounter], bytestocopy);
	iReadCounter += bytestocopy;

    if (iTotalSize <= iReadCounter)
    {
      eof_flag = true;
      *Peof_flag = true;
    }

    return (bytestocopy);
  }

  void attachjpg()
  {
  }

  void detachjpg()
  {
  }
};
//------------------------------------------------------------------------------
typedef jpeg_decoder_mem_stream *Pjpeg_decoder_stream;
//------------------------------------------------------------------------------



#define QUANT_TYPE short
#define BLOCK_TYPE short

//------------------------------------------------------------------------------
class jpeg_decoder
{
  friend class progressive_block_decoder;

private:

  void free_all_blocks(void);

  void terminate(int status);

  void *alloc(int n);

  void word_clear(void *p, unsigned short c, unsigned int n);

  void prep_in_buffer(void);

  void read_dht_marker(void);

  void read_dqt_marker(void);

  void read_sof_marker(void);

  void skip_variable_marker(void);

  void read_dri_marker(void);

  void read_sos_marker(void);

  int next_marker(void);

  int process_markers(void);

  void locate_soi_marker(void);

  void locate_sof_marker(void);

  int locate_sos_marker(void);

  void init(Pjpeg_decoder_stream Pstream);

  void create_look_ups(void);

  void fix_in_buffer(void);

  void transform_row(void);

  Pcoeff_buf_t coeff_buf_open(
    int block_num_x, int block_num_y,
    int block_len_x, int block_len_y);

  void coeff_buf_read(
    Pcoeff_buf_t cb,
    int block_x, int block_y,
    BLOCK_TYPE *buffer);

  void coeff_buf_write(
    Pcoeff_buf_t cb,
    int block_x, int block_y,
    BLOCK_TYPE *buffer);

  BLOCK_TYPE *coeff_buf_getp(
    Pcoeff_buf_t cb,
    int block_x, int block_y);

  void load_next_row(void);

  void decode_next_row(void);

  void make_huff_table(
    int index,
    Phuff_tables_t hs);

  void check_quant_tables(void);

  void check_huff_tables(void);

  void calc_mcu_block_order(void);

  int init_scan(void);

  void init_frame(void);

  void process_restart(void);

  void decode_scan(
    Pdecode_block_func decode_block_func);

  void init_progressive(void);

  void init_sequential(void);

  void decode_start(void);

  void decode_init(Pjpeg_decoder_stream Pstream);

  void H2V2Convert(void);
  void H2V1Convert(void);
  void H1V2Convert(void);
  void H1V1Convert(void);
  void GrayConvert(void);

  void find_eoi(void);
//------------------
  inline unsigned int rol(unsigned int i, unsigned char j);
  inline unsigned int get_char(void);
  inline unsigned int get_char(bool *Ppadding_flag);
  inline void stuff_char(unsigned char q);
  inline unsigned char get_octet(void);
  inline unsigned int get_bits_1(int num_bits);
  inline unsigned int get_bits_2(int numbits);
  inline int huff_decode(Phuff_tables_t Ph);
  inline unsigned char clamp(int i);

//------------------
  int   image_x_size;
  int   image_y_size;

  Pjpeg_decoder_stream Pstream;

  int   progressive_flag;

  unsigned char *huff_num[JPGD_MAXHUFFTABLES];  /* pointer to number of Huffman codes per bit size */
  unsigned char *huff_val[JPGD_MAXHUFFTABLES];  /* pointer to Huffman codes per bit size */

  QUANT_TYPE *quant[JPGD_MAXQUANTTABLES];    /* pointer to quantization tables */

  int   scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                           CMYK111, CMYK4114 */

  int   comps_in_frame;                 /* # of components in frame */
  int   comp_h_samp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
  int   comp_v_samp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
  int   comp_quant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
  int   comp_ident[JPGD_MAXCOMPONENTS];      /* component's ID */

  int   comp_h_blocks[JPGD_MAXCOMPONENTS];
  int   comp_v_blocks[JPGD_MAXCOMPONENTS];

  int   comps_in_scan;                  /* # of components in scan */
  int   comp_list[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
  int   comp_dc_tab[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
  int   comp_ac_tab[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

  int   spectral_start;                 /* spectral selection start */
  int   spectral_end;                   /* spectral selection end   */
  int   successive_low;                 /* successive approximation low */
  int   successive_high;                /* successive approximation high */

  int   max_mcu_x_size;                 /* MCU's max. X size in pixels */
  int   max_mcu_y_size;                 /* MCU's max. Y size in pixels */

  int   blocks_per_mcu;
  int   max_blocks_per_row;
  int   mcus_per_row, mcus_per_col;

  int   mcu_org[JPGD_MAXBLOCKSPERMCU];

  int   total_lines_left;               /* total # lines left in image */
  int   mcu_lines_left;                 /* total # lines left in this MCU */

  int   real_dest_bytes_per_scan_line;
  int   dest_bytes_per_scan_line;        /* rounded up */
  int   dest_bytes_per_pixel;            /* currently, 4 (RGB) or 1 (Y) */

  void  *blocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */

  Phuff_tables_t h[JPGD_MAXHUFFTABLES];

  Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
  Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

  int eob_run;

  int block_y_mcu[JPGD_MAXCOMPONENTS];

  unsigned char *Pin_buf_ofs;
  int in_buf_left;
  int tem_flag;
  bool eof_flag;

  unsigned char padd_1[128];
  unsigned char in_buf[JPGD_INBUFSIZE + 128];
  unsigned char padd_2[128];

  int   bits_left;
  union
  {
    unsigned int bit_buf;
    unsigned int bit_buf_64[2];
  };
  
  unsigned int  saved_mm1[2];

  bool  use_mmx_getbits;

  int   restart_interval;
  int   restarts_left;
  int   next_restart_num;

  int   max_mcus_per_row;
  int   max_blocks_per_mcu;

  int   max_mcus_per_col;

  unsigned int *component[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
  unsigned int  last_dc_val[JPGD_MAXCOMPONENTS];

  Phuff_tables_t dc_huff_seg[JPGD_MAXBLOCKSPERMCU];
  Phuff_tables_t ac_huff_seg[JPGD_MAXBLOCKSPERMCU];

  BLOCK_TYPE *block_seg[JPGD_MAXBLOCKSPERROW];
  int   block_max_zag_set[JPGD_MAXBLOCKSPERROW];

  unsigned char *Psample_buf;
  //int   block_num[JPGD_MAXBLOCKSPERROW];

  int   crr[256];
  int   cbb[256];
  int   padd;
  long  crg[256];
  long  cbg[256];

  unsigned char *scan_line_0;
  unsigned char *scan_line_1;

  BLOCK_TYPE temp_block[64];

  bool use_mmx;
  bool use_mmx_idct;
  bool mmx_active;

  int error_code;
  bool ready_flag;

  jmp_buf jmp_state;

  int total_bytes_read;

public:

  // If SUPPORT_MMX is not defined, the use_mmx flag is ignored.
  jpeg_decoder();
  jpeg_decoder(Pjpeg_decoder_stream Pstream);
  void start(Pjpeg_decoder_stream Pstream);
  void cleanup();

  int begin(void);

  int decode(void * *Pscan_line_ofs, unsigned int *Pscan_line_len);

  ~jpeg_decoder();

  int get_error_code(void)
  {
    return (error_code);
  }

  int get_width(void)
  {
    return (image_x_size);
  }

  int get_height(void)
  {
    return (image_y_size);
  }

  int get_num_components(void)
  {
    return (comps_in_frame);
  }

  int get_bytes_per_pixel(void)
  {
    return (dest_bytes_per_pixel);
  }

  int get_bytes_per_scan_line(void)
  {
    return (image_x_size * get_bytes_per_pixel());
  }

  int get_total_bytes_read(void)
  {
    return (total_bytes_read);
  }
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// inlines-- moved from .h file for clarity
//------------------------------------------------------------------------------
// Logical rotate left operation.
inline unsigned int jpeg_decoder::rol(unsigned int i, unsigned char j)
{
  return ((i << j) | (i >> (32 - j)));
}
//------------------------------------------------------------------------------
// Retrieve one character from the input stream.
inline unsigned int jpeg_decoder::get_char(void)
{
  // Any bytes remaining in buffer?
  if (!in_buf_left)
  {
    // Try to get more bytes.
    prep_in_buffer();
    // Still nothing to get?
    if (!in_buf_left)
    {
      // Padd the end of the stream with 0xFF 0xD9 (EOI marker)
      // FIXME: Is there a better padding pattern to use?
      int t = tem_flag;
      tem_flag ^= 1;
      if (t)
        return (0xD9);
      else
        return (0xFF);
    }
  }

  unsigned int c = *Pin_buf_ofs++;
  in_buf_left--;

  return (c);
}
//------------------------------------------------------------------------------
// Same as previus method, except can indicate if the character is
// a "padd" character or not.
inline unsigned int jpeg_decoder::get_char(bool *Ppadding_flag)
{
  if (!in_buf_left)
  {
    prep_in_buffer();
    if (!in_buf_left)
    {
      *Ppadding_flag = true;
      int t = tem_flag;
      tem_flag ^= 1;
      if (t)
        return (0xD9);
      else
        return (0xFF);
    }
  }

  *Ppadding_flag = false;

  unsigned int c = *Pin_buf_ofs++;
  in_buf_left--;

  return (c);
}
//------------------------------------------------------------------------------
// Inserts a previously retrieved character back into the input buffer.
inline void jpeg_decoder::stuff_char(unsigned char q)
{
  *(--Pin_buf_ofs) = q;
  in_buf_left++;
}
//------------------------------------------------------------------------------
// Retrieves one character from the input stream, but does
// not read past markers. Will continue to return 0xFF when a
// marker is encountered.
// FIXME: Bad name?
inline unsigned char jpeg_decoder::get_octet(void)
{
  bool padding_flag;
  int c = get_char(&padding_flag);

  if (c == 0xFF)
  {
    if (padding_flag)
      return (0xFF);

    c = get_char(&padding_flag);
    if (padding_flag)
    {
      stuff_char(0xFF);
      return (0xFF);
    }

    if (c == 0x00)
      return (0xFF);
    else
    {
      stuff_char(c);
      stuff_char(0xFF);
      return (0xFF);
    }
  }

  return (c);
}
//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Does not recognize markers.
inline unsigned int jpeg_decoder::get_bits_1(int num_bits)
{
  unsigned int i;

  i = (bit_buf >> (16 - num_bits)) & ((1 << num_bits) - 1);

  if ((bits_left -= num_bits) <= 0)
  {
    bit_buf = rol(bit_buf, num_bits += bits_left);

    unsigned int c1 = get_char();
    unsigned int c2 = get_char();

    bit_buf = (bit_buf & 0xFFFF) | (((unsigned long)c1) << 24) | (((unsigned long)c2) << 16);

    bit_buf = rol(bit_buf, -bits_left);

    bits_left += 16;
  }
  else
    bit_buf = rol(bit_buf, num_bits);

  return i;
}
//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Markers will not be read into the input bit buffer. Instead,
// an infinite number of all 1's will be returned when a marker
// is encountered.
// FIXME: Is it better to return all 0's instead, like the older implementation?
inline unsigned int jpeg_decoder::get_bits_2(int numbits)
{
  unsigned int i;

  i = (bit_buf >> (16 - numbits)) & ((1 << numbits) - 1);

  if ((bits_left -= numbits) <= 0)
  {
    bit_buf = rol(bit_buf, numbits += bits_left);

    unsigned int c1 = get_octet();
    unsigned int c2 = get_octet();

    bit_buf = (bit_buf & 0xFFFF) | (((unsigned long)c1) << 24) | (((unsigned long)c2) << 16);

    bit_buf = rol(bit_buf, -bits_left);

    bits_left += 16;
  }
  else
    bit_buf = rol(bit_buf, numbits);

  return i;
}
//------------------------------------------------------------------------------
// Decodes a Huffman encoded symbol.
inline int jpeg_decoder::huff_decode(Phuff_tables_t Ph)
{
  int symbol;

  // Check first 8-bits: do we have a complete symbol?
  if ((symbol = Ph->look_up[(bit_buf >> 8) & 0xFF]) < 0)
  {
    // Decode more bits, use a tree traversal to find symbol.
    get_bits_2(8);

    do
    {
      symbol = Ph->tree[~symbol + (1 - get_bits_2(1))];
    } while (symbol < 0);
  }
  else
    get_bits_2(Ph->code_size[symbol]);

  return symbol;
}
//------------------------------------------------------------------------------
// Tables and macro used to fully decode the DPCM differences.
// (Note: In x86 asm this can be done without using tables.)
const int extend_test[16] =   /* entry n is 2**(n-1) */
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

const int extend_offset[16] = /* entry n is (-1 << n) + 1 */
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };

// used by huff_extend()
const int extend_mask[] =
{
  0,
  (1<<0), (1<<1), (1<<2), (1<<3),
  (1<<4), (1<<5), (1<<6), (1<<7),
  (1<<8), (1<<9), (1<<10), (1<<11),
  (1<<12), (1<<13), (1<<14), (1<<15),
  (1<<16),
};

#define HUFF_EXTEND_TBL(x,s) ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

#define HUFF_EXTEND(x,s) HUFF_EXTEND_TBL(x,s)
#define HUFF_EXTEND_P(x,s) HUFF_EXTEND_TBL(x,s)

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Clamps a value between 0-255.
inline unsigned char jpeg_decoder::clamp(int i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);

  return (i);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef jpeg_decoder *Pjpeg_decoder;
//------------------------------------------------------------------------------
// idct.cpp
void idct(BLOCK_TYPE *data, unsigned char *Pdst_ptr);
//------------------------------------------------------------------------------
// fidctfst.cpp
void jpeg_idct_ifast (
  BLOCK_TYPE* inptr,
  short *quantptr,
  unsigned char * *outptr,
  int output_col);

void jpeg_idct_ifast_deinit(void);

bool jpeg_idct_ifast_avail(void);
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

