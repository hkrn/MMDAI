create table mmq_vertices (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model_id` integer not null,
  `type` integer not null,
  foreign key(parent_model_id) references mmq_models(id)
 );
