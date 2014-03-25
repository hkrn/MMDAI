create table mmq_labels (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `is_special` integer not null
 );
