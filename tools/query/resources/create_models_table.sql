create table `models` (
  `id` integer not null primary key autoincrement,
  `version` real not null,
  `encoding` integer not null,
  `uv` integer not null,
  `sha1` char(40) not null unique on conflict fail,
  `name_ja` text not null,
  `comment_ja` text not null,
  `name_en` text not null,
  `comment_en` text not null,
  `filename` text not null
 );
