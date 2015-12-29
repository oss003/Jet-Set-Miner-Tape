;COLOR VERSION

tiles:
        .BYTE %00000000      ;Tiles - all 8x8 1 bit per pixel, so easy to change!
        .BYTE %00000000      ;This one is air
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000
        .BYTE %00000000

        .BYTE %11111111      ;Floor that can be walked and jumped through
        .BYTE %11101110
        .BYTE %10111011
        .BYTE %11101110
        .BYTE %10111011
        .BYTE %11101110
        .BYTE %10111011
        .BYTE %11101110

        .BYTE %11111101      ;Brick wall
        .BYTE %11111101
        .BYTE %11111101
        .BYTE %01010101
        .BYTE %01111111
        .BYTE %01111111
        .BYTE %01111111
        .BYTE %01010101

        .BYTE %00000000      ;Spike
        .BYTE %00010000
        .BYTE %00010000
        .BYTE %00010001
        .BYTE %00010101
        .BYTE %01010101
        .BYTE %01010101
        .BYTE %01010101
