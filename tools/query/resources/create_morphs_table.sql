create table mmq_morphs (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model` integer not null,
  `name_ja` text not null,
  `name_en` text not null,
  `type` integer not null,
  `category` integer not null
 );
