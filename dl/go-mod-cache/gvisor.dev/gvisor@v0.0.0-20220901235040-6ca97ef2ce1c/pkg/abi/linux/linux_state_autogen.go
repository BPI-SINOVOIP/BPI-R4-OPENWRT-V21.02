// automatically generated by stateify.

package linux

import (
	"gvisor.dev/gvisor/pkg/state"
)

func (i *IOEvent) StateTypeName() string {
	return "pkg/abi/linux.IOEvent"
}

func (i *IOEvent) StateFields() []string {
	return []string{
		"Data",
		"Obj",
		"Result",
		"Result2",
	}
}

func (i *IOEvent) beforeSave() {}

// +checklocksignore
func (i *IOEvent) StateSave(stateSinkObject state.Sink) {
	i.beforeSave()
	stateSinkObject.Save(0, &i.Data)
	stateSinkObject.Save(1, &i.Obj)
	stateSinkObject.Save(2, &i.Result)
	stateSinkObject.Save(3, &i.Result2)
}

func (i *IOEvent) afterLoad() {}

// +checklocksignore
func (i *IOEvent) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &i.Data)
	stateSourceObject.Load(1, &i.Obj)
	stateSourceObject.Load(2, &i.Result)
	stateSourceObject.Load(3, &i.Result2)
}

func (b *BPFInstruction) StateTypeName() string {
	return "pkg/abi/linux.BPFInstruction"
}

func (b *BPFInstruction) StateFields() []string {
	return []string{
		"OpCode",
		"JumpIfTrue",
		"JumpIfFalse",
		"K",
	}
}

func (b *BPFInstruction) beforeSave() {}

// +checklocksignore
func (b *BPFInstruction) StateSave(stateSinkObject state.Sink) {
	b.beforeSave()
	stateSinkObject.Save(0, &b.OpCode)
	stateSinkObject.Save(1, &b.JumpIfTrue)
	stateSinkObject.Save(2, &b.JumpIfFalse)
	stateSinkObject.Save(3, &b.K)
}

func (b *BPFInstruction) afterLoad() {}

// +checklocksignore
func (b *BPFInstruction) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &b.OpCode)
	stateSourceObject.Load(1, &b.JumpIfTrue)
	stateSourceObject.Load(2, &b.JumpIfFalse)
	stateSourceObject.Load(3, &b.K)
}

func (s *SigAction) StateTypeName() string {
	return "pkg/abi/linux.SigAction"
}

func (s *SigAction) StateFields() []string {
	return []string{
		"Handler",
		"Flags",
		"Restorer",
		"Mask",
	}
}

func (s *SigAction) beforeSave() {}

// +checklocksignore
func (s *SigAction) StateSave(stateSinkObject state.Sink) {
	s.beforeSave()
	stateSinkObject.Save(0, &s.Handler)
	stateSinkObject.Save(1, &s.Flags)
	stateSinkObject.Save(2, &s.Restorer)
	stateSinkObject.Save(3, &s.Mask)
}

func (s *SigAction) afterLoad() {}

// +checklocksignore
func (s *SigAction) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &s.Handler)
	stateSourceObject.Load(1, &s.Flags)
	stateSourceObject.Load(2, &s.Restorer)
	stateSourceObject.Load(3, &s.Mask)
}

func (s *SignalStack) StateTypeName() string {
	return "pkg/abi/linux.SignalStack"
}

func (s *SignalStack) StateFields() []string {
	return []string{
		"Addr",
		"Flags",
		"Size",
	}
}

func (s *SignalStack) beforeSave() {}

// +checklocksignore
func (s *SignalStack) StateSave(stateSinkObject state.Sink) {
	s.beforeSave()
	stateSinkObject.Save(0, &s.Addr)
	stateSinkObject.Save(1, &s.Flags)
	stateSinkObject.Save(2, &s.Size)
}

func (s *SignalStack) afterLoad() {}

// +checklocksignore
func (s *SignalStack) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &s.Addr)
	stateSourceObject.Load(1, &s.Flags)
	stateSourceObject.Load(2, &s.Size)
}

func (s *SignalInfo) StateTypeName() string {
	return "pkg/abi/linux.SignalInfo"
}

func (s *SignalInfo) StateFields() []string {
	return []string{
		"Signo",
		"Errno",
		"Code",
		"Fields",
	}
}

func (s *SignalInfo) beforeSave() {}

// +checklocksignore
func (s *SignalInfo) StateSave(stateSinkObject state.Sink) {
	s.beforeSave()
	stateSinkObject.Save(0, &s.Signo)
	stateSinkObject.Save(1, &s.Errno)
	stateSinkObject.Save(2, &s.Code)
	stateSinkObject.Save(3, &s.Fields)
}

func (s *SignalInfo) afterLoad() {}

// +checklocksignore
func (s *SignalInfo) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &s.Signo)
	stateSourceObject.Load(1, &s.Errno)
	stateSourceObject.Load(2, &s.Code)
	stateSourceObject.Load(3, &s.Fields)
}

func (c *ControlMessageIPPacketInfo) StateTypeName() string {
	return "pkg/abi/linux.ControlMessageIPPacketInfo"
}

