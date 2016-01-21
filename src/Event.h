#ifndef __EVENT_H__
#define __EVENT_H__

struct MouseEvent
{
    int x, y;
    int w, h;
    bool down;
};

struct KeyEvent
{
    const char* name;
    bool down;

    // TODO: more keys to add
    // TODO: finalize key names?
    static constexpr const char* const key_space = "key_space";
    static constexpr const char* const key_apostrophe = "key_apostrophe";
    static constexpr const char* const key_comma = "key_comma";
    static constexpr const char* const key_minus = "key_minus";
    static constexpr const char* const key_period = "key_period";
    static constexpr const char* const key_slash = "key_slash";
    static constexpr const char* const key_0 = "key_0";
    static constexpr const char* const key_1 = "key_1";
    static constexpr const char* const key_2 = "key_2";
    static constexpr const char* const key_3 = "key_3";
    static constexpr const char* const key_4 = "key_4";
    static constexpr const char* const key_5 = "key_5";
    static constexpr const char* const key_6 = "key_6";
    static constexpr const char* const key_7 = "key_7";
    static constexpr const char* const key_8 = "key_8";
    static constexpr const char* const key_9 = "key_9";
    static constexpr const char* const key_semicolon = "key_semicolon";
    static constexpr const char* const key_equal = "key_equal";
    static constexpr const char* const key_a = "key_a";
    static constexpr const char* const key_b = "key_b";
    static constexpr const char* const key_c = "key_c";
    static constexpr const char* const key_d = "key_d";
    static constexpr const char* const key_e = "key_e";
    static constexpr const char* const key_f = "key_f";
    static constexpr const char* const key_g = "key_g";
    static constexpr const char* const key_h = "key_h";
    static constexpr const char* const key_i = "key_i";
    static constexpr const char* const key_j = "key_j";
    static constexpr const char* const key_k = "key_k";
    static constexpr const char* const key_l = "key_l";
    static constexpr const char* const key_m = "key_m";
    static constexpr const char* const key_n = "key_n";
    static constexpr const char* const key_o = "key_o";
    static constexpr const char* const key_p = "key_p";
    static constexpr const char* const key_q = "key_q";
    static constexpr const char* const key_r = "key_r";
    static constexpr const char* const key_s = "key_s";
    static constexpr const char* const key_t = "key_t";
    static constexpr const char* const key_u = "key_u";
    static constexpr const char* const key_v = "key_v";
    static constexpr const char* const key_w = "key_w";
    static constexpr const char* const key_x = "key_x";
    static constexpr const char* const key_y = "key_y";
    static constexpr const char* const key_z = "key_z";
    static constexpr const char* const key_leftbracket = "key_leftbracket";
    static constexpr const char* const key_backslash = "key_backslash";
    static constexpr const char* const key_rightbracket = "key_rightbracket";
    static constexpr const char* const key_grave = "key_grave";
    static constexpr const char* const key_escape = "key_escape";
    static constexpr const char* const key_enter = "key_enter";
    static constexpr const char* const key_tab = "key_tab";
    static constexpr const char* const key_backspace = "key_backspace";
    static constexpr const char* const key_insert = "key_insert";
    static constexpr const char* const key_delete = "key_delete";
    static constexpr const char* const key_right = "key_right";
    static constexpr const char* const key_left = "key_left";
    static constexpr const char* const key_up = "key_up";
    static constexpr const char* const key_down = "key_down";
};

#endif
