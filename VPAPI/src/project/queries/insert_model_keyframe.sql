insert into model_keyframe (
  `parent_layer_id`,
  `time_index`,
  `edge_width`,
  `edge_color_r`,
  `edge_color_g`,
  `edge_color_b`,
  `edge_color_a`,
  `is_visible`,
  `is_shadow_enabled`,
  `is_add_blend_enabled`,
  `is_physics_enabled`
) values (
  :parent_layer_id,
  :time_index,
  :edge_width,
  :edge_color_r,
  :edge_color_g,
  :edge_color_b,
  :edge_color_a,
  :is_visible,
  :is_shadow_enabled,
  :is_add_blend_enabled,
  :is_physics_enabled
);
