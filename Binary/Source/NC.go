/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     06/22/2014,
 * Revision 06/22/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

package main

// #cgo CFLAGS: -DMOCOSEL_DEBUGGING=1 -I../../Include
// #cgo LDFLAGS: -L../../Library/Linux/x86
// #cgo LDFLAGS: -L../../Library/Linux/x64
// #cgo LDFLAGS: -lplain
//
// #include <Plain/VM.h>
import "C"

func main() {
    var environment C.struct_MOCOSEL_ENVIRONMENT
    if C.MOCOSEL_VERSION(&environment) == 0 {
        panic("Unsupported platform.")
    }
}
