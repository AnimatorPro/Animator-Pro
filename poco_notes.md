# Functions that call po_say_fatal / longjmp by file

```
poco/bcutil.c:
 (Does not call po_say_fatal)

poco/bop.c:
  enforce_simple
  po_get_binop_expression

poco/chopper.c:
  po_get_csource_line

poco/code.c:
  (Does not call po_say_fatal)

poco/declare.c:
  dirdcl
  based_decs
  array_decl
  po_get_base_type
  check_params_same
  check_dupe_proto

poco/dummylib.c:
 (Does not call po_say_fatal)

poco/fold.c:
  po_eval_const_expression
  po_fold_const
  po_is_static_init_const

poco/funccall.c:
  po_get_param_size
  mk_function_call

poco/linklist.c:
 (Does not call po_say_fatal)

poco/main.c:
  (This is a test program, not part of the library - so this doesn't count in the same way)

poco/mathlib.c:
 (Does not call po_say_fatal)

poco/pocmemry.c:
  new_mblk

poco/poco.c:
  po_expecting_got_str
  po_need_token
  po_redefined
  po_undefined
  po_unmatched_paren
  no_assign_void
  unknown_assignment
  no_struct_assign
  no_use_void
  get_array
  not_a_member
  get_pmember
  get_member
  upgrade_numerical_expression
  po_coerce_to_boolean
#ifdef STRING_EXPERIMENT
  cant_convert_to_String
  po_coerce_to_string
#endif
  po_coerce_numeric_exp
  po_coerce_expression
  po_get_prim
  get_dereference
  get_address
  make_assign
  plus_equals
  po_new_frame
  po_append_type
  po_cat_type
  po_set_base_type
  find_global_use
  po_check_undefined_funcs
  po_compile_file

poco/poco_ffi.c: (Calls through fprintf if ffi_prep_cif fails.)


poco/poco_unix.c:
 (Does not call po_say_fatal)

poco/pocodis.c:
  po_check_instr_table
  po_disasm

poco/pocolib.c:
  po_cat_type
  safe_file_check
  po_fopen
  po_memchk

#ifdef STRING_EXPERIMENT
poco/postring.c:
  po_sr_new_copy
  po_sr_cat

#endif

poco/pp.c:
  pp_say_fatal (wrapper for po_say_fatal and is called extensively)

poco/ppeval.c:
  pp_say_fatal
  pp_atom

poco/runops.c:
  (Does not directly call po_say_fatal, calls fprintf if an error occurs during runtime.)

poco/safefile.c:
  (Does not call po_say_fatal)

poco/statemen.c:
  get_while
  get_do
  get_switch
  get_case
  get_default
  get_if
  get_return
  get_for
  get_break
  get_continue
  po_get_goto
  get_undef_label
  warn_no_effect
  po_need_comma_or_brace
  po_check_array_dim

poco/strlib.c:
  po_strcmp
  po_stricmp
  po_strncmp
  po_strlen
  po_strcpy
  po_strncpy
  po_strcat
  po_strdup
  po_strstr
  po_stristr
  po_atoi
  po_atof
  po_strpbrk
  po_strspn
  po_strcspn
  po_strtok
  po_getenv
  po_strlwr
  po_strupr

poco/struct.c:
  po_get_struct
  pf_to_sif

poco/token.c:
  get_digits
  tokenize_word

poco/trace.c:
  po_compress_line_data
  resize_line_data

poco/varinit.c:
  anytype_init
  quo_init
  array_init
  struct_init
  po_var_init


```


