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

extern "C" uintptr_t _popit_draw_cursor_hook;
extern "C" uintptr_t _run_frame_hook;

extern "C" uintptr_t _base_profile_load_hook;

extern "C" uintptr_t _popit_render_ui_debug_hook;

extern "C" uintptr_t _popit_attempt_tweak_hook;
extern "C" uintptr_t _popit_stop_tweak_hook;

extern "C" uintptr_t _gmat_player_colour_hook;

extern "C" uintptr_t _on_reflect_load_thing_hook;
extern "C" uintptr_t _on_reflect_start_save_thing_hook;
extern "C" uintptr_t _on_reflect_finish_save_thing_hook;

extern "C" uintptr_t _initextradata_part_generatedmesh;

extern "C" uintptr_t _custom_gooey_network_action_hook;

#define PORT_ALLOWED_TO_APPLY_SYNCED_UPDATES (0x000138f4)
#define PORT_RGAME_RUN_FRAME (0x000b12a4)
#define PORT_RGAME_SORT_PAD_INPUTS_INTO_YELLOW_HEADS (0x000a1204)

#endif // ALEAR_HOOKS_H