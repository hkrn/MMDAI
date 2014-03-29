create table `vertices` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `index` integer not null,
  `type` integer not null,
  foreign key (`parent_model_id`) references models(`id`)
 );
