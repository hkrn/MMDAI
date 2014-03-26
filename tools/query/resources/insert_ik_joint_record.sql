insert into mmq_ik_joints (
  `parent_constraint_id`,
  `has_angle_limit`,
  `upper_limit_x`,
  `upper_limit_y`,
  `upper_limit_z`,
  `lower_limit_x`,
  `lower_limit_y`,
  `lower_limit_z`
) values (
  :constraint,
  :has_angle_limit,
  :upper_limit_x,
  :upper_limit_y,
  :upper_limit_z,
  :lower_limit_x,
  :lower_limit_y,
  :lower_limit_z
);
