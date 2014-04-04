insert into project_keyframe (
  `parent_layer_id`,
  `time_index`,
  `shadow_mode`,
  `shadow_distance`,
  `gravity_factor`,
  `gravity_direction_x`,
  `gravity_direction_y`,
  `gravity_direction_z`
) values (
  :parent_layer_id,
  :time_index,
  :shadow_mode,
  :shadow_distance,
  :gravity_factor,
  :gravity_direction_x,
  :gravity_direction_y,
  :gravity_direction_z
);
