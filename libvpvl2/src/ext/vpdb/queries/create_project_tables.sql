pragma auto_vacuum = 2;

create table `project_preferences` (
  `id` integer not null primary key autoincrement,
  `name` varchar(255) not null unique on conflict fail,
  `value` blob not null
);
create index `idx_project_preferences_name` on `project_preferences` (`name`);

create table `models` (
  `id` integer not null primary key autoincrement,
  `uuid` char(38) not null unique on conflict fail default (printf('{%s-%s-%s-%s-%s}', lower(hex(randomblob(4))), lower(hex(randomblob(2))), lower(hex(randomblob(2))), lower(hex(randomblob(2))), lower(hex(randomblob(6))))),
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
  foreign key (parent_model_id) references models (id) on update restrict
);
create index `idx_model_preferences_name` on `model_preferences` (`name`);
create index `idx_model_preferences_fk_parent_model_id` on `model_preferences` (`parent_model_id`);

create table `motions` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `uuid` char(38) not null unique on conflict fail default (printf('{%s-%s-%s-%s-%s}', lower(hex(randomblob(4))), lower(hex(randomblob(2))), lower(hex(randomblob(2))), lower(hex(randomblob(2))), lower(hex(randomblob(6))))),
  `path` text null,
  foreign key (parent_model_id) references models (id) on update restrict
);
create index `idx_motions_uuid` on `motions` (`uuid`);
create index `idx_motions_fk_parent_model_id` on `motions` (`parent_model_id`);

create table `bone_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_bone_tracks_name` on `bone_tracks` (`name`);
create index `idx_bone_tracks_fk_parent_motion_id` on `bone_tracks` (`parent_motion_id`);

create table `bone_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_bone_layers_index` on `bone_layers` (`index`);
create index `idx_bone_layers_fk_parent_motion_id` on `bone_layers` (`parent_motion_id`);

create table `bone_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `translation_x` float not null default 0.0,
  `translation_y` float not null default 0.0,
  `translation_z` float not null default 0.0,
  `orientation_x` float not null default 0.0,
  `orientation_y` float not null default 0.0,
  `orientation_z` float not null default 0.0,
  `orientation_w` float not null default 1.0,
  foreign key (parent_track_id) references bone_tracks (id) on update restrict,
  foreign key (parent_layer_id) references bone_layers (id) on update restrict
);
create index `idx_bone_keyframes_time_index` on `bone_keyframes` (`time_index`);
create index `idx_bone_keyframes_fk_parent_track_id` on `bone_keyframes` (`parent_track_id`);
create index `idx_bone_keyframes_fk_parent_layer_id` on `bone_keyframes` (`parent_layer_id`);

create table `bone_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references bone_keyframes (id) on update restrict
);
create index `idx_bone_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `bone_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `camera_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_camera_layers_index` on `camera_layers` (`index`);
create index `idx_camera_layers_fk_parent_motion_id` on `camera_layers` (`parent_motion_id`);

create table `camera_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `position_x` float not null default 0.0,
  `position_y` float not null default 10.0,
  `position_z` float not null default 0.0,
  `angle_x` float not null default 0.0,
  `angle_y` float not null default 0.0,
  `angle_z` float not null default 0.0,
  `fov` float not null default 27.0,
  `distance` float not null default 50.0,
  foreign key (parent_layer_id) references camera_layers (id) on update restrict
);
create index `idx_camera_keyframes_time_index` on `camera_keyframes` (`time_index`);
create index `idx_camera_keyframes_fk_parent_layer_id` on `camera_keyframes` (`parent_layer_id`);

create table `camera_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references camera_keyframes (id) on update restrict
);
create index `idx_camera_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `camera_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `effect_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_effect_tracks_name` on `effect_tracks` (`name`);
create index `idx_effect_tracks_fk_parent_motion_id` on `effect_tracks` (`parent_motion_id`);

