create table `project_preferences` (
  `id` integer not null primary key autoincrement,
  `name` varchar(255) not null unique on conflict fail,
  `value` blob not null
);
create index `idx_project_preferences_name` on `project_preferences` (`name`);

create table `models` (
  `id` integer not null primary key autoincrement,
  `uuid` char(38) not null unique on conflict fail,
  `order` integer not null,
  `name` text not null,
  `path` text null
);
create index `idx_models_uuid` on `models` (`uuid`);

create table `model_preferences` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `name` varchar(255) not null unique on conflict fail,
  `value` blob not null,
  foreign key (`parent_model_id`) references `models` (`id`) on update restrict on delete cascade
);
create index `idx_model_preferences_name` on `model_preferences` (`name`);
create index `idx_model_preferences_fk_parent_model_id` on `model_preferences` (`parent_model_id`);

create table `motions` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `uuid` char(38) not null unique on conflict fail,
  `duration` integer not null,
  `path` text null,
  foreign key (`parent_model_id`) references `models` (`id`) on update restrict on delete cascade
);
create index `idx_motions_uuid` on `motions` (`uuid`);
create index `idx_motions_fk_parent_model_id` on `motions` (`parent_model_id`);

create table `bone_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references `motions` (`id`) on update restrict on delete cascade
);
create index `idx_bone_tracks_name` on `bone_tracks` (`name`);
create index `idx_bone_tracks_fk_parent_motion_id` on `bone_tracks` (`parent_motion_id`);

create table `bone_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references `motions` (`id`) on update restrict on delete cascade
);
create index `idx_bone_layers_index` on `bone_layers` (`index`);
create index `idx_bone_layers_fk_parent_motion_id` on `bone_layers` (`parent_motion_id`);

create table `bone_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `translation_x` float not null,
  `translation_y` float not null,
  `translation_z` float not null,
  `orientation_x` float not null,
  `orientation_y` float not null,
  `orientation_z` float not null,
  `orientation_w` float not null,
  foreign key (`parent_track_id`) references `bone_tracks` (`id`) on update restrict on delete cascade,
  foreign key (`parent_layer_id`) references `bone_layers` (`id`) on update restrict on delete cascade
);
create index `idx_bone_keyframes_time_index` on `bone_keyframes` (`time_index`);
create index `idx_bone_keyframes_fk_parent_track_id` on `bone_keyframes` (`parent_track_id`);
create index `idx_bone_keyframes_fk_parent_layer_id` on `bone_keyframes` (`parent_layer_id`);

create table `bone_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `bone_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_bone_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `bone_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_bone_keyframes` after insert on `bone_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `camera_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references `motions` (`id`) on update restrict on delete cascade
);
create index `idx_camera_layers_index` on `camera_layers` (`index`);
create index `idx_camera_layers_fk_parent_motion_id` on `camera_layers` (`parent_motion_id`);

create table `camera_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `position_x` float not null,
  `position_y` float not null,
  `position_z` float not null,
  `angle_x` float not null,
  `angle_y` float not null,
  `angle_z` float not null,
  `fov` float not null,
  `distance` float not null,
  foreign key (`parent_layer_id`) references `camera_layers` (`id`) on update restrict on delete cascade
);
create index `idx_camera_keyframes_time_index` on `camera_keyframes` (`time_index`);
create index `idx_camera_keyframes_fk_parent_layer_id` on `camera_keyframes` (`parent_layer_id`);

create table `camera_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `camera_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_camera_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `camera_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_camera_keyframes` after insert on `camera_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `effect_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references `motions` (`id`) on update restrict on delete cascade
);
create index `idx_effect_tracks_name` on `effect_tracks` (`name`);
create index `idx_effect_tracks_fk_parent_motion_id` on `effect_tracks` (`parent_motion_id`);

create table `effect_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_effect_layers_index` on `effect_layers` (`index`);
create index `idx_effect_layers_fk_parent_motion_id` on `effect_layers` (`parent_motion_id`);

create table `effect_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `parent_model_id` integer null,
  `parent_bone_id` integer null,
  `scale_factor` float not null,
  `opacity` float not null,
  `is_visible` integer not null,
  `is_shadow_enabled` integer not null,
  `is_add_blend_enabled` integer not null,
  foreign key (`parent_track_id`) references bone_tracks(`id`) on update restrict on delete cascade,
  foreign key (`parent_layer_id`) references bone_layers(`id`) on update restrict on delete cascade
);
create index `idx_effect_keyframes_time_index` on `effect_keyframes` (`time_index`);
create index `idx_effect_keyframes_fk_parent_track_id` on `effect_keyframes` (`parent_track_id`);
create index `idx_effect_keyframes_fk_parent_layer_id` on `effect_keyframes` (`parent_layer_id`);

create table `effect_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `effect_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_effect_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `effect_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_effect_keyframes` after insert on `effect_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `light_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_light_layers_index` on `light_layers` (`index`);
create index `idx_light_fk_parent_motion_id` on `light_layers` (`parent_motion_id`);

