create table mmq_ik_constraints (
  `id` integer not null primary key autoincrement,
  `parent_model_id` integer not null,
  `effector_bone_id` integer not null,
  `root_bone_id` integer null,
  `angle_limit` float not null,
  `num_iterations` integer not null,
  foreign key(parent_model_id) references mmq_models(id)
 );