func (c *ControlMessageIPPacketInfo) StateFields() []string {
	return []string{
		"NIC",
		"LocalAddr",
		"DestinationAddr",
	}
}

func (c *ControlMessageIPPacketInfo) beforeSave() {}

// +checklocksignore
func (c *ControlMessageIPPacketInfo) StateSave(stateSinkObject state.Sink) {
	c.beforeSave()
	stateSinkObject.Save(0, &c.NIC)
	stateSinkObject.Save(1, &c.LocalAddr)
	stateSinkObject.Save(2, &c.DestinationAddr)
}

func (c *ControlMessageIPPacketInfo) afterLoad() {}

// +checklocksignore
func (c *ControlMessageIPPacketInfo) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &c.NIC)
	stateSourceObject.Load(1, &c.LocalAddr)
	stateSourceObject.Load(2, &c.DestinationAddr)
}

func (c *ControlMessageIPv6PacketInfo) StateTypeName() string {
	return "pkg/abi/linux.ControlMessageIPv6PacketInfo"
}

func (c *ControlMessageIPv6PacketInfo) StateFields() []string {
	return []string{
		"Addr",
		"NIC",
	}
}

func (c *ControlMessageIPv6PacketInfo) beforeSave() {}

// +checklocksignore
func (c *ControlMessageIPv6PacketInfo) StateSave(stateSinkObject state.Sink) {
	c.beforeSave()
	stateSinkObject.Save(0, &c.Addr)
	stateSinkObject.Save(1, &c.NIC)
}

func (c *ControlMessageIPv6PacketInfo) afterLoad() {}

// +checklocksignore
func (c *ControlMessageIPv6PacketInfo) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &c.Addr)
	stateSourceObject.Load(1, &c.NIC)
}

func (i *ICMP6Filter) StateTypeName() string {
	return "pkg/abi/linux.ICMP6Filter"
}

func (i *ICMP6Filter) StateFields() []string {
	return []string{
		"Filter",
	}
}

func (i *ICMP6Filter) beforeSave() {}

// +checklocksignore
func (i *ICMP6Filter) StateSave(stateSinkObject state.Sink) {
	i.beforeSave()
	stateSinkObject.Save(0, &i.Filter)
}

func (i *ICMP6Filter) afterLoad() {}

// +checklocksignore
func (i *ICMP6Filter) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &i.Filter)
}

func (t *KernelTermios) StateTypeName() string {
	return "pkg/abi/linux.KernelTermios"
}

func (t *KernelTermios) StateFields() []string {
	return []string{
		"InputFlags",
		"OutputFlags",
		"ControlFlags",
		"LocalFlags",
		"LineDiscipline",
		"ControlCharacters",
		"InputSpeed",
		"OutputSpeed",
	}
}

func (t *KernelTermios) beforeSave() {}

// +checklocksignore
func (t *KernelTermios) StateSave(stateSinkObject state.Sink) {
	t.beforeSave()
	stateSinkObject.Save(0, &t.InputFlags)
	stateSinkObject.Save(1, &t.OutputFlags)
	stateSinkObject.Save(2, &t.ControlFlags)
	stateSinkObject.Save(3, &t.LocalFlags)
	stateSinkObject.Save(4, &t.LineDiscipline)
	stateSinkObject.Save(5, &t.ControlCharacters)
	stateSinkObject.Save(6, &t.InputSpeed)
	stateSinkObject.Save(7, &t.OutputSpeed)
}

func (t *KernelTermios) afterLoad() {}

// +checklocksignore
func (t *KernelTermios) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &t.InputFlags)
	stateSourceObject.Load(1, &t.OutputFlags)
	stateSourceObject.Load(2, &t.ControlFlags)
	stateSourceObject.Load(3, &t.LocalFlags)
	stateSourceObject.Load(4, &t.LineDiscipline)
	stateSourceObject.Load(5, &t.ControlCharacters)
	stateSourceObject.Load(6, &t.InputSpeed)
	stateSourceObject.Load(7, &t.OutputSpeed)
}

func (w *WindowSize) StateTypeName() string {
	return "pkg/abi/linux.WindowSize"
}

func (w *WindowSize) StateFields() []string {
	return []string{
		"Rows",
		"Cols",
	}
}

func (w *WindowSize) beforeSave() {}

// +checklocksignore
func (w *WindowSize) StateSave(stateSinkObject state.Sink) {
	w.beforeSave()
	stateSinkObject.Save(0, &w.Rows)
	stateSinkObject.Save(1, &w.Cols)
}

func (w *WindowSize) afterLoad() {}

// +checklocksignore
func (w *WindowSize) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.Load(0, &w.Rows)
	stateSourceObject.Load(1, &w.Cols)
}

func init() {
	state.Register((*IOEvent)(nil))
	state.Register((*BPFInstruction)(nil))
	state.Register((*SigAction)(nil))
	state.Register((*SignalStack)(nil))
	state.Register((*SignalInfo)(nil))
	state.Register((*ControlMessageIPPacketInfo)(nil))
	state.Register((*ControlMessageIPv6PacketInfo)(nil))
	state.Register((*ICMP6Filter)(nil))
	state.Register((*KernelTermios)(nil))
	state.Register((*WindowSize)(nil))
}
