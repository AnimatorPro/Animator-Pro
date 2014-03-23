#ifndef FILEMENU_H
#define FILEMENU_H

struct button;

enum file_types {
	FTP_FLIC = 0,
	FTP_PIC = 1,
	FTP_CEL = 2,
	FTP_PALETTE = 3,
	FTP_TEXT = 4,
	FTP_FONT = 5,
	FTP_POLY = 6,
	FTP_PATH = 7,
	FTP_OPTICS = 8,
	FTP_SETTINGS = 9,
	FTP_MASK = 10,
	FTP_MACRO = 11,
	FTP_TWEEN = 12,
};

extern void go_files(int type);
extern void mb_go_files(struct button *b);

#endif
