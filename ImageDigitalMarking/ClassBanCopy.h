//*****************************************************************************
// FILE: ClassBanCopy.h
// DESC: Defines a macro to ban class copy.
// 
// By Giovanni Dicanio <giovanni.dicanio@gmail.com>
// 2008, September 26th
//*****************************************************************************

#pragma once


//-----------------------------------------------------------------------------
// Ban class copy, making copy constructor and operator= private
//-----------------------------------------------------------------------------
#define CLASS_BAN_COPY( className )                 \
    private:                                        \
        className( const className & );             \
        className & operator=( const className & );

