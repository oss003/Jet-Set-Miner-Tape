;MONOCHROME VERSION

tiles:
        .BYTE %00000000      ;Tiles - all 8x8 1 bit per pixel, so easy to change!
        .BYTE %00000000      ;This one is air
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000

;        .BYTE %11000011      ;Floor that can be walked and jumped through
;        .BYTE %01100110
;        .BYTE %00111100
;        .BYTE %01100110
;        .BYTE %00000000
;        .BYTE %10011001
;        .BYTE %11111111
;        .BYTE %00000000

        .BYTE %11111111      ;Floor that can be walked and jumped through
        .BYTE %10101011
        .BYTE %11010101
        .BYTE %10101011
        .BYTE %11010101
        .BYTE %10101011
        .BYTE %11010101
        .BYTE %11111111

        .BYTE %11101110      ;Brick wall
        .BYTE %11101110
        .BYTE %11101110
        .BYTE %00000000
        .BYTE %01110111
        .BYTE %01110111
        .BYTE %01110111
        .BYTE %00000000

        .BYTE %00010000      ;Spike
        .BYTE %00010000
        .BYTE %00111000
        .BYTE %00111000
        .BYTE %01111100
        .BYTE %01111100
        .BYTE %11111110
        .BYTE %11111110
