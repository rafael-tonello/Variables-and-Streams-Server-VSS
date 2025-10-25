package misc

import (
	"errors"
	"sync"
	"time"
)

// --- named lock manager -------------------------------------------------
// A simple named-lock implementation using buffered channels as semaphores.
var namedLocks = struct {
	mu    sync.Mutex
	locks map[string]chan struct{}
}{locks: make(map[string]chan struct{})}

func acquireNamedLock(name string, timeout time.Duration) error {
	namedLocks.mu.Lock()
	ch, ok := namedLocks.locks[name]
	if !ok {
		ch = make(chan struct{}, 1)
		// token available
		ch <- struct{}{}
		namedLocks.locks[name] = ch
	}
	namedLocks.mu.Unlock()

	if timeout <= 0 {
		<-ch
		return nil
	}
	select {
	case <-ch:
		return nil
	case <-time.After(timeout):
		return errors.New("timeout acquiring named lock")
	}
}

func releaseNamedLock(name string) {
	namedLocks.mu.Lock()
	ch, ok := namedLocks.locks[name]
	namedLocks.mu.Unlock()
	if !ok {
		return
	}
	// try to put token back, but avoid blocking if double-release
	select {
	case ch <- struct{}{}:
	default:
	}
}

func NamedLockRun(name string, timeout time.Duration, f func()) error {
	if err := acquireNamedLock(name, timeout); err != nil {
		return err
	}
	defer releaseNamedLock(name)
	f()
	return nil
}