create table `effect_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
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
  `scale_factor` float not null default 1.0,
  `opacity` float not null default 1.0,
  `is_visible` integer not null default 1,
  `is_shadow_enabled` integer not null default 1,
  `is_add_blend_enabled` integer not null default 1,
  foreign key (parent_track_id) references effect_tracks (id) on update restrict,
  foreign key (parent_layer_id) references effect_layers (id) on update restrict
);
create index `idx_effect_keyframes_time_index` on `effect_keyframes` (`time_index`);
create index `idx_effect_keyframes_fk_parent_track_id` on `effect_keyframes` (`parent_track_id`);
create index `idx_effect_keyframes_fk_parent_layer_id` on `effect_keyframes` (`parent_layer_id`);

create table `effect_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references effect_keyframes (id) on update restrict
);
create index `idx_effect_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `effect_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `light_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions(id) on update restrict
);
create index `idx_light_layers_index` on `light_layers` (`index`);
create index `idx_light_fk_parent_motion_id` on `light_layers` (`parent_motion_id`);

create table `light_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `color_r` float not null default 0.6,
  `color_g` float not null default 0.6,
  `color_b` float not null default 0.6,
  `direction_x` float not null default -1.0,
  `direction_y` float not null default 1.0,
  `direction_z` float not null default -1.0,
  foreign key (parent_layer_id) references light_layers (id) on update restrict
);
create index `idx_light_keyframes_time_index` on `light_keyframes` (`time_index`);
create index `idx_light_fk_parent_layer_id` on `light_keyframes` (`parent_layer_id`);

create table `light_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` integer not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references light_keyframes (id) on update restrict
);
create index `idx_light_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `light_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `model_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_model_tracks_name` on `model_tracks` (`name`);
create index `idx_model_tracks_fk_parent_motion_id` on `model_tracks` (`parent_motion_id`);

create table `model_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_model_layers_index` on `model_layers` (`index`);
create index `idx_model_layers_fk_parent_motion_id` on `model_layers` (`parent_motion_id`);

create table `model_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `edge_width` float not null default 1.0,
  `edge_color_r` float not null default 1.0,
  `edge_color_g` float not null default 1.0,
  `edge_color_b` float not null default 1.0,
  `edge_color_a` float not null default 1.0,
  `is_visible` integer not null default 1,
  `is_shadow_enabled` integer not null default 1,
  `is_add_blend_enabled` integer not null default 0,
  `is_physics_enabled` integer not null default 1,
  foreign key (parent_track_id) references model_tracks (id) on update restrict,
  foreign key (parent_layer_id) references model_layers (id) on update restrict
);
create index `idx_model_keyframes_time_index` on `model_keyframes` (`time_index`);
create index `idx_model_keyframes_fk_parent_track_id` on `model_keyframes` (`parent_track_id`);
create index `idx_model_keyframes_fk_parent_layer_id` on `model_keyframes` (`parent_layer_id`);

create table `model_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references model_keyframes (id) on update restrict
);
create index `idx_model_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `model_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `model_keyframe_ik_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_keyframe_id` integer not null,
  `is_enabled` integer not null default 1,
  foreign key (parent_track_id) references bone_tracks (id) on update restrict,
  foreign key (parent_keyframe_id) references model_keyframes (id) on update restrict
);
create index `idx_model_keyframe_ik_parameters_fk_parent_track_id` on `model_keyframe_ik_parameters` (`parent_track_id`);
create index `idx_model_keyframe_ik_parameters_fk_parent_keyframe_id` on `model_keyframe_ik_parameters` (`parent_keyframe_id`);

create table `morph_tracks` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_morph_tracks_name` on `morph_tracks` (`name`);
create index `idx_morph_tracks_fk_parent_motion_id` on `morph_tracks` (`parent_motion_id`);

create table `morph_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions (id) on update restrict
);
create index `idx_morph_layers_index` on `morph_layers` (`index`);
create index `idx_morph_layers_fk_parent_motion_id` on `morph_layers` (`parent_motion_id`);

create table `morph_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_track_id` integer not null,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `weight` float not null default 0.0,
  foreign key (parent_track_id) references morph_tracks (id) on update restrict,
  foreign key (parent_layer_id) references morph_layers (id) on update restrict
);
create index `idx_morph_keyframes_time_index` on `morph_keyframes` (`time_index`);
create index `idx_morph_keyframes_fk_parent_track_id` on `morph_keyframes` (`parent_track_id`);
create index `idx_morph_keyframes_fk_parent_layer_id` on `morph_keyframes` (`parent_layer_id`);

