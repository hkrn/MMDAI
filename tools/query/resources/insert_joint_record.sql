insert into `joints` (
  `index`,
  `parent_model_id`,
  `parent_rigidbody_a_id`,
  `parent_rigidbody_b_id`,
  `name_ja`,
  `name_en`,
  `type`
) values (
  :index,
  :parent_model,
  :parent_rigidbody_a,
  :parent_rigidbody_b,
  :name_ja,
  :name_en,
  :type
)
