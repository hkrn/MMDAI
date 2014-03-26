create table mmq_models (
  `id` integer not null primary key autoincrement,
  `version` real not null,
  `encoding` integer not null,
  `uv` integer not null,
  `name_ja` text not null default "",
  `comment_ja` text not null default "",
  `name_en` text not null default "",
  `comment_en` text not null default "",
  `filename` text not null default "",
  `sha1` text not null unique on conflict fail
 );
