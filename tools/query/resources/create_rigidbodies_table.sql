create table mmq_rigidbodies (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `object_type` int not null,
  `shape_type` int not null,
  `mass` float not null,
  `linear_damping` float not null,
  `angular_damping` float not null,
  `friction` float not null,
  `restitution` float not null
 );
