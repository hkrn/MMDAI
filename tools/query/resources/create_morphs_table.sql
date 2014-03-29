create table `morphs` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `index` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `type` integer not null,
  `category` integer not null,
  foreign key (`parent_model_id`) references models(`id`)
 );
