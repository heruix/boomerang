int main(int argc, char *argv[]);
void rux_encrypt(__size32 param1);

/** address: 0x08048460 */
int main(int argc, char *argv[])
{
    ssize_t eax; 		// r24
    int esp; 		// r28
    int local0; 		// m[esp - 8]

    for(;;) {
        eax = read(0, &local0, 4);
        if (eax == 4) {
            rux_encrypt(esp - 8);
            write(1, &local0, 4);
        }
    }
    if (eax != 0) {
        memset(esp + eax - 8, 0, 4 - eax);
        rux_encrypt(esp - 8);
        write(1, &local0, 4);
    }
    return 0;
}

/** address: 0x08048504 */
void rux_encrypt(__size32 param1)
{
    unsigned char bl; 		// r11
    unsigned char bl_1; 		// r11{8}
    unsigned char bl_4; 		// r11{11}
    unsigned char cl; 		// r9
    __size32 esp; 		// r28
    unsigned int local0; 		// m[esp - 6]

    local0 = 0;
    while (local0 <= 3) {
        bl_1 = *((local0) + param1);
        cl = *((local0) + param1);
        cl = (cl & 0xf ^ *((local0) + 0x8049644));
        bl_4 = *((cl) + esp - 40);
        *(unsigned char*)((local0) + param1) = bl_4;
        bl = (bl_1 >> 4 ^ *((local0) + 0x8049648));
        bl = *((bl) + esp - 24);
        bl = (bl << 4 ^ *((local0) + param1));
        *(unsigned char*)((local0) + param1) = bl;
        local0++;
    }
    return;
}

