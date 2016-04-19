//===============================================================================

//

// Copyright(c) 2015 Advanced Micro Devices, Inc.All Rights Reserved

//

// Permission is hereby granted, free of charge, to any person obtaining a copy

// of this software and associated documentation files(the "Software"), to deal

// in the Software without restriction, including without limitation the rights

// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell

// copies of the Software, and to permit persons to whom the Software is

// furnished to do so, subject to the following conditions :

//

// The above copyright notice and this permission notice shall be included in

// all copies or substantial portions of the Software.

//

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR

// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,

// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE

// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER

// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,

// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN

// THE SOFTWARE.

//

//=================================================================================



#ifndef _PWR_PROF_DRV_ASM_H_

#define _PWR_PROF_DRV_ASM_H_



#ifdef __x86_64



#undef read_cr2

#define read_cr2() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%cr2,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#undef read_cr3

#define read_cr3() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%cr3,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr0() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr0,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr1() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr1,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr2() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr2,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr3() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr3,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr4() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr4,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr5() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr5,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr6() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr6,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr7() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq %%dr7,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#undef write_cr2

#undef write_cr3

#define write_cr2(x) __asm__ __volatile__("movq %0,%%cr2": :"r" (x));

#define write_cr3(x) __asm__ __volatile__("movq %0,%%cr3": :"r" (x));

#define write_dr0(x) __asm__ __volatile__("movq %0,%%dr0": :"r" (x));

#define write_dr1(x) __asm__ __volatile__("movq %0,%%dr1": :"r" (x));

#define write_dr2(x) __asm__ __volatile__("movq %0,%%dr2": :"r" (x));

#define write_dr3(x) __asm__ __volatile__("movq %0,%%dr3": :"r" (x));

#define write_dr4(x) __asm__ __volatile__("movq %0,%%dr4": :"r" (x));

#define write_dr5(x) __asm__ __volatile__("movq %0,%%dr5": :"r" (x));

#define write_dr6(x) __asm__ __volatile__("movq %0,%%dr6": :"r" (x));

#define write_dr7(x) __asm__ __volatile__("movq %0,%%dr7": :"r" (x));



#ifndef AMD_TARGET_PUBLIC

#define rdsasmsrd(msr,ebxValue,esiValue,password,result) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             :"=a"(result) \

                             :"b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })



#define rdsasmsrq(msr,ebxValue,esiValue,password,result) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             "shlq $32, %%rdx\n" \

                             "movl %%eax, %%eax\n" \

                             "orq  %%rdx, %%rax\n" \

                             :"=A"(result) \

                             :"b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })



#define wrsasmsrd(msr,ebxValue,esiValue,password,value) ({ \

        __asm__ __volatile__(\

                             "wrmsr\n" \

                             : \

                             :"a"(value), "b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })



#define wrsasmsrq(msr,ebxValue,esiValue,password,value) ({ \

        __asm__ __volatile__(\

                             "movq %%rax, %%rdx\n" \

                             "shrq $32, %%rdx\n" \

                             "wrmsr\n" \

                             : \

                             :"A"(value), "b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })

#endif

#define flushtlball_and_wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "movq %%cr4,%%rbx\n\t" \

                             "xorq $0x00000040, %%rbx\n\t" \

                             "movq %%rbx, %%cr4\n\t" \

                             "xorq $0x00000040, %%rbx\n\t" \

                             "movq %%rbx, %%cr4\n\t" \

                             "wrmsr\n\t" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                             :"rbx" \

                            ); \

    })



#define flushtlblocal_and_wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "movq %%cr3,%%rbx\n\t" \

                             "movq %%rbx,%%cr3\n\t" \

                             "wrmsr\n" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                             :"rbx" \

                            ); \

    })



#define read_qword(address) ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movq (%1),%0\n\t" \

                             :"=a"(__dummy) \

                             :"r"(address) \

                            ); \

        __dummy; \

    })



#define write_qword(address,value) __asm__ __volatile__("movq %0,(%1)": :"a" (value), "r" (address) );



#else



#define read_cr2() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%cr2,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })

#define read_cr3() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%cr3,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr0() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr0,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr1() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr1,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr2() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr2,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr3() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr3,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr4() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr4,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr5() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr5,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr6() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr6,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define read_dr7() ({ \

        unsigned long __dummy; \

        __asm__ __volatile__(\

                             "movl %%dr7,%0\n\t" \

                             :"=r"(__dummy)); \

        __dummy; \

    })



#define write_cr2(x) __asm__ __volatile__("movl %0,%%cr2": :"r" (x));

#define write_cr3(x) __asm__ __volatile__("movl %0,%%cr3": :"r" (x));

#define write_dr0(x) __asm__ __volatile__("movl %0,%%dr0": :"r" (x));

#define write_dr1(x) __asm__ __volatile__("movl %0,%%dr1": :"r" (x));

#define write_dr2(x) __asm__ __volatile__("movl %0,%%dr2": :"r" (x));

#define write_dr3(x) __asm__ __volatile__("movl %0,%%dr3": :"r" (x));

#define write_dr4(x) __asm__ __volatile__("movl %0,%%dr4": :"r" (x));

#define write_dr5(x) __asm__ __volatile__("movl %0,%%dr5": :"r" (x));

#define write_dr6(x) __asm__ __volatile__("movl %0,%%dr6": :"r" (x));

#define write_dr7(x) __asm__ __volatile__("movl %0,%%dr7": :"r" (x));



#ifndef AMD_TARGET_PUBLIC

#define rdsasmsrd(msr,ebxValue,esiValue,password,result) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             :"=a"(result) \

                             :"b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })



#define rdsasmsrq(msr,ebxValue,esiValue,password,result) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             :"=A"(result) \

                             :"b"(ebxValue), "c"(msr), "D"(password), "S"(esiValue) \

                            ); \

    })



#define wrsasmsrd(msr,ebxValue,esiValue,password,value) ({ \

        __asm__ __volatile__(\

                             "wrmsr\n" \

                             : \

                             :"a"(value), "c"(msr), "b"(ebxValue), "D"(password), "S"(esiValue) \

                            ); \

    })



#define wrsasmsrq(msr,ebxValue,esiValue,password,value) ({ \

        __asm__ __volatile__(\

                             "wrmsr\n" \

                             : \

                             :"A"(value), "c"(msr), "b"(ebxValue), "D"(password), "S"(esiValue) \

                            ); \

    })

#endif

#define flushtlball_and_wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "movl %%cr4,%%ebx\n\t" \

                             "xorl $0x00000040, %%ebx\n\t" \

                             "movl %%ebx, %%cr4\n\t" \

                             "xorl $0x00000040, %%ebx\n\t" \

                             "movl %%ebx, %%cr4\n\t" \

                             "wrmsr\n\t" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                             :"ebx" \

                            ); \

    })



#define flushtlblocal_and_wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "movl %%cr3,%%ebx\n\t" \

                             "movl %%ebx,%%cr3\n\t" \

                             "wrmsr\n" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                             :"ebx" \

                            ); \

    })



#endif



#define rdmsrq(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             :"=a"(val1), "=d"(val2), "=S"(val3), "=D"(val4) \

                             :"c"(msr) \

                            ); \

    })



#define wrmsrq(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "wrmsr\n" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "S"(val3), "D"(val4) \

                            ); \

    })



#define rdmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "rdmsr\n" \

                             :"=a"(val1), "=d"(val2) \

                             :"c"(msr), "D"(val3), "S"(val4) \

                            ); \

    })



#define wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "wrmsr\n" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                            ); \

    })



#define flushcache_and_wrmsrpw(msr,val1,val2,val3,val4) ({ \

        __asm__ __volatile__(\

                             "wbinvd\n" \

                             "wrmsr\n" \

                             : \

                             :"c"(msr), "a"(val1), "d"(val2), "D"(val3), "S"(val4) \

                            ); \

    })



#define read_byte(address) ({ \

        unsigned char __dummy; \

        __asm__ __volatile__(\

                             "movb (%1),%0\n\t" \

                             :"=a"(__dummy) \

                             :"r"(address) \

                            ); \

        __dummy; \

    })



#define read_word(address) ({ \

        unsigned short int __dummy; \

        __asm__ __volatile__(\

                             "movw (%1),%0\n\t" \

                             :"=a"(__dummy) \

                             :"r"(address) \

                            ); \

        __dummy; \

    })



#define read_dword(address) ({ \

        unsigned int __dummy; \

        __asm__ __volatile__(\

                             "movl (%1),%0\n\t" \

                             :"=a"(__dummy) \

                             :"r"(address) \

                            ); \

        __dummy; \

    })



#define write_byte(address,value) __asm__ __volatile__("movb %0,(%1)": :"a" (value), "r" (address) );

#define write_word(address,value) __asm__ __volatile__("movw %0,(%1)": :"a" (value), "r" (address) );

#define write_dword(address,value) __asm__ __volatile__("movl %0,(%1)": :"a" (value), "r" (address) );



#endif  // _PWR_PROF_DRV_ASM_H_

