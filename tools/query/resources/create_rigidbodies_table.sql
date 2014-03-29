create table rigidbodies (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `parent_bone_id` integer null,
  `index` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `object_type` int not null,
  `shape_type` int not null,
  `mass` real not null,
  `linear_damping` real not null,
  `angular_damping` real not null,
  `friction` real not null,
  `restitution` real not null,
  foreign key (`parent_model_id`) references models(`id`) on update restrict on delete cascade
 );
