#include <stdio.h>
#include <stdlib.h>

#include "Utils/typedefs.h"
#include "Disassembler.h"

int main(void)
{
    CDisassembler disassembler;

    disassembler.SetSvmMode();

    unsigned count = 0;
    char line[256];

    while (gets(line))
    {
        count++;

        AMD_UINT8 instBuf[18];  // 15 for instruction bytes and 3 for dbit, longmode, rex32mode
        int len = 0;
        char* ptr = line;

#ifdef CONVEY_RIP
        AMD_UINT64 rip = strtoul(ptr, &ptr, 16);
#endif

        while ((*ptr != '\0') && (len < 18))
        {
            instBuf[len++] = (AMD_UINT8)strtoul(ptr, &ptr, 16);
        }

        disassembler.SetDbit(instBuf[len - 2]);
        disassembler.SetLongMode(instBuf[len - 1]);

#ifdef CONVEY_RIP

        if (disassembler.Disassemble(instBuf, rip) != NULL)
#else
        if (disassembler.Disassemble(instBuf) != NULL)
#endif
            printf("%s\n", disassembler.GetMnemonic());
        else
        {
            printf("%08d: Unable to disassemble (", count);
            int i;

            for (i = 0; i < (len - 2); i++)
            {
                printf("%02x ", instBuf[i]);
            }

            printf("%d ", instBuf[i++]);
            printf("%d)\n", instBuf[i]);
        }

        fflush(stdout);
    }

    return 0;
}
