create table mmq_joints (
  `id` integer not null primary key autoincrement,
  `index` integer not null,
  `parent_model_id` integer not null,
  `name_ja` text not null default "",
  `name_en` text not null default "",
  `type` int not null,
  foreign key(parent_model_id) references mmq_models(id)
 );
