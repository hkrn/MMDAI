insert into morph_keyframe (
  `parent_track_id`,
  `parent_layer_id`,
  `time_index`,
  `weight`
) values (
  :parent_track_id,
  :parent_layer_id,
  :time_index,
  :weight
);
