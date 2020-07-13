.intel_syntax noprefix

.global moore
.global _moore // extern function name must start with underscore if compiled on mac

.text


moore:
_moore: // mac compatibility

ret