// automatically generated by stateify.

package inet

import (
	"gvisor.dev/gvisor/pkg/state"
)

func (p *NamespaceAtomicPtr) StateTypeName() string {
	return "pkg/sentry/inet.NamespaceAtomicPtr"
}

func (p *NamespaceAtomicPtr) StateFields() []string {
	return []string{
		"ptr",
	}
}

func (p *NamespaceAtomicPtr) beforeSave() {}

// +checklocksignore
func (p *NamespaceAtomicPtr) StateSave(stateSinkObject state.Sink) {
	p.beforeSave()
	var ptrValue *Namespace
	ptrValue = p.savePtr()
	stateSinkObject.SaveValue(0, ptrValue)
}

func (p *NamespaceAtomicPtr) afterLoad() {}

// +checklocksignore
func (p *NamespaceAtomicPtr) StateLoad(stateSourceObject state.Source) {
	stateSourceObject.LoadValue(0, new(*Namespace), func(y interface{}) { p.loadPtr(y.(*Namespace)) })
}

func init() {
	state.Register((*NamespaceAtomicPtr)(nil))
}
