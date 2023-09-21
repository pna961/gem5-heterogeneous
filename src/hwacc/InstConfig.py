# AUTO-GENERATED FILE

from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject

class InstConfig(SimObject):
	# SimObject type
	type = 'InstConfig'
	# gem5-SALAM attached header
	cxx_header = "hwacc/HWModeling/src/instruction_config.hh"

	add = Param.Add(Parent.any, "add instruction SimObject")
	addrspacecast = Param.Addrspacecast(Parent.any, "addrspacecast instruction SimObject")
	alloca = Param.Alloca(Parent.any, "alloca instruction SimObject")
	and_inst = Param.AndInst(Parent.any, "and_inst instruction SimObject")
	ashr = Param.Ashr(Parent.any, "ashr instruction SimObject")
	bitcast = Param.Bitcast(Parent.any, "bitcast instruction SimObject")
	br = Param.Br(Parent.any, "br instruction SimObject")
	call = Param.Call(Parent.any, "call instruction SimObject")
	fadd = Param.Fadd(Parent.any, "fadd instruction SimObject")
	fcmp = Param.Fcmp(Parent.any, "fcmp instruction SimObject")
	fdiv = Param.Fdiv(Parent.any, "fdiv instruction SimObject")
	fence = Param.Fence(Parent.any, "fence instruction SimObject")
	fmul = Param.Fmul(Parent.any, "fmul instruction SimObject")
	fpext = Param.Fpext(Parent.any, "fpext instruction SimObject")
	fptosi = Param.Fptosi(Parent.any, "fptosi instruction SimObject")
	fptoui = Param.Fptoui(Parent.any, "fptoui instruction SimObject")
	fptrunc = Param.Fptrunc(Parent.any, "fptrunc instruction SimObject")
	frem = Param.Frem(Parent.any, "frem instruction SimObject")
	fsub = Param.Fsub(Parent.any, "fsub instruction SimObject")
	gep = Param.Gep(Parent.any, "gep instruction SimObject")
	icmp = Param.Icmp(Parent.any, "icmp instruction SimObject")
	indirectbr = Param.Indirectbr(Parent.any, "indirectbr instruction SimObject")
	inttoptr = Param.Inttoptr(Parent.any, "inttoptr instruction SimObject")
	invoke = Param.Invoke(Parent.any, "invoke instruction SimObject")
	landingpad = Param.Landingpad(Parent.any, "landingpad instruction SimObject")
	load = Param.Load(Parent.any, "load instruction SimObject")
	lshr = Param.Lshr(Parent.any, "lshr instruction SimObject")
	mul = Param.Mul(Parent.any, "mul instruction SimObject")
	or_inst = Param.OrInst(Parent.any, "or_inst instruction SimObject")
	phi = Param.Phi(Parent.any, "phi instruction SimObject")
	ptrtoint = Param.Ptrtoint(Parent.any, "ptrtoint instruction SimObject")
	resume = Param.Resume(Parent.any, "resume instruction SimObject")
	ret = Param.Ret(Parent.any, "ret instruction SimObject")
	sdiv = Param.Sdiv(Parent.any, "sdiv instruction SimObject")
	select = Param.Select(Parent.any, "select instruction SimObject")
	sext = Param.Sext(Parent.any, "sext instruction SimObject")
	shl = Param.Shl(Parent.any, "shl instruction SimObject")
	srem = Param.Srem(Parent.any, "srem instruction SimObject")
	store = Param.Store(Parent.any, "store instruction SimObject")
	sub = Param.Sub(Parent.any, "sub instruction SimObject")
	switch_inst = Param.SwitchInst(Parent.any, "switch_inst instruction SimObject")
	trunc = Param.Trunc(Parent.any, "trunc instruction SimObject")
	udiv = Param.Udiv(Parent.any, "udiv instruction SimObject")
	uitofp = Param.Uitofp(Parent.any, "uitofp instruction SimObject")
	unreachable = Param.Unreachable(Parent.any, "unreachable instruction SimObject")
	urem = Param.Urem(Parent.any, "urem instruction SimObject")
	vaarg = Param.Vaarg(Parent.any, "vaarg instruction SimObject")
	xor_inst = Param.XorInst(Parent.any, "xor_inst instruction SimObject")
	zext = Param.Zext(Parent.any, "zext instruction SimObject")

#AUTO-GENERATED CLASSES
class Add(SimObject):
	# SimObject type
	type = 'Add'	# gem5-SALAM attached header
	cxx_header = "hwacc/HWModeling/generated/instructions/add.hh"
	# Instruction params
	functional_unit = Param.UInt32(1, "Default functional unit assignment.")
