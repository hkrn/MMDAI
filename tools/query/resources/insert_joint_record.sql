insert into `joints` (
  `index`,
  `parent_model_id`,
  `parent_rigid_body_a_id`,
  `parent_rigid_body_b_id`,
  `name_ja`,
  `name_en`,
  `type`
) values (
  :index,
  :parent_model,
  :parent_rigid_body_a,
  :parent_rigid_body_b,
  :name_ja,
  :name_en,
  :type
)
