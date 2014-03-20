
#ifndef TWEENPUL_H
#define TWEENPUL_H

#define TWEEN_BOTH 0
#define TWEEN_START 1
#define TWEEN_END 2

typedef enum tween_tool_ids 
	{
	TTI_POLY,
	TTI_SHAPE,
	TTI_STAR,
	TTI_PETAL,
	TTI_RPOLY,
	TTI_OVAL,
	TTI_MPOINT,
	TTI_MAGNET,
	TTI_BLOW,
	TTI_MSHAPE,
	TTI_MTWEEN,
	TTI_SSHAPE,
	TTI_STWEEN,
	TTI_LINK,
	} Tween_tool_ids;

Boolean tween_got_either();
Boolean tween_renderable();
Boolean got_tween();
void twe_go_tool(Button *b);

void tween_undraw();
void tween_redraw();
void twe_init_menu();

#define TWE_PUL	100
#define TWE_UND_PUL	101
#define TWE_ONC_PUL	102
#define TWE_LOO_PUL	103
#define TWE_REN_PUL	104
#define TWE_TRA_PUL	105
#define TWE_END_PUL	106
#define TWE_SWA_PUL	107
#define TWE_CLE_PUL	108
#define TWE_FIL_PUL	109
#define TWE_QUI_PUL	110
#define SHA_PUL	200
#define SHA_POL_PUL	201
#define SHA_SHA_PUL	202
#define SHA_STA_PUL	203
#define SHA_PET_PUL	204
#define SHA_RPO_PUL	205
#define SHA_OVA_PUL	206
#define SHA_REV_PUL	208
#define SHA_USE_PUL	209
#define SHA_LOA_PUL	210
#define SHA_SAV_PUL	211
#define MOV_PUL	300
#define MOV_MVP_PUL	301
#define MOV_MAG_PUL	302
#define MOV_BLO_PUL	303
#define MOV_MVS_PUL	304
#define MOV_MVT_PUL	305
#define MOV_SZS_PUL	306
#define MOV_SZT_PUL	307
#define MOV_LIN_PUL	308
#define MOV_CLE_PUL	309
#define OPT_PUL	400
#define OPT_CLO_PUL	401
#define OPT_SPL_PUL	402
#define OPT_TWO_PUL	403
#define OPT_IN__PUL	405
#define OPT_OUT_PUL	406
#define OPT_STI_PUL	407
#define OPT_PIN_PUL	408
#define OPT_REV_PUL	409
#define OPT_COM_PUL	410
#define ACT_PUL	500
#define ACT_STA_PUL	501
#define ACT_END_PUL	502
#define ACT_BOT_PUL	503

#endif /* TWEENPUL_H */
