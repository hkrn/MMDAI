create table mmq_ik_joints (
  `id` integer not null primary key autoincrement,
  `parent_constraint_id` integer not null,
  `has_angle_limit` integer not null,
  `upper_limit_x` real not null,
  `upper_limit_y` real not null,
  `upper_limit_z` real not null,
  `lower_limit_x` real not null,
  `lower_limit_y` real not null,
  `lower_limit_z` real not null,
  foreign key(parent_constraint_id) references mmq_ik_constraints(id)
 );