create table `light_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `position_x` float not null,
  `position_y` float not null,
  `position_z` float not null,
  `direction_x` float not null,
  `direction_y` float not null,
  `direction_z` float not null,
  foreign key (`parent_layer_id`) references light_layers(`id`) on update restrict on delete cascade
);
create index `idx_light_keyframes_time_index` on `light_keyframes` (`time_index`);
create index `idx_light_fk_parent_layer_id` on `light_keyframes` (`parent_layer_id`);

create table `light_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `light_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_light_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `light_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_light_keyframes` after insert on `light_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `model_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_model_tracks_name` on `model_tracks` (`name`);
create index `idx_model_tracks_fk_parent_motion_id` on `model_tracks` (`parent_motion_id`);

create table `model_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_model_layers_index` on `model_layers` (`index`);
create index `idx_model_layers_fk_parent_motion_id` on `model_layers` (`parent_motion_id`);

create table `model_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `edge_width` float not null,
  `edge_color_r` float not null,
  `edge_color_g` float not null,
  `edge_color_b` float not null,
  `edge_color_a` float not null,
  `is_visible` integer not null,
  `is_shadow_enabled` integer not null,
  `is_add_blend_enabled` integer not null,
  `is_physics_enabled` integer not null,
  foreign key (`parent_track_id`) references model_tracks(`id`) on update restrict on delete cascade,
  foreign key (`parent_layer_id`) references model_layers(`id`) on update restrict on delete cascade
);
create index `idx_model_keyframes_time_index` on `model_keyframes` (`time_index`);
create index `idx_model_keyframes_fk_parent_track_id` on `model_keyframes` (`parent_track_id`);
create index `idx_model_keyframes_fk_parent_layer_id` on `model_keyframes` (`parent_layer_id`);

create table `model_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `model_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_model_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `model_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_model_keyframes` after insert on `model_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `morph_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_morph_tracks_name` on `morph_tracks` (`name`);
create index `idx_morph_tracks_fk_parent_motion_id` on `morph_tracks` (`parent_motion_id`);

create table `morph_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_morph_layers_index` on `morph_layers` (`index`);
create index `idx_morph_layers_fk_parent_motion_id` on `morph_layers` (`parent_motion_id`);

create table `morph_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `weight` float not null,
  foreign key (`parent_track_id`) references morph_tracks(`id`) on update restrict on delete cascade,
  foreign key (`parent_layer_id`) references morph_layers(`id`) on update restrict on delete cascade
);
create index `idx_morph_keyframes_time_index` on `morph_keyframes` (`time_index`);
create index `idx_morph_keyframes_fk_parent_track_id` on `morph_keyframes` (`parent_track_id`);
create index `idx_morph_keyframes_fk_parent_layer_id` on `morph_keyframes` (`parent_layer_id`);

create table `morph_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `morph_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_morph_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `morph_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_morph_keyframes` after insert on `morph_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;

create table `project_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (`parent_motion_id`) references motions(`id`) on update restrict on delete cascade
);
create index `idx_project_layers_fk_parent_motion_id` on `project_layers` (`parent_motion_id`);

create table `project_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `shadow_mode` integer not null,
  `shadow_distance` float not null,
  `shadow_depth` float not null,
  `gravity_factor` float not null,
  `gravity_direction_x` float not null,
  `gravity_direction_y` float not null,
  `gravity_direction_z` float not null,
  foreign key (`parent_layer_id`) references project_layers(`id`) on update restrict on delete cascade
);
create index `idx_project_keyframes_time_index` on `project_keyframes` (`time_index`);
create index `idx_project_keyframes_fk_parent_layer_id` on `project_keyframes` (`parent_layer_id`);

create table `project_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null,
  `x2` float not null,
  `y1` float not null,
  `y2` float not null,
  foreign key (`parent_keyframe_id`) references `project_keyframes` (`id`) on update restrict on delete cascade
);
create index `idx_project_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `project_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create trigger `update_motion_duration_at_inserting_project_keyframes` after insert on `project_keyframes`
  when motion.id = new.parent_motion_id and motion.duration < new.time_index
  begin
    update motion set duration = new.time_index where motion.id = new.parent_motion_id;
  end;
