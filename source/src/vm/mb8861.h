#ifndef _MB8861_H_ 
#define _MB8861_H_

#include "mc6800.h"
#include "device.h"

class DEBUGGER;
class FIFO;
//#endif
class MB8861 : public MC6800
{
private:
#define XX 5 // invalid opcode unknown cc
	const uint8_t cycles[256] = {
		XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
		2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
		6, 8, 8, 6, 6, 8, 6, 6, 6, 6, 6, 7, 6, 6, 3, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 4,XX, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 7,XX, 5, 6
	};
#undef XX // invalid opcode unknown cc
protected:
	void insn(uint8_t code) override;

	void nim_ix();
	void oim_ix_mb8861();
	void xim_ix();
	void tmm_ix();
	void adx_im();
	void adx_ex();

public:
	MB8861(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MC6800(parent_vm, parent_emu)
	{
		set_device_name(_T("MB8861 MPU"));
	}
	~MB8861() {}
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len) override;


};

#endif
