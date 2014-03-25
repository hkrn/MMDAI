create table mmq_bones (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `parent_bone` interger null,
  `destination_bone` integer null,
  `is_movable` integer not null,
  `is_rotateable` integer not null,
  `is_visible` integer not null,
  `is_interactive` integer not null,
  `is_inherent_translation_enabled` integer not null,
  `is_inherent_orientation_enabled` integer not null,
  `has_inverse_kinematics` integer not null,
  `has_local_axes` integer not null,
  `has_fixed_axes` integer not null
 );
