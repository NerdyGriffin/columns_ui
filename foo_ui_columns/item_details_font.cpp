#include "stdafx.h"
#include "item_details.h"
#include "config.h"

bool operator==(const FontData& item1, const FontData& item2)
{
    return !FontData::g_compare(item1, item2);
}

void FontChangeNotify::reset(bool bKeepHandles /*= false*/)
{
    if (!bKeepHandles)
        m_fonts.set_size(0);
    m_font_changes.set_size(0);
}

bool FontChangeNotify::find_font(const FontData& p_font, t_size& index)
{
    t_size count = m_fonts.get_count();
    for (t_size i = 0; i < count; i++) {
        if (m_fonts[i]->m_data == p_font) {
            index = i;
            return true;
        }
    }

    return false;
}

TitleformatHookChangeFont::TitleformatHookChangeFont(const LOGFONT& lf)
{
    HDC dc = GetDC(nullptr);
    m_default_font_size = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(nullptr, dc);

    m_default_font_face = pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName));
}

bool TitleformatHookChangeFont::process_function(titleformat_text_out* p_out, const char* p_name,
    unsigned p_name_length, titleformat_hook_function_params* p_params, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "set_font", pfc_infinite)) {
        switch (p_params->get_param_count()) {
        case 2:
        case 3: {
            bool b_have_flags = p_params->get_param_count() == 3;
            const char* face;
            const char* pointsize;
            const char* flags;
            t_size face_length;
            t_size pointsize_length;
            t_size flags_length;
            p_params->get_param(0, face, face_length);
            p_params->get_param(1, pointsize, pointsize_length);
            if (b_have_flags)
                p_params->get_param(2, flags, flags_length);
            pfc::string8 temp;
            temp.add_byte('\x7');
            temp.add_string(face, face_length);
            temp.add_byte('\t');
            temp.add_string(pointsize, pointsize_length);
            temp.add_byte('\t');
            if (b_have_flags)
                temp.add_string(flags, flags_length);
            temp.add_byte('\x7');
            p_out->write(titleformat_inputtypes::unknown, temp);
            p_found_flag = true;
        }
        }
        return true;
    }
    if (!stricmp_utf8_ex(p_name, p_name_length, "reset_font", pfc_infinite)) {
        switch (p_params->get_param_count()) {
        case 0: {
            p_out->write(titleformat_inputtypes::unknown,
                "\x07"
                ""
                "\x07",
                pfc_infinite);
            p_found_flag = true;
        }
        }
        return true;
    }
    return false;
}

bool TitleformatHookChangeFont::process_field(
    titleformat_text_out* p_out, const char* p_name, unsigned p_name_length, bool& p_found_flag)
{
    p_found_flag = false;
    if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_face", pfc_infinite)) {
        p_out->write(titleformat_inputtypes::unknown, m_default_font_face);
        p_found_flag = true;
        return true;
    }
    if (!stricmp_utf8_ex(p_name, p_name_length, "default_font_size", pfc_infinite)) {
        p_out->write_int(titleformat_inputtypes::unknown, m_default_font_size);
        p_found_flag = true;
        return true;
    }
    return false;
}

void FontCodeGenerator::initialise(const LOGFONT& p_lf_default, HWND parent, UINT edit)
{
    m_lf = p_lf_default;
    uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, StringFontCode(m_lf));
}

void FontCodeGenerator::run(HWND parent, UINT edit)
{
    if (auto font_description = cui::fonts::select_font(parent, m_lf); font_description) {
        m_lf = font_description->log_font;
        uSendDlgItemMessageText(parent, edit, WM_SETTEXT, 0, StringFontCode(m_lf));
    }
}

FontCodeGenerator::StringFontCode::StringFontCode(const LOGFONT& lf)
{
    prealloc(64);
    HDC dc = GetDC(nullptr);
    unsigned pt = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(nullptr, dc);

    add_string("$set_font(");
    add_string(pfc::stringcvt::string_utf8_from_wide(lf.lfFaceName, tabsize(lf.lfFaceName)));
    add_byte(',');
    add_string(pfc::format_int(pt));
    add_string(",");
    if (lf.lfWeight == FW_BOLD)
        add_string("bold;");
    if (lf.lfItalic)
        add_string("italic;");
    add_string(")");
}

FontCodeGenerator::StringFontCode::operator const char*() const
{
    return get_ptr();
}

void g_parse_font_format_string(const char* str, t_size len, FontData& p_out)
{
    t_size ptr = 0;
    while (ptr < len) {
        t_size keyStart = ptr;
        while (ptr < len && str[ptr] != '=' && str[ptr] != ';')
            ptr++;
        t_size keyLen = ptr - keyStart;

        bool valueValid = false;
        t_size valueStart = 0;
        t_size valueLen = 0;

        if (str[ptr] == '=') {
            ptr++;
            valueStart = ptr;
            while (ptr < len && str[ptr] != ';')
                ptr++;
            valueLen = ptr - valueStart;
            ptr++;
            valueValid = true;
        } else if (ptr < len)
            ptr++;

        if (!stricmp_utf8_ex("bold", pfc_infinite, &str[keyStart], keyLen)) {
            p_out.m_bold = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
        } else if (!stricmp_utf8_ex("italic", pfc_infinite, &str[keyStart], keyLen)) {
            p_out.m_italic = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
        } else if (!stricmp_utf8_ex("underline", pfc_infinite, &str[keyStart], keyLen)) {
            p_out.m_underline = (!valueValid || !stricmp_utf8_ex("true", pfc_infinite, &str[valueStart], valueLen));
        }
    }
}

class ItemDetailsFontClient : public cui::fonts::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_font_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    cui::fonts::font_type_t get_default_font_type() const override { return cui::fonts::font_type_items; }

    void on_font_changed() const override { ItemDetails::g_on_font_change(); }
};

class ItemDetailsColoursClient : public cui::colours::client {
public:
    const GUID& get_client_guid() const override { return g_guid_item_details_colour_client; }
    void get_name(pfc::string_base& p_out) const override { p_out = "Item details"; }

    t_size get_supported_colours() const override
    {
        return cui::colours::colour_flag_background | cui::colours::colour_flag_text;
    }; // bit-mask
    t_size get_supported_bools() const override { return 0; }; // bit-mask

    bool get_themes_supported() const override { return false; };

    void on_bool_changed(t_size mask) const override{};
    void on_colour_changed(t_size mask) const override { ItemDetails::g_on_colours_change(); };
};

namespace {
ItemDetailsFontClient::factory<ItemDetailsFontClient> g_font_client_item_details;
ItemDetailsColoursClient::factory<ItemDetailsColoursClient> g_colour_client_item_details;
} // namespace
