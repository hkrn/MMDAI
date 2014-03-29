insert into `rigidbodies` (
  `index`,
  `parent_model_id`,
  `parent_bone_id`,
  `name_ja`,
  `name_en`,
  `object_type`,
  `shape_type`,
  `mass`,
  `linear_damping`,
  `angular_damping`,
  `friction`,
  `restitution`
) values (
  :index,
  :parent_model,
  :parent_bone,
  :name_ja,
  :name_en,
  :object_type,
  :shape_type,
  :mass,
  :linear_damping,
  :angular_damping,
  :friction,
  :restitution
)
