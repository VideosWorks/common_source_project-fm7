// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Phil Bennett
#pragma once

#ifndef __I386INTF_H__
#define __I386INTF_H__

#define INPUT_LINE_A20      1
#define INPUT_LINE_SMI      2

struct i386_interface
{
	devcb_write_line smiact;
};

// mingw has this defined for 32-bit compiles
#undef i386

DECLARE_LEGACY_CPU_DEVICE(I386, i386);
DECLARE_LEGACY_CPU_DEVICE(I386SX, i386SX);
DECLARE_LEGACY_CPU_DEVICE(I486, i486);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM, pentium);
DECLARE_LEGACY_CPU_DEVICE(MEDIAGX, mediagx);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM_PRO, pentium_pro);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM_MMX, pentium_mmx);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM2, pentium2);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM3, pentium3);
DECLARE_LEGACY_CPU_DEVICE(PENTIUM4, pentium4);



#endif /* __I386INTF_H__ */
