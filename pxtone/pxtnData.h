// '17/10/11 pxtnData.

// ewan '20/9/13 add endian

#ifndef pxtnData_H
#define pxtnData_H

#include "pxtn.h"

typedef bool (* pxtnIO_r   )( void* user,       void* p_dst, int32_t size, int32_t num );
typedef bool (* pxtnIO_w   )( void* user, const void* p_dst, int32_t size, int32_t num );
typedef bool (* pxtnIO_seek)( void* user,       int   mode , int32_t size              );
typedef bool (* pxtnIO_pos )( void* user,                    int32_t* p_pos            );

// endian changes begin
#if defined(__BYTE_ORDER__)
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define _is_big_endian() true
#else
#define _is_big_endian() false
#endif
#else
bool _is_big_endian();
#endif
// endian changes end

class pxtnData
{
private:

	void operator = (const pxtnData& src){}
	pxtnData        (const pxtnData& src){}

protected:

	bool        _b_init  ;

	pxtnIO_r    _io_read ;
	pxtnIO_w    _io_write;
	pxtnIO_seek _io_seek ;
	pxtnIO_pos  _io_pos  ;

	void _release();

	bool    _data_w_v     ( void* desc, int32_t   v, int32_t* p_add ) const;
	bool    _data_r_v     ( void* desc, int32_t* pv                 ) const;
	bool    _data_get_size( void* desc, int32_t* p_size             ) const;
	int32_t _data_check_v_size( uint32_t v ) const;

	void _set_io_funcs( pxtnIO_r io_read, pxtnIO_w io_write, pxtnIO_seek io_seek, pxtnIO_pos io_pos );

public :

	pxtnData();
	virtual ~pxtnData();

	bool copy_from( const pxtnData* src );

	bool init();
	bool Xxx ();

	// endian changes begin
	static void _correct_endian(unsigned char* correct, int elem_size, int elem_count);
	// endian changes end
};

#endif
