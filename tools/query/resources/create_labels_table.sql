create table `labels` (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `index` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `is_special` integer not null,
  foreign key (`parent_model_id`) references models(`id`)
 );
