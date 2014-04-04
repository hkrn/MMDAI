insert into bone_keyframes (
  `parent_track_id`,
  `parent_layer_id`,
  `time_index`,
  `translation_x`,
  `translation_y`,
  `translation_z`,
  `orientation_x`,
  `orientation_y`,
  `orientation_z`,
  `orientation_w`
) values (
  :parent_track_id,
  :parent_layer_id,
  :time_index,
  :translation_x,
  :translation_y,
  :translation_z,
  :orientation_x,
  :orientation_y,
  :orientation_z,
  :orientation_w
);
