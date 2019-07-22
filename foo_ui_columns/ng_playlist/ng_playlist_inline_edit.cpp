#include "../stdafx.h"
#include "ng_playlist.h"

namespace pvt {
bool PlaylistView::notify_before_create_inline_edit(
    const pfc::list_base_const_t<t_size>& indices, unsigned column, bool b_source_mouse)
{
    return (!b_source_mouse || main_window::config_get_inline_metafield_edit_mode() != main_window::mode_disabled)
        && column < m_edit_fields.get_count() && strlen(m_edit_fields[column]);
};
bool PlaylistView::notify_create_inline_edit(const pfc::list_base_const_t<t_size>& indices, unsigned column,
    pfc::string_base& p_text, t_size& p_flags, mmh::ComPtr<IUnknown>& pAutocompleteEntries)
{
    t_size indices_count = indices.get_count();
    m_edit_handles.remove_all();
    m_edit_handles.set_count(indices_count);

    pfc::array_t<file_info_impl> infos;
    pfc::ptr_list_t<const char> ptrs;
    infos.set_count(indices_count);

    m_edit_field = m_edit_fields[column];

    bool matching = true;

    for (t_size i = 0; i < indices_count; i++) {
        if (m_playlist_api->activeplaylist_get_item_handle(m_edit_handles[i], indices[i])) {
            m_edit_handles[i]->get_info(infos[i]);
            ptrs.add_item(infos[i].meta_get(m_edit_field, 0));
        } else {
            return false;
        }
        if (matching && i > 0
            && ((ptrs[i] && ptrs[i - 1] && strcmp(ptrs[i], ptrs[i - 1]) != 0)
                   || ((!ptrs[i] || !ptrs[i - 1]) && (ptrs[i] != ptrs[i - 1]))))
            matching = false;
    }

    p_text = matching ? (ptrs[0] ? ptrs[0] : "") : "<multiple values>";

    try {
        library_meta_autocomplete::ptr p_library_autocomplete = standard_api_create_t<library_meta_autocomplete>();
        p_flags |= inline_edit_autocomplete;
        pfc::com_ptr_t<IUnknown> pUnk;
        p_library_autocomplete->get_value_list(m_edit_field, pUnk);
        // p_library_autocomplete->get_value_list(m_edit_field, pUnk);
        pAutocompleteEntries = pUnk.get_ptr();
    } catch (exception_service_not_found const&) {
    }
    return true;
};
void PlaylistView::notify_save_inline_edit(const char* value)
{
    static_api_ptr_t<metadb_io_v2> tagger_api;
    if (strcmp(value, "<multiple values>") != 0) {
        metadb_handle_list ptrs(m_edit_handles);
        pfc::list_t<file_info_impl> infos;
        pfc::list_t<bool> mask;
        pfc::list_t<const file_info*> infos_ptr;
        t_size count = ptrs.get_count();
        mask.set_count(count);
        infos.set_count(count);
        // infos.set_count(count);
        for (t_size i = 0; i < count; i++) {
            assert(ptrs[i].is_valid());
            mask[i] = !ptrs[i]->get_info(infos[i]);
            infos_ptr.add_item(&infos[i]);
            if (!mask[i]) {
                const char* ptr = infos[i].meta_get(m_edit_field, 0);
                if (!(mask[i] = !((!ptr && strlen(value)) || (ptr && strcmp(ptr, value) != 0))))
                    infos[i].meta_set(m_edit_field, value);
            }
        }
        infos_ptr.remove_mask(mask.get_ptr());
        ptrs.remove_mask(mask.get_ptr());

        {
            service_ptr_t<file_info_filter_impl> filter = new service_impl_t<file_info_filter_impl>(ptrs, infos_ptr);
            tagger_api->update_info_async(ptrs, filter, GetAncestor(get_wnd(), GA_ROOT),
                metadb_io_v2::op_flag_no_errors | metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui,
                nullptr);
        }
    }
};
void PlaylistView::notify_exit_inline_edit()
{
    m_edit_field.reset();
    m_edit_handles.remove_all();
};

} // namespace pvt