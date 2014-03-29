create table `joints` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `parent_rigidbody_a_id` integer null,
  `parent_rigidbody_b_id` integer null,
  `index` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `type` int not null,
  foreign key (`parent_model_id`) references models(`id`) on update restrict on delete cascade
 );
