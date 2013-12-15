
#ifndef HPJET_H
#define HPJET_H

typedef struct hpj_head
	{
	UBYTE format;
	UBYTE type;
	USHORT reserved;
	SHORT baseline_distance;
	SHORT cel_width;
	SHORT cel_height;
	UBYTE orientation;
	UBYTE spacing;
	SHORT symbol_set;
	SHORT pitch;
	SHORT height;
	SHORT x_height;
	UBYTE width_type;
	UBYTE style;
	UBYTE stroke_width;
	UBYTE typeface_lo;
	UBYTE typeface_hi;
	UBYTE serif_style;
	USHORT reserved2;
	UBYTE underline_distance;
	UBYTE underline_height;
	SHORT text_height;
	short text_width;
	ULONG reserved3;
	UBYTE pitch_extended;
	UBYTE height_extended;
	UBYTE cap_height;
	UBYTE reserved4[5];
	} Hpj_head;

typedef struct hpj_letter
	{
	UBYTE format;
	UBYTE continuation;
	UBYTE descriptor_size;
	UBYTE class;
	UBYTE orientation;
	UBYTE reserved;
	SHORT left_offset;
	SHORT top_offset;
	SHORT character_width;
	SHORT character_height;
	SHORT delta_x;
	} Hpj_letter;

typedef struct hfcb		/* HP Laser Jet font control block */
	{
	Hpj_letter **letters;
	Hpj_head head;
	SHORT widest;
	SHORT tallest;
	} Hfcb;

extern Hfcb *glo_hfcb;

#endif /* HPJET_H */
