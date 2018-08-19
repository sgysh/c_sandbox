.section .bindata,"a"

.global __start_bindata
__start_bindata:
.incbin "binary.dat"

.global __stop_bindata
__stop_bindata:
