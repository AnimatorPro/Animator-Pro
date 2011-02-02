/* tell wcc to put underscores before symbol not after */
#pragma aux default parm caller [] modify [ax dx];
#pragma aux copy_bytes "_*" parm caller [] modify [ax dx];
#pragma aux exchange_words "_*" parm caller [] modify [ax dx];
#pragma aux back_scan "_*" parm caller [] modify [ax dx];
#pragma aux wait_vblank "_*" parm caller [] modify [ax dx];
#pragma aux jset_colors "_*" parm caller [] modify [ax dx];
#pragma aux fcuncomp "_*" parm caller [] modify [ax dx];
#pragma aux cset_colors "_*" parm caller [] modify [ax dx];
#pragma aux sysint "_*" parm caller [] modify [ax cx dx];
#pragma aux norm_pointer "_*" parm caller [] modify [ax dx];
#pragma aux enorm_pointer "_*" parm caller [] modify [ax dx];
#pragma aux stuff_words "_*" parm caller [] modify [ax dx];
#pragma aux xor_words "_*" parm caller [] modify [ax dx];
#pragma aux copy_structure "_*" parm caller [] modify [ax dx];
#pragma aux copy_words "_*" parm caller [] modify [ax dx];
#pragma aux xlat "_*" parm caller [] modify [ax dx];
#pragma aux bsame "_*" parm caller [] modify [ax dx];
#pragma aux fsame "_*" parm caller [] modify [ax dx];
#pragma aux bcontrast "_*" parm caller [] modify [ax dx];
#pragma aux fcontrast "_*" parm caller [] modify [ax dx];
#pragma aux bcompare "_*" parm caller [] modify [ax dx];
#pragma aux fcompare "_*" parm caller [] modify [ax dx];
#pragma aux conv_screen "_*" parm caller [] modify [ax dx];
#pragma aux blit8 "_*" parm caller [] modify [ax dx];
#pragma aux unrun "_*" parm caller [] modify [ax dx];
#pragma aux unsbsrsccomp "_*" parm caller [] modify [ax dx];
#pragma aux unlccomp "_*" parm caller [] modify [ax dx];
#pragma aux a2blit "_*" parm caller [] modify [ax dx];
#pragma aux xorblock "_*" parm caller [] modify [ax dx];
#pragma aux cblock "_*" parm caller [] modify [ax dx];
#pragma aux chli "_*" parm caller [] modify [ax dx];
#pragma aux cvli "_*" parm caller [] modify [ax dx];
#pragma aux getd "_*" parm caller [] modify [ax dx];
#pragma aux cdot "_*" parm caller [] modify [ax dx];
#pragma aux xordot "_*" parm caller [] modify [ax dx];
#pragma aux closestc "_*" parm caller [] modify [ax dx];
#pragma aux tnskip "_*" parm caller [] modify [ax dx];
#pragma aux tnsame "_*" parm caller [] modify [ax dx];
#pragma aux unbrun "_*" parm caller [] modify [ax dx];
#pragma aux a1blit "_*" parm caller [] modify [ax dx];
#pragma aux wait_novblank "_*" parm caller [] modify [ax dx];
#ifdef SOON
#endif /* SOON */

#define _toupper(c)	((c) + 'A' - 'a')
#define _tolower(c)	((c) + 'a' - 'A')
