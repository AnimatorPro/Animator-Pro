
struct spacing_data
    {
    WORD xoff;
    WORD new_width;
    };

struct rast_font
    {
    WORD *font_raster;
    WORD char_count;
    WORD start_char;
    struct spacing_data *spacing_array;
    WORD norm_char_width;
    WORD char_height;
    WORD words_in_line;
    WORD lines;
    };

