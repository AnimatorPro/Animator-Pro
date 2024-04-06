//
// Created by kiki on 2024-04-06.
//

#ifndef ANIMATOR_PRO_QPOCO_H
#define ANIMATOR_PRO_QPOCO_H

#include "stdtypes.h"

/* from qpoco.c */
void report_err_in_file(char *filename);
Errcode run_poco_stripped_environment(char *source_name);
Errcode qrun_poco(char *sourcename, bool edit_err);
Errcode compile_cl_poco(char *name);
Errcode do_cl_poco(char *name);
Errcode qrun_pocofile(char *poco_path, bool editable);
void go_pgmn(void);
Errcode quse_poco();

/* from qpocoed.c */
bool qedit_poco(long line, int cpos);

#endif // ANIMATOR_PRO_QPOCO_H
