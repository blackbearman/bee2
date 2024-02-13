package bee2

import "testing"

func TestBeltHashKeep(t *testing.T) {
    a := BeltHash_keep()
    if a != 200 {
        t.Fatalf(`BeltHash_keep = %v, want match for %v`, a, 200)
    }
}
