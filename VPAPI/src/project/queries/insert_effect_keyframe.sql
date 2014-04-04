insert into effect_keyframes (
  `parent_track_id`,
  `parent_layer_id`,
  `time_index`,
  `parent_model_id`,
  `parent_bone_id`,
  `scale_factor`,
  `opacity`,
  `is_visible`,
  `is_shadow_enabled`,
  `is_add_blend_enabled`
) values (
  :parent_track_id,
  :parent_layer_id,
  :time_index,
  :parent_model_id,
  :parent_bone_id,
  :scale_factor,
  :opacity,
  :is_visible,
  :is_shadow_enabled,
  :is_add_blend_enabled
);
