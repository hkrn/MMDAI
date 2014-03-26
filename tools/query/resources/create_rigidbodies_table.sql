create table mmq_rigidbodies (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model_id` integer not null,
  `name_ja` text not null default "",
  `name_en` text not null default "",
  `object_type` int not null,
  `shape_type` int not null,
  `mass` real not null,
  `linear_damping` real not null,
  `angular_damping` real not null,
  `friction` real not null,
  `restitution` real not null,
  foreign key(parent_model_id) references mmq_models(id)
 );