create table `morph_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references morph_keyframes (id) on update restrict
);
create index `idx_morph_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `morph_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create table `project_layers` (
  `id` integer not null primary key autoincrement,
  `parent_motion_id` integer not null,
  `index` integer not null,
  `name` text not null,
  foreign key (parent_motion_id) references motions(id) on update restrict
);
create index `idx_project_layers_fk_parent_motion_id` on `project_layers` (`parent_motion_id`);

create table `project_keyframes` (
  `id` integer not null primary key autoincrement,
  `parent_layer_id` integer not null,
  `time_index` integer not null,
  `shadow_mode` integer not null default 0,
  `shadow_distance` float not null default 100.0,
  `shadow_depth` float not null default 10000.0,
  `gravity_factor` float not null default -1.0,
  `gravity_direction_x` float not null default 0.0,
  `gravity_direction_y` float not null default 9.8,
  `gravity_direction_z` float not null default 0.0,
  foreign key (parent_layer_id) references project_layers (id) on update restrict
);
create index `idx_project_keyframes_time_index` on `project_keyframes` (`time_index`);
create index `idx_project_keyframes_fk_parent_layer_id` on `project_keyframes` (`parent_layer_id`);

create table `project_keyframe_interpolation_parameters` (
  `id` integer not null primary key autoincrement,
  `parent_keyframe_id` integer not null,
  `type` int not null,
  `x1` float not null default 0.15748031496062992,
  `x2` float not null default 0.84251968503937,
  `y1` float not null default 0.15748031496062992,
  `y2` float not null default 0.84251968503937,
  foreign key (parent_keyframe_id) references project_keyframes (id) on update restrict
);
create index `idx_project_keyframe_interpolation_parameters_fk_parent_keyframe_id` on `project_keyframe_interpolation_parameters` (`parent_keyframe_id`);

create view `project_duration` as
  select coalesce(max(bone_keyframes.time_index), 0) as bone_keyframe_duration,
         coalesce(max(camera_keyframes.time_index), 0) as camera_keyframe_duration,
         coalesce(max(effect_keyframes.time_index), 0) as effect_keyframe_duration,
         coalesce(max(light_keyframes.time_index), 0) as light_keyframe_duration,
         coalesce(max(model_keyframes.time_index), 0) as model_keyframe_duration,
         coalesce(max(morph_keyframes.time_index), 0) as morph_keyframe_duration,
         coalesce(max(project_keyframes.time_index), 0) as project_keyframe_duration
    from bone_keyframes,
         camera_keyframes,
         effect_keyframes,
         light_keyframes,
         model_keyframes,
         morph_keyframes,
         project_keyframes;

create trigger `create_bone_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `bone_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
  end;

create trigger `create_camera_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `camera_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
    insert into `camera_keyframes` (`parent_layer_id`, `time_index`) values (last_insert_rowid(), 0);
  end;

create trigger `create_effect_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `effect_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
  end;

create trigger `create_light_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `light_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
    insert into `light_keyframes` (`parent_layer_id`, `time_index`) values (last_insert_rowid(), 0);
  end;

create trigger `create_model_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `model_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
  end;

create trigger `create_morph_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `morph_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
  end;

create trigger `create_project_layer_at_inserting_motion` after insert on `motions`
  begin
    insert into `project_layers` (`parent_motion_id`, `index`, `name`) values (new.id, 0, 'base');
    insert into `project_keyframes` (`parent_layer_id`, `time_index`) values (last_insert_rowid(), 0);
  end;

create trigger `create_interpolation_parameters_at_inserting_bone_keyframes` after insert on `bone_keyframes`
  begin
    insert into `bone_keyframe_interpolation_parameters` (`parent_keyframe_id`, `type`) values (new.id, 0), (new.id, 1), (new.id, 2), (new.id, 3);
  end;

create trigger `create_interpolation_parameters_at_inserting_camera_keyframes` after insert on `camera_keyframes`
  begin
    insert into `camera_keyframe_interpolation_parameters` (`parent_keyframe_id`, `type`) values (new.id, 0), (new.id, 1), (new.id, 2), (new.id, 3), (new.id, 4), (new.id, 5);
  end;

create trigger `create_interpolation_parameters_at_inserting_morph_keyframes` after insert on `morph_keyframes`
  begin
    insert into `morph_keyframe_interpolation_parameters` (`parent_keyframe_id`, `type`)
      values (new.id, 0);
  end;

create trigger `delete_keyframe_interpolation_parameters_at_deleting_bone_keyframe` before delete on `bone_keyframes`
  begin
    delete from `bone_keyframe_interpolation_parameters` where `parent_keyframe_id` = old.id;
  end;

create trigger `delete_keyframe_interpolation_parameters_at_deleting_camera_keyframe` before delete on `camera_keyframes`
  begin
    delete from `camera_keyframe_interpolation_parameters` where `parent_keyframe_id` = old.id;
  end;

create trigger `delete_keyframe_interpolation_parameters_at_deleting_morph_keyframe` before delete on `morph_keyframes`
  begin
    delete from `morph_keyframe_interpolation_parameters` where `parent_keyframe_id` = old.id;
  end;

create trigger `delete_ik_parameters_at_deleting_bone_track` before delete on `bone_tracks`
  begin
    delete from `model_keyframe_ik_parameters` where `parent_track_id` = old.id;
  end;

create trigger `delete_ik_parameters_at_deleting_model_keyframe` before delete on `model_keyframes`
  begin
    delete from `model_keyframe_ik_parameters` where `parent_keyframe_id` = old.id;
  end;

create trigger `delete_bone_keyframes_at_deleting_bone_layer` before delete on `bone_layers`
  begin
    delete from `bone_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_bone_tracks_at_deleting_bone_layer` before delete on `bone_tracks`
  begin
    delete from `bone_keyframes` where `parent_track_id` = old.id;
  end;

create trigger `delete_camera_keyframes_at_deleting_bone_layer` before delete on `camera_layers`
  begin
    delete from `camera_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_effect_keyframes_at_deleting_effect_layer` before delete on `effect_layers`
  begin
    delete from `effect_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_effect_keyframes_at_deleting_effect_tracks` before delete on `effect_tracks`
  begin
    delete from `bone_keyframes` where `parent_track_id` = old.id;
  end;

create trigger `delete_light_keyframes_at_deleting_light_layer` before delete on `light_layers`
  begin
    delete from `light_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_model_keyframes_at_deleting_model_layer` before delete on `model_layers`
  begin
    delete from `model_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_model_keyframes_at_deleting_model_tracks` before delete on `model_tracks`
  begin
    delete from `model_keyframes` where `parent_track_id` = old.id;
  end;

create trigger `delete_morph_keyframes_at_deleting_model_layer` before delete on `morph_layers`
  begin
    delete from `morph_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_morph_keyframes_at_deleting_model_tracks` before delete on `morph_tracks`
  begin
    delete from `morph_keyframes` where `parent_track_id` = old.id;
  end;

create trigger `delete_project_keyframes_at_deleting_project_layers` before delete on `project_layers`
  begin
    delete from `project_keyframes` where `parent_layer_id` = old.id;
  end;

create trigger `delete_motion_layers_and_keyframes` before delete on `motions`
  begin
    delete from `bone_layers` where `parent_motion_id` = old.id;
    delete from `bone_tracks` where `parent_motion_id` = old.id;
    delete from `camera_layers` where `parent_motion_id` = old.id;
    delete from `effect_layers` where `parent_motion_id` = old.id;
    delete from `effect_tracks` where `parent_motion_id` = old.id;
    delete from `light_layers` where `parent_motion_id` = old.id;
    delete from `model_layers` where `parent_motion_id` = old.id;
    delete from `model_tracks` where `parent_motion_id` = old.id;
    delete from `morph_layers` where `parent_motion_id` = old.id;
    delete from `morph_tracks` where `parent_motion_id` = old.id;
    delete from `project_layers` where `parent_motion_id` = old.id;
  end;

insert into models (`uuid`, `order`, `name`) values ( '{00000000-0000-0000-0000-000000000000}', -1, 'NULL' );
insert into motions (`parent_model_id`) values (last_insert_rowid());
insert into motions (`parent_model_id`) select models.id from models where models.uuid = '{00000000-0000-0000-0000-000000000000}';
insert into motions (`parent_model_id`) select models.id from models where models.uuid = '{00000000-0000-0000-0000-000000000000}';

pragma user_version = 1;
