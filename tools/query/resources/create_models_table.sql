create table mmq_models (
  `id` integer not null primary key autoincrement,
  `version` float not null,
  `encoding` integer not null,
  `uv` integer not null,
  `name_ja` text not null,
  `comment_ja` text not null,
  `name_en` text not null,
  `comment_en` text not null,
  `sha1` text not null
 );
