#ifndef ALEAR_HOOKS_H
#define ALEAR_HOOKS_H

extern "C" uintptr_t _gsub_rlst_hook;
extern "C" uintptr_t _popit_isitemselected_hook;
extern "C" uintptr_t _popit_dopoppetsection_hook;
extern "C" uintptr_t _global_onswapbuffers_hook;
extern "C" uintptr_t _initextradata_cthing_hook;
extern "C" uintptr_t _destroyextradata_cthing_hook;
extern "C" uintptr_t _global_webternate_hook;

extern "C" uintptr_t _create_boundary_artist_hook;
extern "C" uintptr_t _create_boundary_primary_index_artist_hook;

extern "C" uintptr_t _create_first_boundary_artist_hook;

extern "C" uintptr_t _apply_boundaries_sort_index_first;
extern "C" uintptr_t _apply_boundaries_sort_index_loop;

extern "C" uintptr_t _global_icon_size_hook;

extern "C" uintptr_t _gooey_image_ctor_animated_hook;
extern "C" uintptr_t _sdf_button_animated_hook;
extern "C" uintptr_t _gooey_image_update_hook;

extern "C" uintptr_t _fady_thing_hook;
extern "C" uintptr_t _fixup_custom_pick_object_select_hook;

extern "C" uintptr_t _gooey_frame_clip_hook;
extern "C" uintptr_t _custom_item_grid_hook;

extern "C" uintptr_t _custom_event_projects_hook;

extern "C" uintptr_t _popit_close_hook;
extern "C" uintptr_t _popit_alphabetical_hook;

extern "C" uintptr_t _global_artist_hook;
extern "C" uintptr_t _global_pref_hook;
extern "C" uintptr_t _custom_tool_type_hook;
extern "C" uintptr_t _custom_poppet_message_hook;
extern "C" uintptr_t _custom_pick_object_action_hook;

#endif // ALEAR_HOOKS_H