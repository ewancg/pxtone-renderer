// '12/03/03

#include "./pxtnOverDrive.h"

pxtnOverDrive::pxtnOverDrive( pxtnIO_r io_read, pxtnIO_w io_write, pxtnIO_seek io_seek, pxtnIO_pos io_pos )
{
    _set_io_funcs( io_read, io_write, io_seek, io_pos );

    _b_played = true;
}

pxtnOverDrive::~pxtnOverDrive()
{
}

float   pxtnOverDrive::get_cut  ()const{ return _cut_f; }
float   pxtnOverDrive::get_amp  ()const{ return _amp_f; }
int32_t pxtnOverDrive::get_group()const{ return _group; }

void  pxtnOverDrive::Set( float cut, float amp, int32_t group )
{
    _cut_f = cut  ;
    _amp_f = amp  ;
    _group = group;
}

bool pxtnOverDrive::get_played()const{ return _b_played; }
void pxtnOverDrive::set_played( bool b ){ _b_played = b; }
bool pxtnOverDrive::switch_played(){ _b_played = _b_played ? false : true; return _b_played; }

void pxtnOverDrive::Tone_Ready()
{
    _cut_16bit_top  = (int32_t)trunc( 32767 * ( 100 - _cut_f ) / 100 );
}

void pxtnOverDrive::Tone_Supple( int32_t *group_smps ) const
{
    if( !_b_played ) return;
    int32_t work = group_smps[ _group ];
    if(      work >  _cut_16bit_top ) work =   _cut_16bit_top;
    else if( work < -_cut_16bit_top ) work =  -_cut_16bit_top;
    group_smps[ _group ] = (int32_t)trunc( (float)work * _amp_f );
}


// (8byte) =================
typedef struct
{
    uint16_t   xxx  ;
    uint16_t   group;
    float cut  ;
    float amp  ;
    float yyy  ;
}
_OVERDRIVESTRUCT;

bool pxtnOverDrive::Write( void* desc ) const
{
    _OVERDRIVESTRUCT over;
    int32_t              size;

    memset( &over, 0, sizeof( _OVERDRIVESTRUCT ) );
    over.cut   = _cut_f;
    over.amp   = _amp_f;
    over.group = (uint16_t)_group;

    // dela ----------
    size = sizeof( _OVERDRIVESTRUCT );
    if( !_io_write( desc, &size, sizeof(uint32_t), 1 ) ) return false;
    // OPNA2608 EDIT
    // this breaks on big-endian, need to write struct members separately
    // if( !_io_write( desc, &over, size,        1 ) ) return false;
    if( !_io_write( desc, &over.xxx, sizeof(uint16_t),        1 ) ) return false;
    if( !_io_write( desc, &over.group, sizeof(uint16_t),        1 ) ) return false;
    if( !_io_write( desc, &over.cut, sizeof(float),        1 ) ) return false;
    if( !_io_write( desc, &over.amp, sizeof(float),        1 ) ) return false;
    if( !_io_write( desc, &over.yyy, sizeof(float),        1 ) ) return false;

    return true;
}

pxtnERR pxtnOverDrive::Read( void* desc )
{
    _OVERDRIVESTRUCT over = {0};
    int32_t          size =  0 ;

    memset( &over, 0, sizeof(_OVERDRIVESTRUCT) );
    if( !_io_read( desc, &size, 4,                        1 ) ) return pxtnERR_desc_r;
    // OPNA2608 EDIT
    // this breaks on big-endian, need to read struct members separately
    // if( !_io_read( desc, &over, sizeof(_OVERDRIVESTRUCT), 1 ) ) return pxtnERR_desc_r;
    if( !_io_read( desc, &over.xxx, sizeof(uint16_t), 1 ) ) return pxtnERR_desc_r;
    if( !_io_read( desc, &over.group, sizeof(uint16_t), 1 ) ) return pxtnERR_desc_r;
    if( !_io_read( desc, &over.cut, sizeof(float), 1 ) ) return pxtnERR_desc_r;
    if( !_io_read( desc, &over.amp, sizeof(float), 1 ) ) return pxtnERR_desc_r;
    if( !_io_read( desc, &over.yyy, sizeof(float), 1 ) ) return pxtnERR_desc_r;

    if( over.xxx                         ) return pxtnERR_fmt_unknown;
    if( over.yyy                         ) return pxtnERR_fmt_unknown;
    if( over.cut > TUNEOVERDRIVE_CUT_MAX || over.cut < TUNEOVERDRIVE_CUT_MIN ) return pxtnERR_fmt_unknown;
    if( over.amp > TUNEOVERDRIVE_AMP_MAX || over.amp < TUNEOVERDRIVE_AMP_MIN ) return pxtnERR_fmt_unknown;

    _cut_f = over.cut  ;
    _amp_f = over.amp  ;
    _group = over.group;

    return pxtnOK;
}
