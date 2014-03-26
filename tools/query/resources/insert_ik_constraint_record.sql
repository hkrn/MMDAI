insert into mmq_ik_constraints (
  `parent_model_id`,
  `effector_bone_id`,
  `root_bone_id`,
  `angle_limit`,
  `num_iterations`
) values (
  :parent_model,
  :effector_bone,
  :root_bone,
  :angle_limit,
  :num_iterations
);
