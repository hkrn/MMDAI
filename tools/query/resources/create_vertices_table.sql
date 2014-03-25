create table mmq_vertices (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model` integer not null,
  `type` integer not null
 );
