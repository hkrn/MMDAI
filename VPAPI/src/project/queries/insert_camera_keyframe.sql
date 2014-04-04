insert into camera_keyframes (
  `parent_layer_id`,
  `time_index`,
  `position_x`,
  `position_y`,
  `position_z`,
  `angle_x`,
  `angle_y`,
  `angle_z`,
  `fov`,
  `distance`
) values (
  :parent_layer_id,
  :time_index,
  :position_x,
  :position_y,
  :position_z,
  :angle_x,
  :angle_y,
  :angle_z,
  :fov,
  :distance
);
